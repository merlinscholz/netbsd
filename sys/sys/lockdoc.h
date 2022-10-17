#if !defined(__LOCKDOC_H__) && !defined(__ASSEMBLER__)
#define __LOCKDOC_H__

#include <sys/types.h>
#include <sys/lockdoc_event.h>

#define DELIMITER	"#"
#define DELIMITER_CHAR	'#'
#define PING_CHAR	'p'
#define MK_STRING(x)	#x
#define IO_PORT_LOG	(0x00e9)

#ifdef LOCKDEBUG

extern struct log_action la_buffer;

/* Basic port I/O */
static inline void outb_(u_int8_t v, u_int16_t port)
{
	__asm __volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

static inline void log_memory(int alloc, const char *datatype, const void *ptr, size_t size) {

}

static inline void log_lock(int lock_op, const kmutex_t* ptr, const char *file, int line, int irq_sync) {
    memset(&la_buffer,0,sizeof(la_buffer));
    outb_(PING_CHAR,IO_PORT_LOG);
}
#else
#define log_lock(a, b, c, d, e)

#endif // !LOCKDEBUG

#endif // __LOCKDOC_H__
