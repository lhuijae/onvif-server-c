//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//						 
// gnuc_btree.h
//					
// Copyright (c) 2008 HaiVision System Inc.	
//				
// Maintained by : Francois Oligny-Lemieux
//          Date : 22.Jan.2008
//      Modified : (see .c)
//
//  Description: 
//      Single-Threaded binairy tree
//      Elements are stored as a btree_T *
//
//  Limitations:
//      WARNING: Not multi-thread safe, protect by mutex.
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#ifndef _C_TOOLBOX_BINARY_TREE_
#define _C_TOOLBOX_BINARY_TREE_

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox-errors.h" 
#include "toolbox-basic-types.h"  // for uint64_t

#define C_BTREE_ITEM_NAME_LENGTH 33

typedef struct bItem_S
{
	char name[C_BTREE_ITEM_NAME_LENGTH];
	void * client;
	struct bItem_S * parent;
	struct bItem_S * left;
	struct bItem_S * right;
	struct bItem_S * iparent;
	struct bItem_S * ileft;
	struct bItem_S * iright;
	uint64_t unique_id;

} bItem_T;

typedef struct binaryTree_S
{
	bItem_T * top; // is the carrier of the precious data
	bItem_T * itop; // for integer tree sorted by unique_id 
	int should_always_be_0xAFAF1313;
	int accept_duplicate_names;
	int accept_duplicate_unique_ids;
	int allow_zero_unique_ids; // usually when passing a 0 unique_id, a random one will be generated. Settings this on will keep 0 as the unique id
	bItem_T * current_iterator;
	bItem_T * current_iiterator;
	uint64_t next_unique_id;

} binaryTree_T;


int binaryTree_Constructor(binaryTree_T * binaryTree);
int binaryTree_Insert(binaryTree_T * binaryTree, const char * name, uint64_t unique_id, void * client, bItem_T ** new_element);
int binaryTree_Delete(binaryTree_T * binaryTree, bItem_T * in_element);

// set either unique_id or name, or both. If both are set, unique_id will be used, and name will only be used if unique_id failed.
// so it is better to use only unique_id otherwise failed Get will result in traversing twice the tree.
int binaryTree_Get(binaryTree_T * binaryTree, const char * name, uint64_t unique_id, bItem_T ** out_element);
int binaryTree_GetClient(binaryTree_T * binaryTree, void * client, bItem_T ** out_element);

// bItem_T * clientIterator is optional and only purpose is for multi-threading.
int binaryTree_FirstItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);
int binaryTree_NextItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);

int binaryTree_FirstiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);
int binaryTree_NextiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);

int binaryTree_LastItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);
int binaryTree_PreviousItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);

int binaryTree_LastiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);
int binaryTree_PreviousiItem(binaryTree_T * binaryTree, void ** out_opaque, bItem_T ** out_element, bItem_T ** clientIterator);

int binaryTree_Clean(binaryTree_T * binaryTree);

int binaryTree_Destructor(binaryTree_T * binaryTree);

#ifdef __cplusplus
}
#endif

#endif
