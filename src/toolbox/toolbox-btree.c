//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// gnuc_btree.c
//
// Copyright (c) 2008 HaiVision System Inc.	
//
// Maintained by : Francois Oligny-Lemieux
//       Created : 22.Jan.2008
//      Modified : 28.Feb.2008 (To accept duplicate names & numeric ids)
//                             (Reverse traversal: LastItem, PreviousItem)
//
//  Description: 
//      Values are classed from small(left) to right(big) in the well-known binaryTree fashion.
//
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "toolbox-config.h"

#if C_TOOLBOX_BINARY_TREE == 1
#	include "gnuc_btree.h"
#	include "toolbox-errors.h"
#	include "toolbox-basic-types.h"

#	include <string.h>
#	include <stdlib.h>



int binaryTree_Constructor(binaryTree_T * binaryTree)
{
	if ( binaryTree == NULL )
	{
		return -1;
	}

	binaryTree->top = NULL;
	binaryTree->itop = NULL;
	binaryTree->should_always_be_0xAFAF1313 = 0xAFAF1313;
	binaryTree->current_iterator = NULL;
	binaryTree->current_iiterator = NULL;
	binaryTree->accept_duplicate_names = 0;
	binaryTree->accept_duplicate_unique_ids = 0;
	binaryTree->allow_zero_unique_ids = 0;
	binaryTree->next_unique_id = 1;

	return 1;
}

