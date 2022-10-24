#include <arch/x86/include/cpufunc.h>
#include <sys/lockdoc.h>

#include "opt_lockdebug.h"

#ifdef LOCKDEBUG

struct log_action la_buffer;

void __x86_disable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_disable_intr();
    __asm __volatile("cli" : : : "memory");
}

void __x86_enable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_enable_intr();
	__asm __volatile ("sti" ::: "memory");
}

#endif /* LOCKDEBUG */