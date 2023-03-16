#include <arch/x86/include/cpufunc.h>
#include <sys/lwp.h>
#include <sys/lockdebug.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

struct log_action la_buffer;

int32_t lockdoc_get_ctx(void) {
	if (curlwp == NULL) {
		return 0;
	} else if (curlwp->l_pflag & LP_INTR) {
		return -1;
	} else {
		return curlwp->l_lid;
	}
}

void lockdoc_send_current_task_addr(void) {
	u_long flags;

	flags = lockdoc_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_CURRENT_TASK;
	la_buffer.ptr = (uint32_t)((uint32_t)curcpu()->ci_self + offsetof(struct cpu_info, ci_curlwp));

	outb_(PING_CHAR,IO_PORT_LOG);

	lockdoc_x86_restore_intr(flags);
}

void lockdoc_send_lwp_flag_offset(void) {
	u_long flags;

	flags = lockdoc_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_LWP_FLAG_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_pflag);
	outb_(PING_CHAR,IO_PORT_LOG);

	lockdoc_x86_restore_intr(flags);
}

void lockdoc_send_pid_offset(void) {
	u_long flags;

	flags = lockdoc_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_PID_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_lid);
	outb_(PING_CHAR,IO_PORT_LOG);

	lockdoc_x86_restore_intr(flags);
}

void lockdoc_send_kernel_version(void) {
	u_long flags;

	flags = lockdoc_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	snprintf((char*)&la_buffer.type, LOG_CHAR_BUFFER_LEN, "%s", "notimplemented"); // TODO Implement
	la_buffer.action = LOCKDOC_KERNEL_VERSION;
	outb_(PING_CHAR,IO_PORT_LOG);

	lockdoc_x86_restore_intr(flags);
}

void trace_irqs_on(struct trapframe *frame) {
	u_long cur_eflags;
	cur_eflags = x86_read_flags();
	if ((frame->tf_eflags & (1 << 9)) && !(cur_eflags & (1 << 9))) {
#ifdef DEBUG_TRAPS
		lockdoc_log_lock(V_WRITE, (struct lock_object*)PSEUDOLOCK_ADDR_HARDIRQ, __FILE__, __LINE__, frame->tf_trapno);
#else
		lockdoc_log_lock(V_WRITE, (struct lock_object*)PSEUDOLOCK_ADDR_HARDIRQ, __FILE__, __LINE__, LOCK_NONE);
#endif
	}
}

void trace_irqs_off(struct trapframe *frame) {
	u_long cur_eflags;
	cur_eflags = x86_read_flags();
	if ((frame->tf_eflags & (1 << 9)) && !(cur_eflags & (1 << 9))) {
#ifdef DEBUG_TRAPS
		lockdoc_log_lock(P_WRITE, (struct lock_object*)PSEUDOLOCK_ADDR_HARDIRQ, __FILE__, __LINE__, frame->tf_trapno);
#else
		lockdoc_log_lock(P_WRITE, (struct lock_object*)PSEUDOLOCK_ADDR_HARDIRQ, __FILE__, __LINE__, LOCK_NONE);
#endif
	}
}

void __x86_disable_intr(const char *file, int line, const char *func){
    u_long eflags;
    
    eflags = x86_read_flags();
    if (eflags & (1 << 9)){  // Check if interrupts were enabled in the first place
		lockdoc_log_lock(P_WRITE, (void*)PSEUDOLOCK_ADDR_HARDIRQ, file, line, LOCK_NONE); 
    }
    lockdoc_x86_disable_intr();
}

u_long lockdoc_x86_disable_intr(void){
    u_long eflags;

    eflags = x86_read_flags();
	__asm volatile ("cli" ::: "memory");
    return eflags;

}

void __x86_enable_intr(const char *file, int line, const char *func){
    u_long eflags;
    
    eflags = x86_read_flags();
    if (!(eflags & (1 << 9))){  // Check if interrupts were disabled in the first place
		lockdoc_log_lock(V_WRITE, (void*)PSEUDOLOCK_ADDR_HARDIRQ, file, line, LOCK_NONE); 
    }
    lockdoc_x86_enable_intr();
}

u_long lockdoc_x86_enable_intr(void){
    u_long eflags;

    eflags = x86_read_flags();
	__asm volatile ("sti" ::: "memory");
    return eflags;
}

void lockdoc_x86_restore_intr(u_long eflags){
    x86_write_flags(eflags);
}

#endif /* LOCKDOC */