int binaryTree_Insert(binaryTree_T * binaryTree, const char * name, uint64_t unique_id, void * client, bItem_T ** new_element)
{
	bItem_T * newItem;
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;
	unsigned int strLen;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( name == NULL && unique_id == 0 && binaryTree->allow_zero_unique_ids == 0 )
	{
		return -2;
	}
    
	newItem = (bItem_T*)malloc(sizeof(bItem_T));
	if ( newItem == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	memset(newItem, 0, sizeof(bItem_T));

	if ( name )
	{
		strLen = (unsigned int)strlen(name);
		if ( strLen >= GNUC_BTREE_ITEM_NAME_LENGTH )
		{
			free(newItem);
			return TOOLBOX_ERROR_YOU_PASSED_A_STRING_TOO_BIG;
		}

		strcpy(newItem->name, name);
	}

	if ( unique_id == 0 )
	{	
		if ( binaryTree->allow_zero_unique_ids == 0 )
		{
			// generate one
			unique_id = binaryTree->next_unique_id;
			binaryTree->next_unique_id++;
		}
		else
		{
			// keep the zero
		}
	}
	else if ( binaryTree->next_unique_id <= unique_id )
	{	binaryTree->next_unique_id = unique_id + 1;
	}
	newItem->unique_id = unique_id;
	newItem->client = client;

	if ( name )
	{
		iterator = binaryTree->top;
		while ( iterator )
		{
			parent = iterator;
			i = strcasecmp(iterator->name, name);
			if ( i < 0 )
			{	// candidate > iterator
				iterator = iterator->right;
			}
			else if ( i > 0 )
			{	// candidate < iterator
				iterator = iterator->left;
			}
			else
			{
				i = strcmp(iterator->name, name);
				if ( i < 0 )
				{	// candidate > iterator
					iterator = iterator->right;
				}
				else if ( i > 0 )
				{	// candidate < iterator
					iterator = iterator->left;
				}
				else
				{
					// already exists!!!
					if ( binaryTree->accept_duplicate_names )
					{
						iterator = iterator->left;
					}
					else
					{
						free(newItem);
						return TOOLBOX_ERROR_ALREADY_EXISTS;
					}
				}
			}
		}
		
		if ( parent == NULL )
		{
			binaryTree->top = newItem;
		}
		else
		{
			if ( i < 0 )
			{	// candidate > iterator
				parent->right = newItem;
			}
			else
			{	// candidate < iterator
				parent->left = newItem;
			}
			newItem->parent = parent;
		}
	} // if ( name )

	parent = NULL;
	iterator = binaryTree->itop;
	while ( iterator )
	{
		parent = iterator;
		if ( iterator->unique_id < unique_id )
		{
			iterator = iterator->iright;
		}
		else if ( iterator->unique_id > unique_id )
		{
			iterator = iterator->ileft;
		}
		else
		{
			// already exists!!!
			if ( binaryTree->accept_duplicate_unique_ids )
			{
				iterator = iterator->ileft;
			}
			else
			{
				binaryTree_Delete(binaryTree, newItem);
				newItem = NULL;
				return TOOLBOX_ERROR_ALREADY_EXISTS;
			}
		}
	}
	
	if ( parent == NULL )
	{
		binaryTree->itop = newItem;
	}
	else
	{
		if ( unique_id <= parent->unique_id )
		{
			parent->ileft = newItem;
		}
		else
		{
			parent->iright = newItem;
		}
		newItem->iparent = parent;
	}

	if ( new_element ) *new_element = newItem;

	return 1;

}

// written 29.Jan.2008
int binaryTree_Delete(binaryTree_T * binaryTree, bItem_T * remItem)
{
	int find_leftmost = -1;
	bItem_T * promote = NULL;
	bItem_T * homeless = NULL;
	bItem_T * bItem = NULL;
	bItem_T * iterator = NULL;

	/* 
	//  promote is the child of the removed item that will take its place in the tree
	//  homeless is the other child that haven't got promoted and that we will find the new place for him
	*/

	if ( binaryTree == NULL )
	{
		return -1;
	}

	if ( remItem == NULL )
	{
		return -2;
	}

	if ( remItem->parent == NULL )
	{
		if ( remItem != binaryTree->top )
		{	goto remove_from_integer_tree;
		}

		if ( remItem->left )
		{
			promote = remItem->left;
			homeless = remItem->right;
			find_leftmost = 0;
		}
		else if ( remItem->right )
		{
			promote = remItem->right;
			homeless = remItem->left;
			find_leftmost = 1;
		}
		else
		{	// will be empty
			binaryTree->top = NULL;
			goto remove_from_integer_tree;
		}
	}
	else
	{
		if ( remItem->parent->right == remItem )
		{	// parent < remItem
			if ( remItem->left )
			{	find_leftmost = 1;
				promote = remItem->left;
				homeless = remItem->right;
			}
			else if ( remItem->right )
			{
				promote = remItem->right;
			}
			else
			{	remItem->parent->right = NULL;
			}
		}
		else if ( remItem->parent->left == remItem )
		{	// parent > remItem
			if ( remItem->right )
			{
				find_leftmost = 0;
				promote = remItem->right;
				homeless = remItem->left;
			}
			else if ( remItem->left )
			{
				promote = remItem->left;
			}
			else
			{	remItem->parent->left = NULL;
			}
		}
		else
		{
#if _DEBUG
			__asm int 3;
#endif
			return TOOLBOX_ERROR_GOT_CORRUPTED;
		}
	}

	bItem = NULL;
	iterator = promote;
	if ( find_leftmost == 1 )
	{	
		while ( iterator )
		{
			bItem = iterator;
			iterator = iterator->left;
		}
	}
	else if ( find_leftmost == 0 )
	{	
		while ( iterator )
		{
			bItem = iterator;
			iterator = iterator->right;
		}
	}

	if ( homeless && bItem != remItem )
	{
		// found new home
		if ( find_leftmost == 1 )
		{
			bItem->left = homeless;
			homeless->parent = bItem;
		}
		else if ( find_leftmost == 0 )
		{
			bItem->right = homeless;
			homeless->parent = bItem;
		}
	}

	if ( promote )
	{
		if ( remItem->parent )
		{
			if ( remItem->parent->left == remItem )
			{
				remItem->parent->left = promote;
			}
			else if ( remItem->parent->right == remItem )
			{
				remItem->parent->right = promote;
			}
			promote->parent = remItem->parent;
		}
		else
		{
			binaryTree->top = promote;
			promote->parent = NULL;
		}
	}
	else
	{	// removed element has no childs
	}

remove_from_integer_tree:

	promote = NULL;
	homeless = NULL;
	find_leftmost = -1;
	if ( remItem->iparent == NULL )
	{
		if ( remItem != binaryTree->itop )
		{	return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
		}

		if ( remItem->ileft )
		{
			promote = remItem->ileft;
			homeless = remItem->iright;
			find_leftmost = 0;
		}
		else if ( remItem->iright )
		{
			promote = remItem->iright;
			homeless = remItem->ileft;
			find_leftmost = 1;
		}
		else
		{	// will be empty
			binaryTree->itop = NULL;
		}
	}
	else
	{
		if ( remItem->iparent->iright == remItem )
		{	// parent < remItem
			if ( remItem->ileft )
			{	find_leftmost = 1;
				promote = remItem->ileft;
				homeless = remItem->iright;
			}
			else if ( remItem->iright )
			{
				promote = remItem->iright;
			}
			else
			{	remItem->iparent->iright = NULL;
			}
		}
		else if ( remItem->iparent->ileft == remItem )
		{	// parent > remItem
			if ( remItem->iright )
			{
				find_leftmost = 0;
				promote = remItem->iright;
				homeless = remItem->ileft;
			}
			else if ( remItem->ileft )
			{
				promote = remItem->ileft;
			}
			else
			{	remItem->iparent->ileft = NULL;
			}
		}
		else
		{
#if _DEBUG
			__asm int 3;
#endif
			return TOOLBOX_ERROR_GOT_CORRUPTED;
		}
	}

	bItem = NULL;
	iterator = promote;
	if ( find_leftmost == 1 )
	{	
		while ( iterator )
		{
			bItem = iterator;
			iterator = iterator->ileft;
		}
	}
	else if ( find_leftmost == 0 )
	{	
		while ( iterator )
		{
			bItem = iterator;
			iterator = iterator->iright;
		}
	}

	if ( homeless && bItem != remItem )
	{
		// found new home
		if ( find_leftmost == 1 )
		{
			bItem->ileft = homeless;
			homeless->iparent = bItem;
		}
		else if ( find_leftmost == 0 )
		{
			bItem->iright = homeless;
			homeless->iparent = bItem;
		}
	}

	if ( promote )
	{
		if ( remItem->iparent )
		{
			if ( remItem->iparent->ileft == remItem )
			{
				remItem->iparent->ileft = promote;
			}
			else if ( remItem->iparent->iright == remItem )
			{
				remItem->iparent->iright = promote;
			}
			promote->iparent = remItem->iparent;
		}
		else
		{
			binaryTree->itop = promote;
			promote->iparent = NULL;
		}
	}
	free(remItem);
	return 1;
}


int binaryTree_Get(binaryTree_T * binaryTree, const char * name, uint64_t unique_id, bItem_T ** out_element)
{
	int i;
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;

	if ( binaryTree == NULL )
	{
		return -1;
	}

	if ( unique_id > 0 )
	{
		iterator = binaryTree->itop;
		while ( iterator )
		{
			parent = iterator;

			if ( iterator->unique_id < unique_id )
			{
				iterator = iterator->iright;
			}
			else if ( iterator->unique_id > unique_id )
			{
				iterator = iterator->ileft;
			}
			else
			{
				// found item !
				*out_element = iterator;
				return 1;
			}
		}
	}
	
	if ( name )
	{
		iterator = binaryTree->top;
		while ( iterator )
		{
			parent = iterator;
			i = strcasecmp(iterator->name, name);
			if ( i < 0 )
			{	// iterator < name
				iterator = iterator->right;
			}
			else if ( i > 0 )
			{	// iterator is > name
				iterator = iterator->left;
			}
			else
			{
				i = strcmp(iterator->name, name);
				if ( i < 0 )
				{	// iterator < name
					iterator = iterator->right;
				}
				else if ( i > 0 )
				{	// iterator is > name
					iterator = iterator->left;
				}
				else
				{
					// found item !
					*out_element = iterator;
					return 1;
				}
			}
		}
	}
	
	return 0;
}


int binaryTree_GetClient(binaryTree_T * binaryTree, void * client_wanted, bItem_T ** out_element)
{
	int iret;
	bItem_T * bItem;
	bItem_T * iterator = NULL;
	void * client;

	if ( out_element == NULL )
	{
		return -3;
	}

	if ( binaryTree->itop )
	{
		client = NULL;
		iret = binaryTree_FirstiItem(binaryTree, &client, &bItem, &iterator);
		while ( iret > 0 && client )
		{
			if ( client_wanted == client )
			{
				*out_element = bItem;
				return 1;
			}

			client = NULL;
			iret = binaryTree_NextiItem(binaryTree, &client, &bItem, &iterator);
		}
	}
	
	if ( binaryTree->top )
	{	
		client = NULL;
		iret = binaryTree_FirstItem(binaryTree, &client, &bItem, &iterator);
		while ( iret > 0 && client )
		{
			if ( client_wanted == client )
			{
				*out_element = bItem;
				return 1;
			}

			client = NULL;
			iret = binaryTree_NextItem(binaryTree, &client, &bItem, &iterator);
		}
	}

	return 0;
}

int binaryTree_FirstItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}

	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{	*clientIterator = NULL;	
	}
	else
	{	binaryTree->current_iterator = NULL;
	}

	if ( binaryTree->top == NULL )
	{
		return 0;
	}

	iterator = binaryTree->top;
	while ( iterator )
	{
		parent = iterator;
		iterator = iterator->left;
	}

	if ( clientIterator )
	{	*clientIterator = parent;	
	}
	else
	{	binaryTree->current_iterator = parent;
	}

	if ( out_element ) *out_element = parent;
	if ( out_opaque ) *out_opaque = parent->client;

	return 1;
}

