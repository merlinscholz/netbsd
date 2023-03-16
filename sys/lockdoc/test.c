#include <sys/module.h>
#include <sys/conf.h>
#include <sys/malloc.h>
#include <sys/kthread.h>
#include <sys/lockdoc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/cpu.h>

#ifndef LOCKDOC_TEST_ITERATIONS
#define LOCKDOC_TEST_ITERATIONS	200
#endif

#define DEV_DIR_NAME			"lockdoc"
#define DEV_FILE_CONTROL_NAME	"control"
#define DEV_FILE_ITER_NAME		"iterations"
#define DEFAULT_ITERATIONS	LOCKDOC_TEST_ITERATIONS

/*
 * For some reasons, our ring buffer (aka BSB ring buffer)
 * can only hold size - 1 elements.
 * If we want to store DEFAULT_ITERATIONS elements, as desired,
 * the buffer must be one element larger.
 * Hence, RING_BUFFER_SIZE_REAL is used for allocating the actual buffer
 * and used for the size member.
 * In contrast, RING_BUFFER_SIZE_VIRT is used when asigning a new value
 * for iterations in procfile_iter_write.
 */
#define RING_BUFFER_SIZE_REAL	(DEFAULT_ITERATIONS + 1)
#define RING_BUFFER_SIZE_VIRT	(RING_BUFFER_SIZE_REAL - 1)
#define RING_BUFFER_STORAGE_TYPE int
#define MK_STRING(x)    #x
#define START_AND_WAIT_THREAD(x)	start_and_wait_thread(MK_STRING(x), x)

dev_type_open(lockdoc_ctl_open);
dev_type_close(lockdoc_ctl_close);
dev_type_write(lockdoc_ctl_read);
dev_type_write(lockdoc_ctl_write);

dev_type_open(lockdoc_iter_open);
dev_type_close(lockdoc_iter_close);
dev_type_read(lockdoc_iter_read);
dev_type_write(lockdoc_iter_write);

static const struct cdevsw lockdoc_ctl_cdevsw = {
	.d_read = lockdoc_ctl_read,
	.d_write = lockdoc_ctl_write,

	.d_open = lockdoc_ctl_open,
	.d_close = lockdoc_ctl_close,

	.d_ioctl = noioctl,
	.d_stop = nostop,
	.d_tty = notty,
	.d_poll = nopoll,
	.d_mmap = nommap,
	.d_kqfilter = nokqfilter,
	.d_discard = nodiscard,
	.d_flag = D_OTHER
};
static const struct cdevsw lockdoc_iter_cdevsw = {
	.d_read = lockdoc_iter_read,
	.d_write = lockdoc_iter_write,

	.d_open = lockdoc_iter_open,
	.d_close = lockdoc_iter_close,

	.d_ioctl = noioctl,
	.d_stop = nostop,
	.d_tty = notty,
	.d_poll = nopoll,
	.d_mmap = nommap,
	.d_kqfilter = nokqfilter,
	.d_discard = nodiscard,
	.d_flag = D_OTHER
};

static int iterations = DEFAULT_ITERATIONS;
static lwp_t *control_thread = NULL;

static kmutex_t rb_lock;
static kmutex_t consumer_lock;
static kmutex_t producer_lock;

MALLOC_DEFINE(M_LOCKDOC, "lockdoc_ring_buffer", "ring buffer for LockDoc test");
struct lockdoc_ring_buffer {
	int next_in;
	int next_out;
	int size;
	RING_BUFFER_STORAGE_TYPE data[RING_BUFFER_SIZE_REAL];
};
static struct lockdoc_ring_buffer *ring_buffer = NULL;

static __noinline int is_full(volatile struct lockdoc_ring_buffer *buffer) { return (buffer->next_in + 1) % buffer->size == buffer->next_out; }
static __noinline int is_empty(volatile struct lockdoc_ring_buffer *buffer) { return buffer->next_out == buffer->next_in; }

static __noinline int produce(volatile struct lockdoc_ring_buffer *buffer, RING_BUFFER_STORAGE_TYPE data) {
	if (is_full(buffer)) {
		return -1;
	}
	buffer->data[buffer->next_in] = data;
	buffer->next_in = (buffer->next_in + 1) % buffer->size;

	return 0;
}

static __noinline RING_BUFFER_STORAGE_TYPE consume(volatile struct lockdoc_ring_buffer *buffer) {
	RING_BUFFER_STORAGE_TYPE result;

	if (is_empty(buffer)) {
		return -1;
	}
	result = buffer->data[buffer->next_out];
	buffer->next_out = (buffer->next_out + 1) % buffer->size;

	return result;
}

int lockdoc_ctl_open(dev_t self __unused, int flag __unused, int mode __unused,
          lwp_t *l __unused)
{
    return 0;
}

