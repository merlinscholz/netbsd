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
	LOCKDOC_PING();

	x86_write_flags(flags);
}

void lockdoc_send_lwp_flag_offset(void) {
	u_long flags;

	flags = x86_read_flags();
   	_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_LWP_FLAG_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_pflag);
	LOCKDOC_PING();

	x86_write_flags(flags);
}

void lockdoc_send_pid_offset(void) {
	u_long flags;

	flags = x86_read_flags();
    	_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	la_buffer.action = LOCKDOC_PID_OFFSET;
	la_buffer.ptr = offsetof(struct lwp, l_lid);
	LOCKDOC_PING();

	x86_write_flags(flags);
}

void lockdoc_send_kernel_version(void) {
	u_long flags;

	flags = x86_read_flags();
    	_x86_disable_intr();

	memset(&la_buffer,0,sizeof(la_buffer));

	snprintf((char*)&la_buffer.type, LOCKDOC_LOG_CHAR_BUFFER_LEN,
		"%d-lockdoc-%s", __NetBSD_Version__, LOCKDOC_VERSION);
	la_buffer.action = LOCKDOC_KERNEL_VERSION;
	LOCKDOC_PING();

	x86_write_flags(flags);
}

#endif