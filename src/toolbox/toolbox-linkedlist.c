//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-linkedlist.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
//				
// Maintained by : Francois Oligny-Lemieux
//       Created : 09.Jan.2007
//
//  Description: 
//      
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "toolbox-config.h"

#if C_TOOLBOX_LINKED_LIST == 1
#	include "toolbox-char-array.h"
#	include "toolbox-linkedlist.h"
#	include "toolbox-errors.h"
#	include "toolbox-basic-types.h"

#	include <limits.h>
#	include <stdlib.h>
#	include <stdio.h>


int linkedList_Constructor(linkedList * list)
{
	if ( list == NULL )
	{
		return -1;
	}

	list->firstItem = NULL;	
	list->lastItem = NULL;
	list->itemAmount = 0;
	list->current_iterator = NULL;
	list->outside_iterator_should_be_revalidated = 0;
	list->should_always_be_0xAFAF1111 = 0xAFAF1111;

	return 1;
}

// takes out first item
int linkedList_Pop(linkedList * list, const void ** const out_element)
{
	linkedList_item * item;

	if ( list == NULL )
	{
		return -1;
	}

	if ( out_element == NULL )
	{
		return -2;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}	

	*out_element = list->firstItem->client;

	if ( list->firstItem->next )
	{
		item = (linkedList_item *) list->firstItem->next;
		list->firstItem->client = item->client;
		list->firstItem->next = item->next;
		list->firstItem->prev = NULL;
		free(item);
	}
	else
	{
		// only one element in fifo, free it.
		free(list->firstItem);
		list->firstItem = NULL;
		list->lastItem = NULL;
	}

	list->itemAmount--;
	list->outside_iterator_should_be_revalidated = 1;
	return 1;
}

