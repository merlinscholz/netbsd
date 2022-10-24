#if !defined(__LOCKDOC_H__) && !defined(__ASSEMBLER__)
#define __LOCKDOC_H__

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/lockdoc_event.h>

#define DELIMITER	"#"
#define DELIMITER_CHAR	'#'
#define PING_CHAR	'p'
#define MK_STRING(x)	#x
#define IO_PORT_LOG	(0x00e9)

#ifdef LOCKDOC

extern struct log_action la_buffer;

/* Basic port I/O */
static inline void outb_(u_int8_t v, u_int16_t port)
{
	__asm __volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline void log_memory(int alloc, const char *datatype, const void *ptr, size_t size) {

}

static inline void log_lock(int lock_op, const void* ptr, const char *file, int line, int irq_sync) {

    memset(&la_buffer,0,sizeof(la_buffer));

    //strncpy((char *)&la_buffer, "abcdefgh", LOG_CHAR_BUFFER_LEN);

    // action
    la_buffer.action = LOCKDOC_LOCK_OP;

    // operation
    la_buffer.lock_op = lock_op;

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
    // TODO irq_sync
    // TODO flags

    outb_(PING_CHAR,IO_PORT_LOG);
}
#else
#define log_lock(a, b, c, d, e)

#endif /* LOCKDOC */

#endif /* __LOCKDOC_H__ */
