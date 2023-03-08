#if !defined(__LOCKDOC_H__) && !defined(__ASSEMBLER__)
#define __LOCKDOC_H__

#include <arch/x86/include/cpufunc.h>

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/lockdebug.h>
#include <sys/lockdoc_event.h>

#define DELIMITER	"#"
#define DELIMITER_CHAR	'#'
#define PING_CHAR	'p'
#define MK_STRING(x)	#x
#define IO_PORT_LOG	(0x00e9)

#ifdef LOCKDOC

int32_t lockdoc_get_ctx(void);
void lockdoc_x86_restore_intr(u_long eflags);

void trace_irqs_on(struct trapframe *frame);
void trace_irqs_off(struct trapframe *frame);

extern struct log_action la_buffer;


// For DEFINEs in lockdebug.h
// Implemented in lockdoc/log.c
void	lockdoc_alloc(const char *, const char *, size_t, volatile void *, lockops_t *, uintptr_t);
void	lockdoc_free(const char *, const char *, size_t, volatile void *);
void	lockdoc_wantlock(const char *, const char *, size_t, const volatile void *, uintptr_t, int);
void	lockdoc_locked(const char *, const char *, size_t, volatile void *, void *, uintptr_t, int);
void	lockdoc_unlocked(const char *, const char *, size_t, volatile void *, uintptr_t, int);
void	lockdoc_barrier(const char *, const char *, size_t, volatile void *, int);
void	lockdoc_mem_check(const char *, const char *, size_t, void *, size_t);
void	lockdoc_wakeup(const char *, const char *, size_t, volatile void *, uintptr_t);

/* Basic port I/O */
static inline void outb_(u_int8_t v, u_int16_t port)
{
	__asm __volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline void log_memory(int alloc, const char *datatype, const void *ptr, size_t size) {
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

static inline void log_lock(int lock_op, const volatile void* ptr, const char *file, int line, int irq_sync) {
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

    // pointer
    la_buffer.ptr = (unsigned long)ptr;

    // size only relevant for memory access

    // TODO type
    strncpy(la_buffer.type, "dummy", LOG_CHAR_BUFFER_LEN);
	la_buffer.type[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // TODO lock_member
    strncpy(la_buffer.lock_member, "dummy", LOG_CHAR_BUFFER_LEN);
	la_buffer.lock_member[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // file
    strncpy(la_buffer.file,file,LOG_CHAR_BUFFER_LEN);
	la_buffer.file[LOG_CHAR_BUFFER_LEN - 1] = '\0';

    // line
    la_buffer.line = line;

    // TODO function
	strncpy(la_buffer.function,"dummy",LOG_CHAR_BUFFER_LEN);
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
