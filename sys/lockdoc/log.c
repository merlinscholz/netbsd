#include <arch/x86/include/cpufunc.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

struct log_action la_buffer;

void __x86_disable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_disable_intr();
}

void __x86_enable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_enable_intr();
}

#endif /* LOCKDOC */