int lockdoc_ctl_close(dev_t self __unused, int flag __unused, int mode __unused,
           lwp_t *l __unused)
{
    return 0;
}

/*
 *	Stub out this method, otherwise (accidentaly) reading from /dev/lockdoc/control causes a kernel panic
 */
int lockdoc_ctl_read(dev_t self __unused, struct uio *uio __unused,
			int flags __unused)
{
	return 0;
}

int lockdoc_iter_open(dev_t self __unused, int flag __unused, int mode __unused,
          lwp_t *l __unused)
{
    return 0;
}

int lockdoc_iter_close(dev_t self __unused, int flag __unused, int mode __unused,
           lwp_t *l __unused)
{
    return 0;
}

static void producer_thread_work(void *data) {
	int i, ret;

	/*
	 * Produce 'iterations' elements.
	 * This fills every element in the ring buffer.
	 * The 'iterations'+1 call to produce() would fail due to a full buffer.
	 * The consumer thread will completely empty the buffer.
	 */
	for (i = 0; i < iterations; i++) {
		mutex_enter(&rb_lock);
		ret = is_full(ring_buffer);
		mutex_exit(&rb_lock);
		if (ret) {
			printf("%s: Ring buffer is full\n", __func__);
		}

		mutex_enter(&producer_lock);
		mutex_enter(&rb_lock);
		ret = produce(ring_buffer, i + 30);
		mutex_exit(&producer_lock);
		mutex_exit(&rb_lock);
		printf("%s-%03d: Produced(%d): %03d\n", __func__, i, ret, i + 30);

		kpause("W", false, mstohz(100), NULL);
	}

	kthread_exit(0);
}

static void consumer_thread_work(void *data) {
	int i, ret;

	for (i = 0; i < iterations; i++) {
		mutex_enter(&rb_lock);
		ret = is_empty(ring_buffer);
		mutex_exit(&rb_lock);
		if (ret) {
			printf("%s: Ring buffer is empty\n", __func__);
		}

		mutex_enter(&consumer_lock);
		mutex_enter(&rb_lock);
		ret = consume(ring_buffer);
		mutex_exit(&consumer_lock);
		mutex_exit(&rb_lock);
		printf("%s-%03d: Consumed: %03d\n", __func__, i, ret);

		kpause("W", false, mstohz(100), NULL);

	}

	kthread_exit(0);
}

static void dirty_nolocks_thread_work(void *data) {
	int i = 0, ret;

	/*
	 * Wait a bit. Otherwise the call to tsleep() in control_thread_work() will wait for ever,
	 * because this thread has terminated and the caller will not be notified.
	 */
	kpause("W", false, mstohz(500), NULL);

	
	ret = is_full(ring_buffer);
	if (ret) {
		printf("%s: Ring buffer is full\n", __func__);
	}

	ret = produce(ring_buffer, i - 1);
	printf("%s-%03d: Produced(%d): %03d\n", __func__, i, ret, i - 1);
	
	ret = is_empty(ring_buffer);
	if (ret) {
		printf("%s: Ring buffer is empty\n",__func__);
	}

	ret = consume(ring_buffer);
	printf("%s-%03d: Consumed: %03d\n", __func__, i, ret);

	kthread_exit(0);
}

static void dirty_fewlocks_thread_work(void *data) {
	int i = 0, ret;

	/*
	 * Wait a bit. Otherwise the call to tsleep() in control_thread_work() will wait for ever,
	 * because this thread has terminated and the caller will not be notified.
	 */
	kpause("W", false, mstohz(500), NULL);


	mutex_enter(&rb_lock);
	ret = is_full(ring_buffer);
	mutex_exit(&rb_lock);
	if (ret) {
		printf("%s: Ring buffer is full\n", __func__);
	}

	mutex_enter(&rb_lock);
	ret = produce(ring_buffer, i - 1);
	mutex_exit(&rb_lock);
	printf("%s-%03d: Produced(%d): %03d\n", __func__, i, ret, i - 1);

	mutex_enter(&rb_lock);
	ret = is_empty(ring_buffer);
	mutex_exit(&rb_lock);
	if (ret) {
		printf("%s: Ring buffer is empty\n", __func__);
	}

	mutex_enter(&rb_lock);
	ret = consume(ring_buffer);
	mutex_exit(&rb_lock);
	printf("%s-%03d: Consumed: %03d\n", __func__, i, ret);

	kthread_exit(0);
}

