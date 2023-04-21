#include <arch/x86/include/cpufunc.h>
#include <arch/i386/include/pio.h>

#include <sys/lwp.h>
#include <sys/lockdoc.h>

#ifdef LOCKDOC

void lockdoc_send_current_task_addr(void) {
	u_long flags;

	flags = x86_read_flags();
    _x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_CURRENT_TASK;
	la_buffer.ptr = (uint32_t)((uint32_t)curcpu()->ci_self + offsetof(struct cpu_info, ci_curlwp));

	lockdoc_outb(PING_CHAR,IO_PORT_LOG);

	x86_write_flags(flags);
}

void lockdoc_send_lwp_flag_offset(void) {
	u_long flags;

	flags = x86_read_flags();
    _x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_LWP_FLAG_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_pflag);
	lockdoc_outb(PING_CHAR,IO_PORT_LOG);

	x86_write_flags(flags);
}

void lockdoc_send_pid_offset(void) {
	u_long flags;

	flags = x86_read_flags();
    _x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_PID_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_lid);
	lockdoc_outb(PING_CHAR,IO_PORT_LOG);

	x86_write_flags(flags);
}

void lockdoc_send_kernel_version(void) {
	u_long flags;

	flags = x86_read_flags();
    _x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	snprintf((char*)&la_buffer.type, LOG_CHAR_BUFFER_LEN, "%s", "notimplemented"); // TODO Implement
	la_buffer.action = LOCKDOC_KERNEL_VERSION;
	lockdoc_outb(PING_CHAR,IO_PORT_LOG);

	x86_write_flags(flags);
}

#endif