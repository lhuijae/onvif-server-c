//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//						 
// toolbox-linkedlist.h
//					
// Copyright (c) 2006 Francois Oligny-Lemieux
//				
// Maintained by : Francois Oligny-Lemieux
//          Date : 24.Nov.2006
//
//  Description: 
//      Single-Threaded linked list
//      Elements are stored as a void *, thus you can put in anything of size == sizeof(void*)
//         unsigned int 
//         int
//         void *
//
//  Limitations:
//      WARNING: Not multi-thread safe, protect by mutex.
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#ifndef _C_TOOLBOX_LINKED_LIST_
#define _C_TOOLBOX_LINKED_LIST_

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox-errors.h" 

typedef struct linkedList_item_S
{
	void * client; // is the carrier of the precious data, opaque (could have been a void*)
	struct linkedList_item_S * next;
	struct linkedList_item_S * prev;

} linkedList_item;

typedef struct linkedList_tag
{
	linkedList_item * firstItem;
	linkedList_item * lastItem;
	linkedList_item * current_iterator;
	unsigned int itemAmount;
	int outside_iterator_should_be_revalidated;
	int should_always_be_0xAFAF1111;

} linkedList;

int linkedList_Constructor(linkedList * list);
int linkedList_Pop(linkedList * list, const void ** const out_element);
int linkedList_Push(linkedList * list, void * in_element);
int linkedList_InsertBefore(linkedList * list, void * in_element, linkedList_item * beforeThisOne);
int linkedList_InsertAfter(linkedList * list, void * in_element, linkedList_item * afterThisOne);
int linkedList_Delete(linkedList * list, void * del_element);
int linkedList_FirstItem(linkedList * list, void ** out_element);
int linkedList_NextItem(linkedList * list, void ** out_element);
int linkedList_FirstItemEx(linkedList * list, linkedList_item ** iterator, void ** out_element);
int linkedList_NextItemEx(linkedList * list, linkedList_item ** iterator, void ** out_element);
int linkedList_FreeClients(linkedList * list); // will loop free(item->client);
int linkedList_Destructor(linkedList * list);


// returns -1 if a is smaller than b
// returns 0 if equal
// returns 1 if a is greater than b
typedef int (*toolbox_linkedlist_sort_callback)(void * client_a, void * client_b);
int linkedList_qsort(linkedList * list, toolbox_linkedlist_sort_callback sortCallback, enum TOOLBOX_SORT_METHOD sortMethod, int first, int last);

#ifdef __cplusplus
}
#endif

#endif
