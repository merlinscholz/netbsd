#if !defined(__LOCKDOC_H__) && !defined(__ASSEMBLER__)
#define __LOCKDOC_H__

#include <arch/x86/include/cpufunc.h>
#include <arch/i386/include/frame.h>

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/lockdoc_event.h>
#include <sys/mutex.h>
#include <sys/rwlock.h>

#define DELIMITER	"#"
#define DELIMITER_CHAR	'#'
#define PING_CHAR	'p'
#define MK_STRING(x)	#x
#define IO_PORT_LOG	(0x00e9)

#ifdef LOCKDOC

/*
 * The struct we will use to transfer data between VM and host
 */
extern struct log_action la_buffer;

/*
 * Prototypes
 */

void    lockdoc_send_current_task_addr(void);
void    lockdoc_send_lwp_flag_offset(void);
void    lockdoc_send_pid_offset(void);
void    lockdoc_send_kernel_version(void);
int32_t lockdoc_get_ctx(void);

void	__mutex_enter(kmutex_t *, const char*, int, const char*);
void	__mutex_exit(kmutex_t *, const char*, int, const char*);
void	__mutex_spin_enter(kmutex_t *, const char*, int, const char*);
void	__mutex_spin_exit(kmutex_t *, const char*, int, const char*);
int    	__mutex_tryenter(kmutex_t *, const char*, int, const char*);

int     __rw_tryupgrade(krwlock_t *, const char*, int, const char*);
void    __rw_downgrade(krwlock_t *, const char*, int, const char*);
void	__rw_enter(krwlock_t *, const krw_t, const char*, int, const char*);
int	    __rw_tryenter(krwlock_t *, const krw_t, const char*, int, const char*);
void	__rw_exit(krwlock_t *, const char*, int, const char*);

void    __x86_disable_intr(const char*, int, const char*);
void    __x86_enable_intr(const char*, int, const char*);

void    __trace_irqs_on(const char *, int);
void    __trace_irqs_on_check(int, const char *, int);
void    __trace_irqs_on_asm(void);
void    __trace_irqs_off(const char *, int);
void    __trace_irqs_off_check(int, const char *, int);
void    __trace_irqs_off_asm(void);

/* Helper function to get a lock type (backported from NetBSD 10.0-STABLE) */
krw_t   rw_lock_op(krwlock_t *rw);


/*
 * Redefine lock/interrupt function as macros instead of normal functions,
 * so that __FILE__ etc. will work properly
 */
#define mutex_enter(lock) __mutex_enter(lock, __FILE__, __LINE__, __func__)
#define mutex_exit(lock) __mutex_exit(lock, __FILE__, __LINE__, __func__)
#define mutex_spin_enter(lock) __mutex_spin_enter(lock, __FILE__, __LINE__, __func__)
#define mutex_spin_exit(lock) __mutex_spin_exit(lock, __FILE__, __LINE__, __func__)
#define mutex_tryenter(lock) __mutex_tryenter(lock, __FILE__, __LINE__, __func__)

#define rw_tryupgrade(lock) __rw_tryupgrade(lock, __FILE__, __LINE__, __func__)
#define rw_downgrade(lock) __rw_downgrade(lock, __FILE__, __LINE__, __func__)
#define rw_enter(lock, type) __rw_enter(lock, type, __FILE__, __LINE__, __func__)
#define rw_tryenter(lock, type) __rw_tryenter(lock, type, __FILE__, __LINE__, __func__)
#define rw_exit(lock) __rw_exit(lock, __FILE__, __LINE__, __func__)

#define x86_disable_intr() __x86_disable_intr(__FILE__, __LINE__, __func__)
#define x86_enable_intr() __x86_enable_intr(__FILE__, __LINE__, __func__)

#define trace_irqs_on() __trace_irqs_on(__FILE__, __LINE__)
#define trace_irqs_on_check(frame) __trace_irqs_on_check(frame, __FILE__, __LINE__)
#define trace_irqs_off() __trace_irqs_off(__FILE__, __LINE__)
#define trace_irqs_off_check(frame) __trace_irqs_off_check(frame, __FILE__, __LINE__)


static inline void lockdoc_outb(u_int8_t v, u_int16_t port)
{
	__asm __volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline void lockdoc_log_memory(int alloc, const char *datatype, const void *ptr, size_t size) {
    u_long eflags;
    eflags = x86_read_flags();
    _x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = (alloc == 1 ? LOCKDOC_ALLOC : LOCKDOC_FREE);
	la_buffer.ptr = (unsigned long)ptr;
	la_buffer.size = size;
	la_buffer.ctx = lockdoc_get_ctx();

	/*
	 * One could use a more safe string function, e.g., strlcpy. 
	 * However, we want these *log* functions to be fast.
	 * We therefore skip all sanity checks, and all that stuff.
	 * To ensure any string buffer contains a valid string, we
	 * always write a NULL byte at its end.
	 */
	strncpy(la_buffer.type,datatype,LOG_CHAR_BUFFER_LEN);
	la_buffer.type[LOG_CHAR_BUFFER_LEN - 1] = '\0';

	lockdoc_outb(PING_CHAR,IO_PORT_LOG);

	x86_write_flags(eflags);
}

static inline void lockdoc_log_lock(int lock_op, const volatile void* ptr, const char *file, int line, const char* func, const char* lock_type, int irq_sync) {
    u_long eflags;
    eflags = x86_read_flags();
    _x86_disable_intr();

    memset(&la_buffer,0,sizeof(la_buffer));

    // action
    la_buffer.action = LOCKDOC_LOCK_OP;

    // operation
    la_buffer.lock_op = lock_op;

    // context
    la_buffer.ctx = lockdoc_get_ctx();

    // TODO Check for IRQ

    // pointer
    la_buffer.ptr = (uint32_t)ptr;

    // size only relevant for memory access

    // type
    strncpy(la_buffer.type, lock_type, LOG_CHAR_BUFFER_LEN);
	la_buffer.type[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // TODO lock_member
    strncpy(la_buffer.lock_member, "dummy", LOG_CHAR_BUFFER_LEN);
	la_buffer.lock_member[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // file
    strncpy(la_buffer.file, file, LOG_CHAR_BUFFER_LEN);
	la_buffer.file[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // line
    la_buffer.line = line;

    // function
	strncpy(la_buffer.function, func, LOG_CHAR_BUFFER_LEN);
	la_buffer.function[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // TODO preempt_count
    // irq_sync
    la_buffer.irq_sync = irq_sync;

    lockdoc_outb(PING_CHAR,IO_PORT_LOG);

    x86_write_flags(eflags);
}

#endif /* LOCKDOC */

#endif /* __LOCKDOC_H__ */