int binaryTree_NextItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * last_valid = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}
	
	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{
		if ( *clientIterator == NULL )
		{
			*clientIterator = binaryTree->top;
			return -10; // invalid current_iterator
		}
		iterator = *clientIterator;
	}
	else
	{
		if ( binaryTree->current_iterator == NULL )
		{	
			binaryTree->current_iterator = binaryTree->top;
			return -10; // invalid current_iterator
		}
		iterator = binaryTree->current_iterator;
	}

	if ( iterator->right )
	{
		// process right branch
		iterator = iterator->right;
		while ( iterator )
		{
			last_valid = iterator;
			iterator = iterator->left;
		}
		
		if ( clientIterator )
		{	*clientIterator = last_valid;
		}
		else
		{	binaryTree->current_iterator = last_valid;
		}
	}
	else
	{
		if ( iterator->parent == NULL )
		{
			return 0; // finished parcourir le tree.
		}

		if ( iterator->parent->left == iterator )
		{
			// moving up right
			last_valid = iterator->parent;
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iterator = last_valid;
			}
		}
		else
		{
			// moving up left, parent was already outputted.
			// move up until you move up right.
			iterator = iterator->parent;
			while ( iterator )
			{
				last_valid = iterator;
				iterator = iterator->parent;
				if ( iterator && iterator->right == last_valid )
				{
					// moved left
				}
				else
				{
					break;
				}
			}
			if ( iterator == NULL )
			{
				return 0; // finished parcourir le tree.
			}
			last_valid = iterator;
			
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iterator = last_valid;
			}
		}
	}
	
	if ( out_element ) *out_element = last_valid;
	if ( out_opaque ) *out_opaque = last_valid->client;

	return 1;
}

