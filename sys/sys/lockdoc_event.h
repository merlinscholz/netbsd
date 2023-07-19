#ifndef __LOCKDOC_EVENT_H__
#define __LOCKDOC_EVENT_H__

#ifdef LOCKDOC

#define LOCKDOC_LOG_CHAR_BUFFER_LEN 100
#define PSEUDOLOCK_ADDR_RCU		0x42
#define PSEUDOLOCK_ADDR_PREEMPT	0x43
#define PSEUDOLOCK_ADDR_SOFTIRQ	0x45
#define PSEUDOLOCK_ADDR_HARDIRQ	0x44
#define PSEUDOLOCK_NAME_RCU		"rcu"
#define PSEUDOLOCK_NAME_HARDIRQ	"hardirq"
#define PSEUDOLOCK_NAME_SOFTIRQ	"softirq"
#define PSEUDOLOCK_NAME_PREEMPT	"preempt"
#define PSEUDOLOCK_VAR			"static"
#define PSEUDOLOCK_VAR_RCU		PSEUDOLOCK_VAR
#define PSEUDOLOCK_VAR_HARDIRQ	PSEUDOLOCK_VAR
#define PSEUDOLOCK_VAR_SOFTIRQ	PSEUDOLOCK_VAR
#define PSEUDOLOCK_VAR_PREEMPT	PSEUDOLOCK_VAR

/*
 * When altered, this file
 * has to be copied to the according fail experiment
 * as well as to convert!
 */
enum LOCKDOC_OP {
	LOCKDOC_ALLOC			= 'a',
	LOCKDOC_FREE			= 'f',
	LOCKDOC_LOCK_OP			= 'l',
	LOCKDOC_CURRENT_TASK	= 't',
	LOCKDOC_PREEMPT_COUNT	= 'p',
	LOCKDOC_PID_OFFSET		= 'o',
	LOCKDOC_KERNEL_VERSION	= 'v',
	LOCKDOC_READ			= 'r',
	LOCKDOC_WRITE			= 'w',
	LOCKDOC_IRQ_NEST_OFFSET = 'n',
	LOCKDOC_LWP_FLAG_OFFSET = 'g'
};

enum LOCK_OP {
	P_READ = 0, // Take shared lock
	P_WRITE,    // Take excl.  lock
	V_READ,     // Free shared lock
	V_WRITE     // Free excl.  lock
};

enum IRQ_SYNC {
	LOCK_NONE = 0,
	LOCK_IRQ,
	LOCK_IRQ_NESTED,  /* aka irqsave and irqrestore */
	LOCK_BH
};

struct log_action {
	enum LOCKDOC_OP action;
	int32_t ctx;
	uint32_t lock_op;
	uint32_t ptr;
	uint32_t size;
	char type[LOCKDOC_LOG_CHAR_BUFFER_LEN]; // allocated data_type or lock type
	char lock_member[LOCKDOC_LOG_CHAR_BUFFER_LEN];
	char file[LOCKDOC_LOG_CHAR_BUFFER_LEN];
	int32_t line;
	char function[LOCKDOC_LOG_CHAR_BUFFER_LEN]; 
	int32_t preempt_count;
	int32_t irq_sync;
	int32_t flags;
}__attribute__((packed));

#endif /* LOCKDOC */

#endif /* __LOCKDOC_EVENT_H__ */