int linkedList_Push(linkedList * list, void * in_element)
{
	linkedList_item * item;
	linkedList_item * iterate;
	unsigned int overflow_protector=0;

	if ( list == NULL )
	{
		return -1;
	}
	if ( in_element == NULL )
	{
		//27.Sep.2006, nah dont fail because sometimes people pass int values instead of pointers and zero is accepted.
		//return -2;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	item = (linkedList_item *) malloc(sizeof(linkedList_item));

	if ( item == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	item->next = NULL;
	item->prev = NULL;
	item->client = in_element;

	iterate = list->firstItem;

	if ( iterate == NULL )
	{
		// fifo is empty
		list->firstItem = item;
		list->lastItem = item;
		list->itemAmount++;
		return 1;
	}

	while ( 1 )
	{
		if ( iterate->next == NULL )
		{
			// we found the end of fifo
			break;
		}
		iterate = iterate->next;
		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			free(item);
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}

	iterate->next = item;
	item->prev = iterate;
	list->itemAmount++;
	list->lastItem = item;
	
	return 1;

}

int linkedList_InsertBefore(linkedList * list, void * in_element, linkedList_item * beforeThisOne)
{
	linkedList_item * item;
	linkedList_item * iterate;
	unsigned int overflow_protector=0;

	if ( list == NULL )
	{
		return -1;
	}
	if ( in_element == NULL )
	{
		//27.Sep.2006, nah dont fail because sometimes people pass int values instead of pointers and zero is accepted.
		//return -2;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	item = (linkedList_item *) malloc(sizeof(linkedList_item));

	if ( item == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	item->next = NULL;
	item->prev = NULL;
	item->client = in_element;

	iterate = list->firstItem;

	if ( iterate == NULL )
	{
		// fifo is empty
		list->firstItem = item;
		list->lastItem = item;
		list->itemAmount++;
		return 1;
	}
	
	if ( beforeThisOne == list->firstItem )
	{
		item->next = list->firstItem;
		list->firstItem = item;
		item->next->prev = item;
		list->itemAmount++;
		return 1;
	}
    
	while ( 1 )
	{
		if ( iterate->next == NULL || iterate->next == beforeThisOne )
		{
			// we found the desired item or end of fifo or 
			if ( iterate->next == NULL )
			{
				// we are at end
				list->lastItem = item;
			}
			else
			{
				beforeThisOne->prev = item;
				item->next = beforeThisOne;
			}
			iterate->next = item;
			item->prev = iterate;
			list->itemAmount++;
			return 1;
		}
		iterate = iterate->next;
		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			free(item);
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}
	
	return 0;
}

int linkedList_InsertAfter(linkedList * list, void * in_element, linkedList_item * afterThisOne)
{
	linkedList_item * item;
	linkedList_item * iterate;
	unsigned int overflow_protector = 0;

	if ( list == NULL )
	{
		return -1;
	}
	if ( in_element == NULL )
	{
		//27.Sep.2006, nah dont fail because sometimes people pass int values instead of pointers and zero is accepted.
		//return -2;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	item = (linkedList_item *) malloc(sizeof(linkedList_item));

	if ( item == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	item->next = NULL;
	item->prev = NULL;
	item->client = in_element;

	iterate = list->firstItem;

	if ( iterate == NULL )
	{
		// fifo is empty
		list->firstItem = item;
		list->lastItem = item;
		list->itemAmount++;
		return 1;
	}
	
	if ( afterThisOne == list->firstItem )
	{
		item->prev = list->firstItem;
      item->next = list->firstItem->next;
      if (item->next)
      {
		   item->next->prev = item;
      }
      list->firstItem->next = item;
		list->itemAmount++;
		return 1;
	}
    
	while ( 1 )
	{
		if ( iterate->next == NULL )
		{
         // we are at end
			list->lastItem = item;
         iterate->next = item;
         item->prev = iterate;
         item->next = NULL;
			list->itemAmount++;
         if (iterate == afterThisOne)
         {  return 1;
         }
         return 2; // not found was inserted at end
		}

      if (iterate == afterThisOne)
      {
         iterate->next->prev = item;
         item->next = iterate->next;
         iterate->next = item;
         item->prev = iterate;
         return 1;
      }

		iterate = iterate->next;
		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			free(item);
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}
	
	return 0;
}

// written 14.May.2007
int linkedList_Delete(linkedList * list, void * del_element)
{
	linkedList_item * previous;
	//linkedList_item * item;
	linkedList_item * backup;
	linkedList_item * iterate;
	unsigned int overflow_protector=0;
	uint8_t found = 0;

	if ( list == NULL )
	{
		return -1;
	}

	if ( del_element == NULL )
	{
		return -2;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}

	// loop to get item set correctly
	previous = NULL;
	iterate = list->firstItem;
	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}

		if ( iterate->client == del_element )
		{
			found = 1;
			if ( iterate->next /*next*/ )
			{
				backup = (linkedList_item *) iterate->next;
				if ( iterate == list->firstItem )
				{	// NO! bug list->firstItem = backup;
				}
				iterate->client = backup->client;
				iterate->next = backup->next;
				if ( backup->next )
				{
					backup->next->prev = iterate;
				}
				if ( list->current_iterator == backup )
				{
					list->current_iterator = iterate;
				}
#if _DEBUG
				if ( iterate->prev != previous )
				{
					__asm int 3;
				}
#endif
				free(backup); // Pop'ed out the next item
			}
			else
			{
				// last element of fifo
				if ( iterate == list->firstItem )
				{	list->firstItem = NULL;
				}
				if ( list->current_iterator == iterate )
				{
					list->current_iterator = NULL;
				}
				free(iterate);
				if ( previous )
				{
					list->lastItem = previous;
					previous->next = NULL;
				}
			}
			list->itemAmount--;
			break;
		}
		previous = iterate;
		iterate = iterate->next;

		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}
	
	if ( found )
	{
		list->outside_iterator_should_be_revalidated = 1;
		return 1;
	}
	return 0; // not found
}

int linkedList_FirstItem(linkedList * list, void ** out_element)
{
	if ( list == NULL )
	{
		return -1;
	}
	
	if ( out_element == NULL )
	{
		return -2;
	}
		
	*out_element = NULL;
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}

	*out_element = list->firstItem->client;
	list->current_iterator = list->firstItem;

	return 1;
}


int linkedList_NextItem(linkedList * list, void ** out_element)
{
	int success = 0;
	unsigned int overflow_protector=0;

	if ( list == NULL )
	{
		return -1;
	}

	if ( out_element == NULL )
	{
		return -2;
	}

	*out_element = NULL;

	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}

	if ( list->current_iterator == NULL )
	{	
		list->current_iterator = list->firstItem;
		return -10; // invalid current_iterator
	}

	if ( list->current_iterator->next == NULL )
	{
		return -11; // arrived to the end.
	}
	
	list->current_iterator = list->current_iterator->next;
	*out_element = list->current_iterator->client;
	
	return 1;
}


int linkedList_FirstItemEx(linkedList * list, linkedList_item ** iterator, void ** out_element)
{
	if ( list == NULL )
	{
		return -1;
	}
	
	if ( iterator == NULL )
	{
		return -2;
	}

	if ( out_element == NULL )
	{
		return -3;
	}
		
	*out_element = NULL;
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	
	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}

	*out_element = list->firstItem->client;
	*iterator = list->firstItem;

	return 1;
}


int linkedList_NextItemEx(linkedList * list, linkedList_item ** iterator, void ** out_element)
{
	int success = 0;
	unsigned int overflow_protector = 0;

	if ( list == NULL )
	{
		return -1;
	}

	if ( iterator == NULL )
	{
		return -2;
	}

	if ( out_element == NULL )
	{
		return -3;
	}

	*out_element = NULL;

	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 0;
	}

	if ( *iterator == NULL )
	{	
		*iterator = list->firstItem;
		return -10; // invalid current_iterator
	}

	if ( (*iterator)->next == NULL )
	{
		return -11; // arrived to the end.
	}
	
	*iterator = (*iterator)->next;
	*out_element = (*iterator)->client;
	
	return 1;
}