int binaryTree_FirstiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}

	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{	*clientIterator = NULL;	
	}
	else
	{	binaryTree->current_iiterator = NULL;
	}

	if ( binaryTree->itop == NULL )
	{
		return 0;
	}

	iterator = binaryTree->itop;
	while ( iterator )
	{
		parent = iterator;
		iterator = iterator->ileft;
	}
	
	if ( clientIterator )
	{	*clientIterator = parent;	
	}
	else
	{	binaryTree->current_iiterator = parent;
	}
	
	if ( out_element ) *out_element = parent;
	if ( out_opaque ) *out_opaque = parent->client;

	return 1;
}

int binaryTree_NextiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * last_valid = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}
	
	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;
	
	if ( clientIterator )
	{
		if ( *clientIterator == NULL )
		{
			*clientIterator = binaryTree->itop;
			return -10; // invalid current_iterator
		}
		iterator = *clientIterator;
	}
	else
	{
		if ( binaryTree->current_iiterator == NULL )
		{	
			binaryTree->current_iiterator = binaryTree->itop;
			return -10; // invalid current_iterator
		}
		iterator = binaryTree->current_iiterator;
	}

	if ( iterator->iright )
	{
		// process right branch
		iterator = iterator->iright;
		while ( iterator )
		{
			last_valid = iterator;
			iterator = iterator->ileft;
		}
		
		if ( clientIterator )
		{	*clientIterator = last_valid;
		}
		else
		{	binaryTree->current_iiterator = last_valid;
		}
	}
	else
	{
		if ( iterator->iparent == NULL )
		{
			return 0; // finished parcourir le tree.
		}

		if ( iterator->iparent->ileft == iterator )
		{
			// moving up right
			last_valid = iterator->iparent;
			
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iiterator = last_valid;
			}
		}
		else
		{
			// moving up left, parent was already outputted.
			// move up until you move up right.
			iterator = iterator->iparent;
			while ( iterator )
			{
				last_valid = iterator;
				iterator = iterator->iparent;
				if ( iterator && iterator->iright == last_valid )
				{
					// moved left
				}
				else
				{
					break;
				}
			}
			if ( iterator == NULL )
			{
				return 0; // finished parcourir le tree.
			}
			last_valid = iterator;

			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iiterator = last_valid;
			}
		}
	}
	
	if ( out_element ) *out_element = last_valid;
	if ( out_opaque ) *out_opaque = last_valid->client;

	return 1;
}


