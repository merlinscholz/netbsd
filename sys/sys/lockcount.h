#if !defined(__LOCKCOUNT_H__) && !defined(__ASSEMBLER__)
#define __LOCKCOUNT_H__

#include <arch/x86/include/cpufunc.h>

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/lockdebug.h>

#ifdef LOCKCOUNT

extern u_int64_t lockcounter;

// For DEFINEs in lockdebug.h
// Implemented in lockcount/log.c
void	lockcount_locked(void);

#endif /* LOCKCOUNT */

#endif /* __LOCKCOUNT_H__ */