int linkedList_FreeClients(linkedList * list)
{
	linkedList_item * temp;
	linkedList_item * iterate;
	unsigned int overflow_protector=0;

	if ( list == NULL )
	{
		return -1;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 1;
	}

	iterate = list->firstItem;

	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}
		temp = iterate;
		iterate = iterate->next;
		if ( temp->client )
		{
			free(temp->client);
			temp->client = NULL;
		}

		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}

	return 1;
}

int linkedList_Destructor(linkedList * list)
{
	linkedList_item * temp;
	linkedList_item * iterate;
	unsigned int overflow_protector=0;

	if ( list == NULL )
	{
		return -1;
	}
	
	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		// fifo is empty
		return 1;
	}

	iterate = list->firstItem;

	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}
		temp = iterate;
		iterate = iterate->next;
		free(temp);

		overflow_protector++;
		if ( overflow_protector == UINT_MAX )
		{
			return TOOLBOX_OVERFLOW_OF_SOME_KIND;
		}
	}

	list->should_always_be_0xAFAF1111 = 0x00000000;
	list->itemAmount = 0;
	list->lastItem = NULL;
	list->firstItem = NULL;

	return 1;
}



// written 25.Aug.2010
// position is one-based (starts at one)
// FIXME force inline
static linkedList_item * linkedList_GetElementAt(linkedList * list, unsigned int position)
{
	linkedList_item * iterate;
	unsigned int count = 1;

	if ( position > list->itemAmount )
	{
		// invalid position asked
		return NULL;
	}

	// loop to get item set correctly
	iterate = list->firstItem;
	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}

		if ( count == position )
		{
			return iterate;
		}

		iterate = iterate->next;
		count++;
	}
	
	return NULL; // not found
}

// written 25.Aug.2010
// for debugging
static void linkedList_printClients(linkedList * list)
{
	linkedList_item * iterate;
	unsigned int count = 1;

	printf("\nlinkedList PRINT\n");

	// loop to get item set correctly
	iterate = list->firstItem;
	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}

		printf("client(%d)\n", (unsigned int)iterate->client);

		iterate = iterate->next;
		count++;
	}

	printf("linkedList PRINT ENDs\n");
}