int binaryTree_LastItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}

	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{	*clientIterator = NULL;	
	}
	else
	{	binaryTree->current_iterator = NULL;
	}

	if ( binaryTree->top == NULL )
	{
		return 0;
	}

	iterator = binaryTree->top;
	while ( iterator )
	{
		parent = iterator;
		iterator = iterator->right;
	}

	if ( clientIterator )
	{	*clientIterator = parent;	
	}
	else
	{	binaryTree->current_iterator = parent;
	}

	if ( out_element ) *out_element = parent;
	if ( out_opaque ) *out_opaque = parent->client;

	return 1;
}

int binaryTree_PreviousItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{	
	bItem_T * last_valid = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}
	
	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{
		if ( *clientIterator == NULL )
		{
			*clientIterator = binaryTree->top;
			return -10; // invalid current_iterator
		}
		iterator = *clientIterator;
	}
	else
	{
		if ( binaryTree->current_iterator == NULL )
		{	
			binaryTree->current_iterator = binaryTree->top;
			return -10; // invalid current_iterator
		}
		iterator = binaryTree->current_iterator;
	}

	if ( iterator->left )
	{
		// process left branch
		iterator = iterator->left;
		while ( iterator )
		{
			last_valid = iterator;
			iterator = iterator->right;
		}
		
		if ( clientIterator )
		{	*clientIterator = last_valid;
		}
		else
		{	binaryTree->current_iterator = last_valid;
		}
	}
	else
	{
		if ( iterator->parent == NULL )
		{
			return 0; // finished parcourir le tree.
		}

		if ( iterator->parent->right == iterator )
		{
			// moving up left
			last_valid = iterator->parent;
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iterator = last_valid;
			}
		}
		else
		{
			// moving up right, parent was already outputted.
			// move up until you move up left.
			iterator = iterator->parent;
			while ( iterator )
			{
				last_valid = iterator;
				iterator = iterator->parent;
				if ( iterator && iterator->left == last_valid )
				{
					// moved right
				}
				else
				{
					break;
				}
			}
			if ( iterator == NULL )
			{
				return 0; // finished parcourir le tree.
			}
			last_valid = iterator;
			
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iterator = last_valid;
			}
		}
	}
	
	if ( out_element ) *out_element = last_valid;
	if ( out_opaque ) *out_opaque = last_valid->client;

	return 1;
}