static void dirty_alllocks_thread_work(void *data) {
	int i = 0, ret;

	/*
	 * Wait a bit. Otherwise the call to tsleep() in control_thread_work() will wait for ever,
	 * because this thread has terminated and the caller will not be notified.
	 */
	kpause("W", false, mstohz(500), NULL);


	mutex_enter(&producer_lock);
	mutex_enter(&consumer_lock);
	mutex_enter(&rb_lock);
	ret = is_full(ring_buffer);
	mutex_exit(&rb_lock);
	mutex_exit(&consumer_lock);
	mutex_exit(&producer_lock);
	if (ret) {
		printf("%s: Ring buffer is full\n", __func__);
	}

	mutex_enter(&producer_lock);
	mutex_enter(&consumer_lock);
	mutex_enter(&rb_lock);
	ret = produce(ring_buffer, i - 1);
	mutex_exit(&rb_lock);
	mutex_exit(&consumer_lock);
	mutex_exit(&producer_lock);
	printf("%s-%03d: Produced(%d): %03d\n", __func__, i, ret, i - 1);

	mutex_enter(&producer_lock);
	mutex_enter(&consumer_lock);
	mutex_enter(&rb_lock);
	ret = is_empty(ring_buffer);
	mutex_exit(&rb_lock);
	mutex_exit(&consumer_lock);
	mutex_exit(&producer_lock);
	if (ret) {
		printf("%s: Ring buffer is empty\n", __func__);
	}

	mutex_enter(&producer_lock);
	mutex_enter(&consumer_lock);
	mutex_enter(&rb_lock);
	ret = consume(ring_buffer);
	mutex_exit(&rb_lock);
	mutex_exit(&consumer_lock);
	mutex_exit(&producer_lock);
	printf("%s-%03d: Consumed: %03d\n", __func__, i, ret);

	kthread_exit(0);
}

static void dirty_order_thread_work(void *data) {
	int i = 0, ret;

	/*
	 * Wait a bit. Otherwise the call to tsleep() in control_thread_work() will wait for ever,
	 * because this thread has terminated and the caller will not be notified.
	 */
	kpause("W", false, mstohz(500), NULL);


	mutex_enter(&rb_lock);
	mutex_enter(&producer_lock);
	ret = produce(ring_buffer, i - 1);
	mutex_exit(&producer_lock);
	mutex_exit(&rb_lock);
	printf("%s-%03d: Produced(%d): %03d\n", __func__, i, ret, i - 1);

	mutex_enter(&rb_lock);
	mutex_enter(&consumer_lock);
	ret = consume(ring_buffer);
	mutex_exit(&consumer_lock);
	mutex_exit(&rb_lock);
	printf("%s-%03d: Consumed: %03d\n", __func__, i, ret);

	kthread_exit(0);
}

static void start_and_wait_thread(const char *fn_name, void (*work_fn)(void*)) {
	lwp_t *temp = NULL;
	int error;
	
	printf("%s: Starting %s thread...\n", __func__, fn_name);	
	error = kthread_create(PRI_NONE, KTHREAD_MUSTJOIN | KTHREAD_MPSAFE, NULL, work_fn, NULL, &temp, "lockdoc-control");
	
	if (error) {
		return;
	}
	printf("%s: Waiting for %s thread (%d) to terminate...\n", __func__, fn_name, temp->l_lid);
	error = kthread_join(temp);

	if (error) {
		printf("%s: Error waiting for %s thread\n", __func__, fn_name);
	} else {
		printf("%s: %s thread terminated successfully\n", __func__, fn_name);
	}
}

static void control_thread_work(void *data) {

	mutex_init(&rb_lock, /*"LockDoc test rb lock",*/ MUTEX_DEFAULT, IPL_NONE);
	mutex_init(&consumer_lock, /*"LockDoc test consumer lock",*/ MUTEX_DEFAULT, IPL_NONE);
	mutex_init(&producer_lock, /*"LockDoc test producer lock",*/ MUTEX_DEFAULT, IPL_NONE);

	ring_buffer = malloc(sizeof(*ring_buffer), M_LOCKDOC, M_WAITOK | M_ZERO);
	if (!ring_buffer) {
		printf("Cannot allocate %u bytes for ring buffer\n", sizeof(*ring_buffer));
		kthread_exit(1);
	}
	ring_buffer->size = RING_BUFFER_SIZE_REAL;
	lockdoc_log_memory(1, "lockdoc_ring_buffer", ring_buffer, sizeof(*ring_buffer));

	START_AND_WAIT_THREAD(producer_thread_work);
	START_AND_WAIT_THREAD(consumer_thread_work);
	START_AND_WAIT_THREAD(dirty_nolocks_thread_work);
	START_AND_WAIT_THREAD(dirty_fewlocks_thread_work);
	START_AND_WAIT_THREAD(dirty_alllocks_thread_work);
	START_AND_WAIT_THREAD(dirty_order_thread_work);

	lockdoc_log_memory(0, "lockdoc_ring_buffer", ring_buffer, sizeof(*ring_buffer));
	free(ring_buffer, M_LOCKDOC);

	mutex_destroy(&rb_lock);
	mutex_destroy(&producer_lock);
	mutex_destroy(&consumer_lock);

	kthread_exit(0);
}

