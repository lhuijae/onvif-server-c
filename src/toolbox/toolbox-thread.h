#ifndef _GNUC_THREAD_H_
#define _GNUC_THREAD_H_

#if ( defined(_MSC_VER) )
//#	include <windows.h> ouch please do not include this in .h
#else
#	include <pthread.h>
#	include "toolbox-mutex.h"
#endif


typedef void (*gnuc_thread_function)(void * opaque);

// public structures
typedef struct _GNUCThread
{
	int initialized;
	
    gnuc_thread_function function;
    void * opaque;

#if ( defined(_MSC_VER) )
	HANDLE threadHandle;
	unsigned int threadId;
#else
	pthread_t threadHandle;
	unsigned int threadId;
	int suspended;
	CMutex suspendMutex;
#endif

	int beefeater;
} GNUCThread;


int GNUCThread_getId(unsigned int * loadme);

int GNUCThread_Constructor(GNUCThread * root);
int GNUCThread_Destructor(GNUCThread * root);

int GNUCThread_Create(GNUCThread * root, gnuc_thread_function function, void * opaque);
int GNUCThread_Destroy(GNUCThread * root); // never use unless you are a debugger
int GNUCThread_Suspend(GNUCThread * root); // never use unless you are a debugger
int GNUCThread_Resume(GNUCThread * root); // never use unless you are a debugger
int GNUCThread_Join(GNUCThread * root);

unsigned int GNUCThread_GetPriority(GNUCThread * root);
unsigned int GNUCThread_SetPriority(GNUCThread * root, unsigned int priority /* lowest [0,6] highest */, uint32_t win32PriorityClass /*ignored for linux*/);

#if ( defined(_MSC_VER) )
    unsigned int __stdcall GNUCThread_FunctionLauncher(void * gnuc_thread_structure);
#else
	void *GNUCThread_FunctionLauncher(void * gnuc_thread_structure);
#endif



#endif // _THREAD_H_
