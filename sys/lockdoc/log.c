#include <arch/x86/include/cpufunc.h>
#include <sys/lockdebug.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

struct log_action la_buffer;

void lockdoc_alloc(const char *func, const char *file, size_t line, volatile void *lock, lockops_t *lo, uintptr_t initaddr){
    // TODO Actually log things
}
void lockdoc_free(const char *func, const char *file, size_t line, volatile void *lock){
    // TODO Actually log things
}
void lockdoc_wantlock(const char *func, const char *file, size_t line, const volatile void *lock, uintptr_t where, int shared){
    // TODO Actually log things
}
void lockdoc_locked(const char *func, const char *file, size_t line, volatile void *lock, void *cvlock, uintptr_t where, int shared){
    // shared == 0 means exclusive (write) lock; shared > 0 means shared (read) lock
    if(shared > 0) {
        // TODO Check if LOCK_NONE is appropriate
        log_lock(P_READ, lock, file, line, LOCK_NONE);
    } else if (shared == 0){
        log_lock(P_WRITE, lock, file, line, LOCK_NONE);
    }
}
void lockdoc_unlocked(const char *func, const char *file, size_t line, volatile void *lock, uintptr_t where, int shared){
    // shared == 0 means exclusive (write) lock; shared > 0 means shared (read) lock
    if(shared > 0) {
        // TODO Check if LOCK_NONE is appropriate
        log_lock(V_READ, lock, file, line, LOCK_NONE);
    } else if (shared == 0){
        log_lock(V_WRITE, lock, file, line, LOCK_NONE);
    }
}
void lockdoc_barrier(const char *func, const char *file, size_t line, volatile void *spinlock, int slplocks){
    // TODO Actually log things
}
void lockdoc_mem_check(const char *func, const char *file, size_t line, void *base, size_t sz){
    // TODO Actually log things
}
void lockdoc_wakeup(const char *func, const char *file, size_t line, volatile void *lock, uintptr_t where){
    // TODO Actually log things
}

void __x86_disable_intr(const char *file, int line, const char *func){
    u_long eflags;
    
    eflags = x86_read_flags();
    if (eflags & (1 << 9)){  // Check if interrupts were enabled in the first place
		log_lock(P_WRITE, (void*)PSEUDOLOCK_ADDR_HARDIRQ, file, line, LOCK_NONE); 
    }
    lockdoc_x86_disable_intr();
}

void __x86_enable_intr(const char *file, int line, const char *func){
    u_long eflags;
    
    eflags = x86_read_flags();
    if (!(eflags & (1 << 9))){  // Check if interrupts were disabled in the first place
		log_lock(V_WRITE, (void*)PSEUDOLOCK_ADDR_HARDIRQ, file, line, LOCK_NONE); 
    }
    lockdoc_x86_disable_intr();
}

#endif /* LOCKDOC */