#define BUFSIZE	5
int lockdoc_ctl_write(dev_t self, struct uio *uio, int flags){

	unsigned long value = 0;
	size_t amt;
	int error;
	char buffer[BUFSIZE];

	if (uio->uio_offset != 0 && (uio->uio_offset != BUFSIZE)) {
		return (EINVAL);
	}

	amt = MIN(uio->uio_resid, BUFSIZE);
	error = uiomove(buffer, amt, uio);
	if (error) {
		return (error);
	}

	/* parse input */
	value = strtoul(buffer, NULL, 10);
	if (value == 1) {
		error = kthread_create(PRI_NONE, KTHREAD_MUSTJOIN | KTHREAD_MPSAFE, NULL, control_thread_work, NULL, &control_thread, "lockdoc-control");
		if (error) {
			return (error);
		}

		kpause("W", false, mstohz(200), NULL);

		uprintf("%s: Waiting for control_thread to terminate...\n", __func__);
		// This will block the caller until all threads terminated
		error = kthread_join(control_thread);
		if (error) {
			uprintf("%s: Wait for control thread timed out\n", __func__);
			return (error);
		}
		uprintf("%s: Control thread terminated successfully!\n", __func__);
	} else {
		return (EINVAL);
	}

	return 0;
}
#undef BUFSIZE

#define BUFSIZE 20
int lockdoc_iter_read(dev_t self, struct uio *uio, int flags)
{
	size_t amt;
	int error;
	char buffer[BUFSIZE];
	int ret = snprintf(buffer, BUFSIZE, "%u\n", iterations);

	if (ret >= BUFSIZE) {
		ret = BUFSIZE;
	}

	amt = MIN(uio->uio_resid, uio->uio_offset >= ret ? 0 :
	    ret - uio->uio_offset);

	error = uiomove(buffer, amt, uio);
	if (error != 0) {
		uprintf("uiomove failed!\n");
	}

	return (error);
}
#undef BUFSIZE

#define BUFSIZE 20
int lockdoc_iter_write(dev_t self, struct uio *uio, int flags) {
	unsigned long value = 0;
	size_t amt;
	int error;
	char buffer[BUFSIZE];

	if (uio->uio_offset != 0 && (uio->uio_offset != BUFSIZE)) {
		return (EINVAL);
	}

	amt = MIN(uio->uio_resid, BUFSIZE);
	error = uiomove(buffer, amt, uio);
	if (error) {
		return (error);
	}

	/* parse input */
	value = strtoul(buffer, NULL, 10);
	/*
	 * Iterations cannot be larger than the buffer size.
	 * If 'iterations' is larger than the buffer size, 
	 * the consumer/producer thread will execute different code paths (iterations - RING_BUFFER_SIZE_VIRT) times.
	 * We want to the consumer and producer thread to execute the same code 'iterations' times.
	 */
	if (value > RING_BUFFER_SIZE_VIRT) {
		uprintf("%s: Desired iterations (%lu) is larger than buffer size (%d)\n", __func__, value, RING_BUFFER_SIZE_VIRT); 
		return (EINVAL);
	}
	iterations = value;
	uprintf("Setting iterations to %d\n", iterations);

	return 0;
}
#undef BUFSIZE

MODULE(MODULE_CLASS_MISC, lockdoc_test, NULL);

static int lockdoc_test_modcmd(modcmd_t what, void *arg __unused)
{
	int error = 0;

	switch (what) {
	case MODULE_CMD_INIT: ;
		int ctl_cmajor = 138, ctl_bmajor = -1;
		error = devsw_attach(DEV_FILE_CONTROL_NAME, NULL, &ctl_bmajor, &lockdoc_ctl_cdevsw, &ctl_cmajor);
		if (error != 0) {
			printf("Could not create lockdoc control device: ERRNO %i\n", error);
			break;
		}
		int iter_cmajor = 139, iter_bmajor = -1;
		error = devsw_attach(DEV_FILE_ITER_NAME, NULL, &iter_bmajor, &lockdoc_iter_cdevsw, &iter_cmajor);
		if (error != 0) {
			printf("Could not create lockdoc iterations device: ERRNO %i\n", error);
	     	devsw_detach(NULL, &lockdoc_ctl_cdevsw);
			break;
		}
		break;
	case MODULE_CMD_FINI:
	    devsw_detach(NULL, &lockdoc_ctl_cdevsw);
	    devsw_detach(NULL, &lockdoc_iter_cdevsw);
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}