int binaryTree_LastiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{
	bItem_T * parent = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}

	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;

	if ( clientIterator )
	{	*clientIterator = NULL;	
	}
	else
	{	binaryTree->current_iiterator = NULL;
	}

	if ( binaryTree->itop == NULL )
	{
		return 0;
	}

	iterator = binaryTree->itop;
	while ( iterator )
	{
		parent = iterator;
		iterator = iterator->iright;
	}
	
	if ( clientIterator )
	{	*clientIterator = parent;	
	}
	else
	{	binaryTree->current_iiterator = parent;
	}
	
	if ( out_element ) *out_element = parent;
	if ( out_opaque ) *out_opaque = parent->client;

	return 1;
}

int binaryTree_PreviousiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator)
{	
	bItem_T * last_valid = NULL;
	bItem_T * iterator = NULL;
	int i = 0;

	if ( binaryTree == NULL )
	{
		return -1;
	}
	if ( binaryTree->should_always_be_0xAFAF1313 != 0xAFAF1313 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED_OR_GOT_CORRUPTED;
	}
	if ( out_opaque == NULL && out_element == NULL )
	{
		return -2;
	}
	
	if ( out_element ) *out_element = NULL;
	if ( out_opaque ) *out_opaque = NULL;
	
	if ( clientIterator )
	{
		if ( *clientIterator == NULL )
		{
			*clientIterator = binaryTree->itop;
			return -10; // invalid current_iterator
		}
		iterator = *clientIterator;
	}
	else
	{
		if ( binaryTree->current_iiterator == NULL )
		{	
			binaryTree->current_iiterator = binaryTree->itop;
			return -10; // invalid current_iterator
		}
		iterator = binaryTree->current_iiterator;
	}

	if ( iterator->ileft )
	{
		// process left branch
		iterator = iterator->ileft;
		while ( iterator )
		{
			last_valid = iterator;
			iterator = iterator->iright;
		}
		
		if ( clientIterator )
		{	*clientIterator = last_valid;
		}
		else
		{	binaryTree->current_iiterator = last_valid;
		}
	}
	else
	{
		if ( iterator->iparent == NULL )
		{
			return 0; // finished parcourir le tree.
		}

		if ( iterator->iparent->iright == iterator )
		{
			// moving up left
			last_valid = iterator->iparent;
			
			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iiterator = last_valid;
			}
		}
		else
		{
			// moving up right, parent was already outputted.
			// move up until you move up left.
			iterator = iterator->iparent;
			while ( iterator )
			{
				last_valid = iterator;
				iterator = iterator->iparent;
				if ( iterator && iterator->ileft == last_valid )
				{
					// moved right
				}
				else
				{
					break;
				}
			}
			if ( iterator == NULL )
			{
				return 0; // finished parcourir le tree.
			}
			last_valid = iterator;

			if ( clientIterator )
			{	*clientIterator = last_valid;
			}
			else
			{	binaryTree->current_iiterator = last_valid;
			}
		}
	}
	
	if ( out_element ) *out_element = last_valid;
	if ( out_opaque ) *out_opaque = last_valid->client;

	return 1;
}

int binaryTree_Clean(binaryTree_T * binaryTree)
{
	int iret;

	if ( binaryTree == NULL )
	{
		return -1;
	}


	while ( binaryTree->top )
	{
		iret = binaryTree_Delete(binaryTree, binaryTree->top); // not the most efficient...
		if ( iret <= 0 )
		{
			break;
		}
	}
	while ( binaryTree->itop )
	{
		iret = binaryTree_Delete(binaryTree, binaryTree->itop); // not the most efficient...	
		if ( iret <= 0 )
		{
			break;
		}
	}

	return 1;
}

int binaryTree_Destructor(binaryTree_T * binaryTree)
{
	if ( binaryTree == NULL )
	{
		return -1;
	}

	binaryTree_Clean(binaryTree);

	binaryTree->should_always_be_0xAFAF1313 = 0;

	return 1;
}



#endif //GNUC_TOOLBOX_BINARY_TREE

