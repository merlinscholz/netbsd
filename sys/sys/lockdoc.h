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

int32_t lockdoc_get_ctx(void);
void lockdoc_x86_restore_intr(u_long eflags);

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

void trace_irqs_on(struct trapframe *frame);
void trace_irqs_off(struct trapframe *frame);

extern struct log_action la_buffer;

/*
 * Redefine lock function as macros instead of normal functions, so that __FILE__ etc. will work properly
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

/* Basic port I/O */
static inline void outb_(u_int8_t v, u_int16_t port)
{
	__asm __volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline void lockdoc_log_memory(int alloc, const char *datatype, const void *ptr, size_t size) {
    u_long flags;

	flags = lockdoc_x86_disable_intr();

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

	outb_(PING_CHAR,IO_PORT_LOG);

	lockdoc_x86_restore_intr(flags);
}

/*
 * TODO: Check for different lock types. We can *not*:
 * - Copy the FreeBSD approach with the embedded struct (since it just doesn't exist)
 * - Copy the Linux approach with __same_type (doesn't exist either)
 * 
 * Possible solutions:
 * - Pass an enum that is statically set in the macro, different in each function (similar to Linux)
 * 
 */
static inline void lockdoc_log_lock(int lock_op, const volatile void* ptr, const char *file, int line, const char* func, const char* lock_type, int irq_sync) {
    u_long eflags;
    eflags = x86_read_psl();
    lockdoc_x86_disable_intr();

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

    outb_(PING_CHAR,IO_PORT_LOG);

    x86_write_flags(eflags);
}

void lockdoc_send_current_task_addr(void);
void lockdoc_send_lwp_flag_offset(void);
void lockdoc_send_pid_offset(void);
void lockdoc_send_kernel_version(void);

#else
#define log_memory(a, b, c, d)
#define log_lock(a, b, c, d, e)

#endif /* LOCKDOC */

#endif /* __LOCKDOC_H__ */
