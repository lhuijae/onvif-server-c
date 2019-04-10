//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// gnuc_thread.c
// 
// Copyright (c) 2006 HaiVision System Inc.	
//				
//    Created by : Francois Oligny-Lemieux
//       Created : 23.Nov.2006
//      Modified : 11.Jun.2007 (to fix uClibc 3.3 bug)
//
//  Description: 
//      portable thread object 
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "toolbox-config.h"

#if C_TOOLBOX_THREAD_AND_MUTEX == 1

#	include "toolbox.h"
#	include "toolbox-errors.h"

#	if ( defined(_MSC_VER) )
	// someone should include windows.h at some point
#		include <windows.h>
//#		include <iostream>
#	else
#		include <string.h>
#		include <pthread.h>
#		include <sys/time.h>
#		include <unistd.h>
#		include <assert.h>
#		include <errno.h>
#	endif

int CMutex_Constructor(CMutex * root, int createAndAcquire)
{
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	root->mutex = CreateMutex( NULL, createAndAcquire, NULL);
	if ( root->mutex )
	{
		root->initialized = 1;
	}
#else
	pthread_mutexattr_t attr;
	pthread_condattr_t cond_attr;
	memset(&attr,0,sizeof(pthread_mutexattr_t));
	memset(&cond_attr,0,sizeof(pthread_condattr_t));
	root->acquired_count = 0;
	root->owner = 0;
	root->waiting = 0;
	pthread_cond_init(&root->cond, &cond_attr);
	pthread_mutexattr_init(&attr);
	//pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE); AHH! not in uClibc 3.3
	memset(&root->mutex, 0x00, sizeof(root->mutex));
	if ( pthread_mutex_init(&root->mutex, &attr)==0 )
	{
#	if GNUC_COND_VAR_BROKEN == 1
		pthread_mutex_init(&root->mutex2, &attr);
#	endif
		if (createAndAcquire)
		{
			root->acquired_count = 1;
			root->owner = pthread_self();
		}
#	if GNUC_COND_VAR_BROKEN == 1
		pthread_mutex_lock(&root->mutex2);
#	endif
		root->initialized = 1;
	}
	pthread_mutexattr_destroy(&attr);
	pthread_condattr_destroy(&cond_attr);
#endif

	return 1;
}	

int CMutex_Destructor(CMutex * root)
{
	if ( root == NULL )
	{
		return -1;
	}
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

#if ( defined(_MSC_VER) )
	CloseHandle(root->mutex);
#else
	pthread_mutex_destroy(&root->mutex);
#	if GNUC_COND_VAR_BROKEN == 1
	pthread_mutex_destroy(&root->mutex2);
#	endif
	pthread_cond_destroy(&root->cond);
#endif

	return 1;
}



