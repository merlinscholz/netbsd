#include <arch/x86/include/cpufunc.h>
#include <sys/lockdebug.h>
#include <sys/lockcount.h>

#ifdef LOCKCOUNT

u_int64_t lockcounter;

void lockcount_locked(void){
    lockcounter += 1;
}

#endif /* LOCKCOUNT */