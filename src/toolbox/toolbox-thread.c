//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// gnuc_thread.c
// 
// Copyright (c) 2006 HaiVision System Inc.	
//				
// Maintained by : Francois Oligny-Lemieux
//       Created : 23.Nov.2006
//      Modified : 21.Oct.2008 (Using _beginthreadex instead of CreateThread to fix memory leak issues, msdn:Q104641)
//
//  Description: 
//      portable thread object 
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "toolbox-config.h"

#if GNUC_TOOLBOX_THREAD_AND_MUTEX == 1 

#	include "toolbox.h"
#	include "toolbox-basic-types.h"

#	if ( defined(_MSC_VER) )
	// windows specific include
//#		include <windows.h>
#		include <process.h>
#	else
	// linux specific include
#		include <pthread.h>
#		include <signal.h>
#	endif


static int priority_values[] = 
{
	-15,
	-2,
	-1,
	0,
	1,
	2,
	15
};


int GNUCThread_getId(unsigned int * loadme)
{
	if ( loadme == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	//*loadme = GetThreadId(NULL);
	*loadme = GetCurrentThreadId();
#else
	*loadme = pthread_self();
#endif
	return 1;
}


/* not needed
enum Win32PriorityClasses
{
	ABOVE_NORMAL_PRIORITY_CLASS = 0x00008000,
	BELOW_NORMAL_PRIORITY_CLASS = 0x00004000,
	HIGH_PRIORITY_CLASS = 0x00000080,
	IDLE_PRIORITY_CLASS = 0x00000040,
	NORMAL_PRIORITY_CLASS = 0x00000020,
	PROCESS_MODE_BACKGROUND_BEGIN = 0x00100000; // non Windows XP compatible
	PROCESS_MODE_BACKGROUND_END = 0x00200000; // non Windows XP compatible
	REALTIME_PRIORITY_CLASS = 0x00000100;
};
*/

const unsigned int k_num_priorities = sizeof(priority_values)/sizeof(int);

int GNUCThread_Constructor(GNUCThread * root)
{
	
#if ( defined(_MSC_VER) )
#else
	int iret;
#endif

	if ( root == NULL )
	{
		return -1;
	}

	root->threadId = 0;
	root->function = NULL;
	root->opaque = NULL;
	
#if ( defined(_MSC_VER) )	
	root->threadHandle = NULL;	   
#else
    root->threadHandle = (pthread_t) NULL;
    root->suspended = 0;
	iret = CMutex_Constructor(&root->suspendMutex, 1);
	if ( iret <= 0 )
	{
		// failed creating mutex
		//root->suspendMutex = 0;
		return -1;
	}
#endif

	return 1;
}

int GNUCThread_Destructor(GNUCThread * root)
{
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
    if ( root->threadHandle )
    {
	    CloseHandle(root->threadHandle);
		root->threadHandle = NULL;
	}
#else
	CMutex_Destructor(&root->suspendMutex);
	root->initialized = 0;
	root->opaque = NULL;
#endif

	return 1;
}


#if ( defined(_MSC_VER) )
unsigned long __stdcall 
#else
void *
#endif
GNUCThread_FunctionLauncher( void * gnuc_thread_structure )
{
	GNUCThread * thread;

	if ( gnuc_thread_structure == NULL )
	{
#if defined(_MSC_VER)
		return -1;
#else
		return NULL;
#endif
	}

    thread = (GNUCThread *) gnuc_thread_structure;

    thread->function(thread->opaque);

#if ( defined(_MSC_VER) )
	return 0;
#else
	return NULL;
#endif
}

int GNUCThread_Create(GNUCThread * root, gnuc_thread_function function, void * opaque)
{
	int result = -1;
	
	if ( root == NULL ) 
	{
		return -1;
	}

    root->function = function;
    root->opaque = opaque;

#if ( defined(_MSC_VER) )

	//root->threadHandle = CreateThread(NULL, 0, GNUCThread_FunctionLauncher, root, 0, &root->threadId); //frank fixme, use _beginthreadex
	root->threadHandle = (HANDLE)_beginthreadex(NULL, 0, GNUCThread_FunctionLauncher, root, 0, &root->threadId); 
	
	if ( root->threadHandle )
	{
		result = 1;
	}

#else
    if ( pthread_create(&root->threadHandle, NULL, GNUCThread_FunctionLauncher, root) )
    {
		result = -1;
    }
	else
	{
		result = 1;
	}
#endif
	
	return result;
}

int GNUCThread_Destroy(GNUCThread * root)
{	
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	//TerminateThread(root->threadHandle, 0);
	return TOOLBOX_ERROR_FEATURE_NOT_IMPLEMENTED;
#else
	pthread_cancel(root->threadHandle);
#endif

	return 1;
}

int GNUCThread_Suspend(GNUCThread * root)
{	
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	 //SuspendThread(root->threadHandle);
	 return TOOLBOX_ERROR_FEATURE_NOT_IMPLEMENTED;
#else
    CMutex_Acquire(&root->suspendMutex);
    if ( root->suspended==0 ) 
	{
		pthread_kill(root->threadHandle, SIGSTOP);
		root->suspended = 1;
    }
    CMutex_Release(&root->suspendMutex);
#endif

	return 1;
}

int GNUCThread_Resume(GNUCThread * root)
{
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	//ResumeThread(root->threadHandle);
	return TOOLBOX_ERROR_FEATURE_NOT_IMPLEMENTED;
#else
	CMutex_Acquire(&root->suspendMutex);
    if ( root->suspended ) 
	{
		pthread_kill(root->threadHandle, SIGCONT);
		root->suspended = 0;
    }
    CMutex_Release(&root->suspendMutex);
#endif

	return 1;
}

int GNUCThread_Join(GNUCThread * root)
{	
	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	WaitForSingleObject(root->threadHandle, INFINITE);
#else
    pthread_join(root->threadHandle,NULL);
#endif	

	return 1;
}

unsigned int GNUCThread_GetPriority(GNUCThread * root)
{
	int priority;
	unsigned int i;

	if ( root == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
    priority = GetThreadPriority(root->threadHandle);

	if ( k_num_priorities < 0 )
	{
		return -10; // internal error ver bad, probably corrupted memory because k_num_priorities is const 
	}
    for( i=0; i < k_num_priorities; i++ ) // k_num_priorities should never be negative ??
    {
        if ( priority_values[i] == priority )
        {
            priority = i;
            break;
        }
    }
	return priority;
#else
	return(0);
#endif
}


unsigned int GNUCThread_SetPriority(GNUCThread * root, uint32_t priority, uint32_t win32PriorityClass)
{	
	int32_t old;
	unsigned int i;
	
#if ( defined(_MSC_VER) )
	BOOL bret;
#endif

	if ( root == NULL )
	{
		return -1;
	}
	
	if ( priority > 6 )
	{
		return -2; // outside of range
	}

#if ( defined(_MSC_VER) )

	bret = SetPriorityClass(root->threadHandle, win32PriorityClass);

	old = GetThreadPriority(root->threadHandle);

	SetThreadPriority(root->threadHandle, priority_values[priority]);

	if ( k_num_priorities < 0 )
	{
		return -10; // internal error ver bad, probably corrupted memory because k_num_priorities is const 
	}

    for ( i=0; i < k_num_priorities; i++ )
    {
        if ( priority_values[i] == old )
        {
            old = i;
            break;
        }
    }
	return old;
#else
	return(0);
#endif
}


#endif // GNUC_TOOLBOX_THREAD == 1 
