#include <arch/x86/include/cpufunc.h>
#include <sys/lwp.h>
#include <sys/lockdebug.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

struct log_action la_buffer;

#define LOCK_TYPE "kmutex_t"

// Mutexes are always considered write locks
inline void __mutex_enter(kmutex_t * lock, const char* file, int line, const char* func) {
	_mutex_enter(lock);
	lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
}

inline void __mutex_exit(kmutex_t * lock, const char* file, int line, const char* func) {
	lockdoc_log_lock(V_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	_mutex_exit(lock);
}

inline void __mutex_spin_enter(kmutex_t * lock, const char* file, int line, const char* func) {
	_mutex_spin_enter(lock);
	lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
}

inline void __mutex_spin_exit(kmutex_t * lock, const char* file, int line, const char* func) {
	lockdoc_log_lock(V_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	_mutex_spin_exit(lock);
}

inline int __mutex_tryenter(kmutex_t * lock, const char* file, int line, const char* func) {
	int enter = _mutex_tryenter(lock);
	if(enter != 0) {
		lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	}
	return enter;
}

#undef LOCK_TYPE
#define LOCK_TYPE "krwlock_t"

inline int __rw_tryupgrade(krwlock_t * lock, const char* file, int line, const char* func) {
	int upgrade = _rw_tryupgrade(lock);
	if(upgrade != 0) {
		lockdoc_log_lock(V_READ, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
		lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	}
	return upgrade;
}

inline void __rw_downgrade(krwlock_t * lock, const char* file, int line, const char* func) {
	_rw_downgrade(lock);
	lockdoc_log_lock(V_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	lockdoc_log_lock(P_READ, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
}

inline void __rw_enter(krwlock_t * lock, const krw_t type, const char* file, int line, const char* func) {
	_rw_enter(lock, type);
	if(type == RW_READER)
		lockdoc_log_lock(P_READ, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	else
		lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
}

inline int __rw_tryenter(krwlock_t * lock, const krw_t type, const char* file, int line, const char* func) {
	int enter = _rw_tryenter(lock, type);
	if(enter != 0) {
		if(type == RW_READER)
			lockdoc_log_lock(P_READ, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
		else
			lockdoc_log_lock(P_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	}
	return enter;
}

inline void __rw_exit(krwlock_t * lock, const char* file, int line, const char* func) {
	/*
	 * See sys/sys/rwlock.h
	 * We cannot include this directly due to circular dependencies.
	 */
	if(rw_lock_op(lock) == RW_WRITER)
		lockdoc_log_lock(V_WRITE, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	else
		lockdoc_log_lock(V_READ, lock, file, line, func, LOCK_TYPE, LOCK_NONE);
	_rw_exit(lock);
}
#undef LOCK_TYPE

inline void __x86_disable_intr(const char* file, int line, const char* func) {
	u_long eflags;
    
    eflags = x86_read_flags();
    if (eflags & (1 << 9)){  // Check if interrupts were enabled in the first place
		lockdoc_log_lock(P_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", LOCK_NONE); 
    }

	_x86_disable_intr();
}

inline void __x86_enable_intr(const char* file, int line, const char* func) {
	u_long eflags;
    
    eflags = x86_read_flags();
    if (!(eflags & (1 << 9))){  // Check if interrupts were disabled in the first place
		lockdoc_log_lock(V_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", LOCK_NONE); 
    }
	
	_x86_enable_intr();
}

int32_t lockdoc_get_ctx(void) {
	if (curlwp == NULL) {
		return 0;
	} else if (curlwp->l_pflag & LP_INTR) {
		return -1;
	} else {
		return curlwp->l_lid;
	}
}

void __trace_irqs_on(struct trapframe *frame, const char *file, int line) {
	lockdoc_log_lock(V_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", LOCK_NONE);
}

void __trace_irqs_on_check(struct trapframe *frame, const char *file, int line) {
	u_long cur_eflags;
	cur_eflags = x86_read_flags();
	if ((frame->tf_eflags & (1 << 9)) && !(cur_eflags & (1 << 9))) {
		lockdoc_log_lock(V_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", LOCK_NONE);
	}
}

void __trace_irqs_off(struct trapframe *frame, const char *file, int line) {
	lockdoc_log_lock(P_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", frame->tf_trapno);
}

void __trace_irqs_off_check(struct trapframe *frame, const char *file, int line) {
	u_long cur_eflags;
	cur_eflags = x86_read_flags();
	if ((frame->tf_eflags & (1 << 9)) && !(cur_eflags & (1 << 9))) {
		lockdoc_log_lock(P_WRITE, (void *)PSEUDOLOCK_ADDR_HARDIRQ, file, line, "dummy", "dummy", frame->tf_trapno);
	}
}

#endif /* LOCKDOC */