int CMutex_Acquire(CMutex * root)
{
	int result = 0;
	
#if ( defined(_MSC_VER) )
	DWORD dwordResult;
#else
	int iret_mutex;
	char temp[128];
	int count = 0;
#endif

	if ( root == NULL )
	{	return -1;
	}
	if ( root->initialized == 0 )
	{	return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

#if ( defined(_MSC_VER) )
	dwordResult = WaitForSingleObject(root->mutex, GNUC_WAIT_FOREVER);
	
	if ( dwordResult == WAIT_OBJECT_0 )
	{	result = 1;
	}
	else if ( dwordResult == WAIT_ABANDONED )
	{	result = 1;
	}
	else if ( dwordResult == WAIT_TIMEOUT
		||	dwordResult == WAIT_FAILED 
			)
	{	result = 0;
	}
	else
	{	result = 0;
	}

#else //PTHREAD
	iret_mutex = pthread_mutex_lock(&root->mutex);
	if ( iret_mutex != 0 )
	{
		printf("Thread(%d) pthread_mutex_lock FAILED with iret(%d)\n", (int)pthread_self(), iret_mutex);
		return TOOLBOX_ERROR_MUTEX_LOCK_FAILED;
	}
	//printf("Thread(%d) Acquire\n", (int)pthread_self());
	if (root->acquired_count != 0)
	{
		if ( root->owner != pthread_self() )
		{
			do {
				root->waiting++;
#if GNUC_COND_VAR_BROKEN == 1
				iret_mutex = pthread_mutex_trylock(&root->mutex2);
				while ( iret_mutex )
				{
					pthread_mutex_unlock(&root->mutex);
					usleep(100);
					pthread_mutex_lock(&root->mutex);
					iret_mutex = pthread_mutex_trylock(&root->mutex2);
				}
#else				
				pthread_cond_wait(&root->cond, &root->mutex);
#endif
				root->waiting--;
				//printf("Thread(%d) Acquire COND success\n", (int)pthread_self());
				count++;
				if ( count > 300 )
				{
					exit(0);
				}	
			} while(root->acquired_count != 0);
		}
	}
	root->owner = pthread_self();
	root->acquired_count++;
	result = 1;
	//printf("Thread(%d) Acquire going to unlock by success, root->acquired_count(%d)\n", (int)pthread_self(), root->acquired_count);
	pthread_mutex_unlock(&root->mutex);
	//printf("Thread(%d) Acquire unlock by success, root->acquired_count(%d)\n", (int)pthread_self(), root->acquired_count);
 #endif

	return result;
}

int CMutex_AcquireTimeout(CMutex * root, long timeout_ms)
{	
#if ( defined(_MSC_VER) )
	DWORD dwordResult;
	int result = 0;
#else
	char temp[128];
	int iret;
	int count = 0;
	int iret_mutex;
#endif
	
	if ( root == NULL )
	{
		return -1;
	}
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

#if ( defined(_MSC_VER) )
	dwordResult = WaitForSingleObject(root->mutex, timeout_ms);
	
	if ( dwordResult == WAIT_OBJECT_0 )
	{	result = 1;
	}
	else if ( dwordResult == WAIT_ABANDONED )
	{	result = 1;
	}
	else if ( dwordResult == WAIT_TIMEOUT
		||	dwordResult == WAIT_FAILED 
			)
	{	result = 0;
	}
	else
	{	result = 0;
	}
	return result;
#else
	iret_mutex = pthread_mutex_lock(&root->mutex);
	if ( iret_mutex != 0 )
	{
		printf("Thread(%d) ATimeout pthread_mutex_lock FAILED with iret(%d)\n", (int)pthread_self(), iret_mutex);
		return TOOLBOX_ERROR_MUTEX_LOCK_FAILED;
	}
	printf("Thread(%d) AcquireTimeout lock\n", (int)pthread_self());
	if (root->acquired_count != 0)
	{
		if ( root->owner != pthread_self() )
		{
			struct timespec sTime;
			struct timeval sNow;
#if GNUC_COND_VAR_BROKEN == 1
			struct timeval endTime;
#endif
			printf("Thread(%d) owner is not us, going to gettimeofday\n", (int)pthread_self());
			gettimeofday(&sNow, NULL);
#if GNUC_COND_VAR_BROKEN == 1
			endTime.tv_sec = sNow.tv_sec + timeout_ms/1000;
			endTime.tv_usec = sNow.tv_usec + timeout_ms%1000;
			if (endTime.tv_usec >= 1000000)
			{
				endTime.tv_usec -= 1000000;
				endTime.tv_sec++;
			}
#else
			sTime.tv_sec = sNow.tv_sec + timeout_ms/1000;
			sTime.tv_nsec = (sNow.tv_usec)*1000 + (timeout_ms%1000)*1000000;
			//sTime.tv_nsec += 100;  // +100 is to prevent a bug when timeout_ms is zero (frank)
			if (sTime.tv_nsec >= 1000000000)
			{
				sTime.tv_nsec -= 1000000000;
				sTime.tv_sec++;
			}
#endif
			do {
				if (timeout_ms == GNUC_WAIT_FOREVER)
				{
					printf("Thread(%d) AcquireTimeout going to wait forever\n", (int)pthread_self());
					root->waiting++;
#if GNUC_COND_VAR_BROKEN == 1
					iret = pthread_mutex_trylock(&root->mutex2);
					while ( iret )
					{
						pthread_mutex_unlock(&root->mutex);
						usleep(100);
						pthread_mutex_lock(&root->mutex);
						iret = pthread_mutex_trylock(&root->mutex2);
					}
#else
					pthread_cond_wait(&root->cond, &root->mutex);
#endif
					root->waiting--;
				}
				else
				{
					root->waiting++;
#if GNUC_COND_VAR_BROKEN == 1
					printf("Thread(%d) going to a pthread_mutex_trylock for ms(%ld)\n", (int)pthread_self(), timeout_ms);
					iret = pthread_mutex_trylock(&root->mutex2);
					printf("Thread(%d) pthread_mutex_trylock iret(%d)\n", (int)pthread_self(), iret);
					while ( iret )
					{
						gettimeofday(&sNow, NULL);
						if ( sNow.tv_sec > endTime.tv_sec 
						  || ( sNow.tv_sec==endTime.tv_sec && sNow.tv_usec>=endTime.tv_usec )
						   )
						{
							root->waiting--;
							pthread_mutex_unlock(&root->mutex);
							return 0; // timeout
						}
						pthread_mutex_unlock(&root->mutex);
						usleep(8000); // 8ms
						pthread_mutex_lock(&root->mutex);
						iret = pthread_mutex_trylock(&root->mutex2);						
						printf("Thread(%d) pthread_mutex_trylock iret(%d), EBUSY(%d), EINVAL(%d), EFAULT (%d)\n", (int)pthread_self(), iret, EBUSY, EINVAL, EFAULT );
					}
					printf("Thread(%d) out of pthread_mutex_trylock loop\n", (int)pthread_self());
					root->waiting--;
#else
					printf("Thread(%d) going to a pthread_cond_timedwait for ms(%ld)\n", (int)pthread_self(), timeout_ms);
					iret = pthread_cond_timedwait(&root->cond, &root->mutex, &sTime);
					root->waiting--;
					if ( iret == ETIMEDOUT ) 
					{
						//printf("Thread(%d) AcquireTimeout going to unlock by timeout\n", (int)pthread_self());
						pthread_mutex_unlock(&root->mutex);
						//printf("Thread(%d) AcquireTimeout unlock by timeout\n", (int)pthread_self());
						return 0;
					}
#endif
				}
			} while ( root->acquired_count > 0 );
		}
	}
	root->owner = pthread_self();
	root->acquired_count++;
	pthread_mutex_unlock(&root->mutex);
	printf("Thread(%d) AcquireTimeout unlock success, busy(%d)\n", (int)pthread_self(), root->acquired_count);

#endif

	return 1;
}

int CMutex_Release(CMutex * root)
{
	if ( root == NULL )
	{
		return -1;
	}
	if ( root->initialized == 0 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

#if ( defined(_MSC_VER) )
	ReleaseMutex(root->mutex);
#else
	int iret_mutex;
	char temp[128];
	if ( root->owner != pthread_self() )
	{	printf("ERROR, Thread(%d) calling release without ownership.\n", (int)pthread_self());
		return -10;
	}
	//printf("Thread(%d) going to lock for release..\n", (int)pthread_self());
	iret_mutex = pthread_mutex_lock(&root->mutex);
	if ( iret_mutex != 0 )
	{
		printf("Thread(%d) Release pthread_mutex_lock FAILED with iret(%d)\n", (int)pthread_self(), iret_mutex);
		// frank fixme should I do while(1) ?
		return TOOLBOX_ERROR_MUTEX_LOCK_FAILED;
	}
	//printf("Thread(%d) locked for release\n", (int)pthread_self());
	root->acquired_count--;
	if ( root->acquired_count < 0 )
	{
		printf("release, root->acquired_count is negative\n");
		exit(0);
	}
	//printf("Thread(%d) unlocked for release root->acquired_count(%d)\n", (int)pthread_self(), root->acquired_count);
	if (root->acquired_count == 0 && root->waiting )
	{
		//printf("Thread(%d) SIGNAL cond root->acquired_count(%d)\n", (int)pthread_self(), root->acquired_count);
#if GNUC_COND_VAR_BROKEN == 1
		pthread_mutex_unlock(&root->mutex2);
#else
		pthread_cond_signal(&root->cond);
#endif
	}
	pthread_mutex_unlock(&root->mutex);
#endif

	return 1;
}

#endif //GNUC_TOOLBOX_THREAD_AND_MUTEX == 1
