#include <arch/x86/include/cpufunc.h>
#include <sys/lockdebug.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

struct log_action la_buffer;

bool lockdoc_alloc(const char *func, size_t line, volatile void *lock, lockops_t *lo, uintptr_t initaddr){
    // TODO Actually log things
    return lockdebug_alloc(func, line, lock, lo, initaddr);
}
void lockdoc_free(bool dodebug, const char *func, size_t line, volatile void *lock){
    // TODO Actually log things
    if (dodebug) lockdebug_free(func, line, lock);
}
void lockdoc_wantlock(bool dodebug, const char *func, size_t line, const volatile void *lock, uintptr_t where, int shared){
    // TODO Actually log things
    if (dodebug) lockdebug_wantlock(func, line, lock, where, shared);
}
void lockdoc_locked(bool dodebug, const char *func, size_t line, volatile void *lock, void *cvlock, uintptr_t where, int shared){
    // TODO Actually log things
    if (dodebug) lockdebug_locked(func, line, lock, cvlock, where, shared);
}
void lockdoc_unlocked(bool dodebug, const char *func, size_t line, volatile void *lock, uintptr_t where, int shared){
    // TODO Actually log things
    if (dodebug) lockdebug_unlocked(func, line, lock, where, shared);
}
void lockdoc_barrier(const char *func, size_t line, volatile void *spinlock, int slplocks){
    // TODO Actually log things
    lockdebug_barrier(func, line, spinlock, slplocks);
}
void lockdoc_mem_check(const char *func, size_t line, void *base, size_t sz){
    // TODO Actually log things
    lockdebug_mem_check(func, line, base, sz);
}
void lockdoc_wakeup(bool dodebug, const char *func, size_t line, volatile void *lock, uintptr_t where){
    // TODO Actually log things
    if (dodebug) lockdebug_wakeup(func, line, lock, where);
}

void __x86_disable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_disable_intr();
}

void __x86_enable_intr(const char *file, int line, const char *func){
    // TODO Actually log things
    lockdoc_x86_enable_intr();
}



#endif /* LOCKDOC */