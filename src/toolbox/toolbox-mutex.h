#ifndef _GNUC_MUTEX_H_
#define _GNUC_MUTEX_H_

#define GNUC_WAIT_FOREVER 0xFFFFFFFF

#ifdef __cplusplus
extern "C" {
#endif

#if ( defined(_MSC_VER) )
//#	define INFINITE 0xFFFFFFFF  // Infinite timeout
#else
#	include <pthread.h>
#	if __GNUC__ == 3 && __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ == 4 // && defined(_UCLIBC_PTHREAD_H)
//#		warning "GNUC_COND_VAR_BROKEN == 1"
#		define GNUC_COND_VAR_BROKEN 1
#	endif
#endif


// public structures
typedef struct CMutex_S
{
	int initialized;
	
#if ( defined(_MSC_VER) )
	HANDLE mutex;	
#else
    pthread_mutex_t mutex;
#	if GNUC_COND_VAR_BROKEN == 1
	pthread_mutex_t mutex2;
#	endif
    pthread_cond_t cond;
    int acquired_count; // recursive count of the acquire calls made by the same owning thread
	int waiting; // amount of people waiting for SIGNAL
    pthread_t owner;
#endif

} CMutex;


int CMutex_Constructor(CMutex * root, int createAndAcquire);
int CMutex_Destructor(CMutex * root);
int CMutex_AcquireTimeout(CMutex * root, long timeout_ms);
int CMutex_Acquire(CMutex * root); // wait forever
int CMutex_Release(CMutex * root);

#ifdef __cplusplus
}
#endif


#endif // _GNUC_MUTEX_H_