// written 26.Aug.2010
// for debugging
static int linkedList_isGuessOk(linkedList * list, linkedList_item * alpha, linkedList_item * guess, linkedList_item * omega)
{
	linkedList_item * iterate;
	unsigned int count = 1;
	int state = 0;

	printf("\nlinkedList PRINT\n");

	// loop to get item set correctly
	iterate = list->firstItem;
	while ( 1 )
	{
		if ( iterate == NULL )
		{
			break;
		}

		if ( state == 0 )
		{
			if ( iterate == alpha )
			{
				state = 1;
			}
			else if ( iterate == guess )
			{
				printf("ERROR - guess is seen first\n");
				break;
			}
			else if ( iterate == omega )
			{
				printf("ERROR - omega is seen first\n");
				break;
			}
		}
		
		if ( state == 1 )
		{
			if ( iterate == guess )
			{
				state = 2;
			}
			else if ( iterate == omega )
			{
				printf("ERROR - omega is seen in the middle\n");
				break;
			}
		}

		if ( state == 2 )
		{
			if ( iterate == omega )
			{
				printf("Success, pivot is OK\n");
				return 1;
			}
		}

		iterate = iterate->next;
		count++;
	}

	printf("linkedList PRINT ENDs\n");
	fflush(stdout);
	return 0;
}


int linkedList_qsort(linkedList * list, toolbox_linkedlist_sort_callback sortCallback, enum TOOLBOX_SORT_METHOD sortMethod, int first, int last)
{
	// $array  - the array to be sorted
	// $column - index (column) on which to sort
	//           can be a string if using an associative array
	// $order  - SORT_ASC (default) for ascending or SORT_DESC for descending
	// $first  - start index (row) for partial array sort
	// $last  - stop  index (row) for partial array sort
	// $keys  - array of key values for hash array sort

	//int i=0,j=0;

	linkedList_item * loopItem;
	linkedList_item * row_alpha;
	linkedList_item * row_omega;
	linkedList_item * guess;
	void * guess_client;
	static int dbgcnt = 0;


	linkedList_item * row_alpha_next;
	linkedList_item * row_omega_next;
	void * client_alpha;
	void * client_omega;

	int alpha;
	int omega;
	int count;
	int iret;

	if ( list == 0 )
	{
		return -2;
	}

	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( list->firstItem == NULL )
	{
		return 0; // nothing to do
	}

	if (last == -2)
	{  
		last = list->itemAmount;
	}

	loopItem = list->firstItem;
	if ( last > first )
	{
		alpha = first;
		omega = last;
		guess = linkedList_GetElementAt(list, (omega+alpha)/2);
		guess_client = guess->client;

		if ( guess == NULL )
		{
			return -100;
		}
		while(omega >= alpha)
		{
			dbgcnt++;
			row_alpha = linkedList_GetElementAt(list, alpha); // added this 16.Feb.2006, removed from end of loop
			row_omega = linkedList_GetElementAt(list, omega); // added this 16.Feb.2006, removed from end of loop
			if (sortMethod == TOOLBOX_SORT_ASC)
			{
				while( sortCallback(row_alpha->client, guess_client) < 0 && row_alpha->next ) // the last check should never happen because we will come accross guess before going at end of list
				{
					alpha++; row_alpha = row_alpha->next;
				}
				while( sortCallback(row_omega->client, guess_client) > 0 && row_omega->prev )
				{
					omega--; row_omega = row_omega->prev;
				}
			}
			else
			{
				while( sortCallback(row_alpha->client, guess_client) > 0 && row_alpha->next )
				{
					alpha++; row_alpha = row_alpha->next; 
				}
				while( sortCallback(row_omega->client, guess) < 0 && row_omega->prev )
				{
					omega--; row_omega = row_omega->prev; 
				}
			}
			if (alpha > omega)
			{
				break;
			}
			// swap the two
			client_alpha = row_alpha->client;
			client_omega = row_omega->client;
			if ( client_omega != client_alpha )
			{
				// let's swap them..
				// just swap the client !
				row_alpha->client = client_omega;
				row_omega->client = client_alpha;
			}
			alpha++;
			omega--;
		}
		linkedList_qsort(list, sortCallback, sortMethod, first, omega);
		linkedList_qsort(list, sortCallback, sortMethod, alpha, last);
	}

	return 1;
}

#endif //C_TOOLBOX_LINKED_LIST

