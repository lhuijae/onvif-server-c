//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-xml.c
// 
// Copyright (c) 2007 Francois Oligny-Lemieux
//				
// Maintained by : Francois Oligny-Lemieux
//       Created : 09.Aug.2007
//      Modified : 12.Feb.2011 (fix for an embedded element of same name as the parent)
//      Modified : 13.Feb.2011 (append a textOnly child element to support <text:p>Some nice sentance <text:span style="T1">and continues here</text:span>.</text:p>)
//
//  Description:
//      xml parser and writer
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "toolbox.h"

#if C_TOOLBOX_XML == 1

#include "toolbox.h"

// private
static int xml_Slice(char * const buffer, xmlElement_T * root);
static int xml_Print2(xmlElement_T * root, FILE * file, int level);

static const char * const g_xml_empty_string = "";

static void xmlElement_Init(xmlElement_T * elementPtr);

int xml_Constructor_fromFile(xmlParser_T * xml, const char * filename)
{
	xmlElement_T temporary;
	int iret;

	if ( xml == 0 )
	{
		return -1;
	}
	if ( filename == 0 )
	{
		return -2; // futur, allow empty initialization when we'll provide an add section function.
	}

	xml->buffer = NULL;
	xml->buffer_size = 0;

	iret = file_to_buffer_malloc(filename, &xml->buffer, &xml->buffer_size);
	if ( iret <= 0 )
	{
		return -10; // file_to_buffer_malloc failure
	}

	linkedList_Constructor(&xml->elements);	

	xmlElement_Init(&temporary);
	temporary.childs = &xml->elements;
	xml_Slice(xml->buffer, &temporary);
	
	xml_Print(&xml->elements, "g:\\mserver.xml");

	xml->should_always_be_0x78667866 = 0x78667866;

	return 1;

}

int xml_Constructor_fromBuffer(xmlParser_T * xml, const char * buffer)
{
	xmlElement_T temporary;
	unsigned int new_length;
	if ( xml == 0 )
	{
		return -1;
	}
	if ( buffer == 0 )
	{
		return -2; // futur, allow empty initialization when we'll provide an add section function.
	}

	linkedList_Constructor(&xml->elements);

	new_length = strlen(buffer)+2;

	xml->buffer = malloc(new_length);
	if ( xml->buffer == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	xml->buffer_size = new_length;
	memcpy(xml->buffer, buffer, new_length-2);
	xml->buffer[new_length-2] = '\0';
	xml->buffer[new_length-1] = '\0';

	xmlElement_Init(&temporary);
	temporary.childs = &xml->elements;
	xml_Slice(xml->buffer, &temporary);

	xml->should_always_be_0x78667866 = 0x78667866;

	return 1;
}

int xml_Destructor(xmlParser_T * xml)
{
	int iret;
	//unsigned int i;
	xmlElement_T * element = NULL;

	if ( xml == 0 )
	{
		return -1;
	}

	if ( xml->should_always_be_0x78667866 != 0x78667866 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( xml->buffer )
	{
		free(xml->buffer);
		xml->buffer = NULL;
		xml->buffer_size = 0;
	}

	iret = linkedList_FirstItem(&xml->elements, (void*)&element);
	while ( iret > 0 && element )
	{
		xml_DeleteBranch(element);

		element = NULL;
		iret = linkedList_NextItem(&xml->elements, (void*)&element);
	}
	
	linkedList_FreeClients(&xml->elements);
	linkedList_Destructor(&xml->elements);
	
	xml->should_always_be_0x78667866 = 0;

	return 1;
}

// recursive
int xml_DeleteBranch(xmlElement_T * branch)
{
	int iret;
	//unsigned int i;
	xmlElement_T * element = NULL;

	if ( branch->value && branch->value != g_xml_empty_string )
	{
		free(branch->value);
		return 1;
	}

	if ( branch->childs == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
		// probably already destroyed but not removed
	}

	iret = linkedList_FirstItem(branch->childs, (void*)&element);
	while ( iret > 0 && element )
	{
		xml_DeleteBranch(element);

		element = NULL;
		iret = linkedList_NextItem(branch->childs, (void*)&element);
	}

	linkedList_FreeClients(branch->childs);
	linkedList_Destructor(branch->childs);

	free(branch->childs); // tagAXML1

	branch->childs = NULL;

	return 1;
}

int xml_getElement(linkedList * list, const char * name, xmlElement_T ** outElement)
{
	int iret;
	xmlElement_T * element = NULL;

	if ( list == NULL )
	{	return -1;
	}
	if ( name == NULL )
	{	return -2;
	}
	if ( outElement == NULL )
	{	return -3;
	}

	*outElement = NULL;

	iret = linkedList_FirstItem(list, (void*)&element);
	while ( iret > 0 && element )
	{
		if ( strcmp(element->name, name)==0 ) // XML is case-sensitive
		{
			*outElement = element;
			return 1;
		}

		element = NULL;
		iret = linkedList_NextItem(list, (void*)&element);
	}

	return 0; // not found
}

// name is optional
int xml_getFirstElement(linkedList * list, const char * name, xmlElement_T ** outElement, xmlIterator_T * iterator)
{
	int iret;
	//unsigned int i;
	xmlElement_T * element = NULL;

	if ( list == NULL )
	{	return -1;
	}
	if ( outElement == NULL )
	{	return -3;
	}
	if ( iterator == NULL )
	{	return -4;
	}

	iterator->opaquePtr = NULL;
	iterator->valid = 1;

	*outElement = NULL;

	iret = linkedList_FirstItem(list, (void*)&element);
	while ( iret > 0 && element )
	{
		if ( name == NULL || strcmp(element->name, name)==0 ) // XML is case-sensitive
		{
			*outElement = element;
			list->outside_iterator_should_be_revalidated = 0;
			iterator->opaquePtr = (void*) list->current_iterator;
			return 1;
		}

		element = NULL;
		iret = linkedList_NextItem(list, (void*)&element);
	}

	return 0; // not found
}

// name is optional
int xml_getNextElement(linkedList * list, const char * name, xmlElement_T ** outElement, xmlIterator_T * iterator)
{
	int iret;
	//unsigned int i;
	xmlElement_T * element = NULL;

	if ( list == NULL )
	{	return -1;
	}
	if ( outElement == NULL )
	{	return -3;
	}
	if ( iterator == NULL )
	{	return -4;
	}

	*outElement = NULL;

	if ( list->outside_iterator_should_be_revalidated )
	{
        iret = linkedList_FirstItem(list, (void*)&element);
		while ( iret > 0 && element )
		{
			if ( iterator->opaquePtr == list->current_iterator
			  && (name == NULL || strcmp(name, element->name)== 0) ) // XML is case-sensitive
			{
				list->outside_iterator_should_be_revalidated = 0;
				break;
			}
	
			element = NULL;
			iret = linkedList_NextItem(list, (void*)&element);
		}
		if ( list->outside_iterator_should_be_revalidated )
		{
			// did not succeed to find the old element
			// 
			return TOOLBOX_ERROR_LINKEDLIST_ELEMENT_WAS_DELETED;
		}
	}

	element = NULL;
	list->current_iterator = (linkedList_item*) iterator->opaquePtr;
	iret = linkedList_NextItem(list, (void*)&element);
	while ( iret > 0 && element )
	{
		if ( name == NULL || strcmp(element->name, name)==0 ) // XML is case-sensitive
		{
			*outElement = element;
			list->outside_iterator_should_be_revalidated = 0;
			iterator->opaquePtr = (void*) list->current_iterator;
			return 1;
		}

		element = NULL;
		iret = linkedList_NextItem(list, (void*)&element);
	}

	return 0; // not found
}

static void xmlElement_Init(xmlElement_T * elementPtr)
{
    elementPtr->childs = NULL;
	elementPtr->value = NULL;
	elementPtr->name[0] = '\0';
	elementPtr->name[TOOLBOX_XML_ELEMENT_NAME_LENGTH-1] = '\0';
	elementPtr->attributes = NULL;
	elementPtr->inlineText = NULL;
}

static int xmlElement_AppendChild(xmlElement_T * parent, xmlElement_T * child)
{
	int iret;
	if ( parent->childs == NULL )
	{
		parent->childs = malloc(sizeof(linkedList)); // destruction tagAXML1
		if ( parent->childs == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		linkedList_Constructor(parent->childs);
	}
	iret = linkedList_Push(parent->childs, (void*)child);
	return iret;
}

// written 30.Jan.2011 - 06.Feb.2011
// problem is that we found a begining of brace and a closing brace., but the real closing brace associated 
// with the opening brace could be after that particular closing brace if there is an embedded tag with same name
// if this is the case, the real closing brace might be after the one found in closingTag
// lookupRoot starts AFTER the opening tag, that's why it's passed in elementName (without ><)
// returns the position of '<' of the closing tag
static char * findClosingBrace(const char * lookupRoot, const char * elementName, const char * closingTag)
{
	char * pointer = lookupRoot;
	char * strRet;
	char * startTagFoundA;
	char * startTagFoundB;
	char * startTagFound;
	char * lookup2;
	char * closing;
	char * firstEndingTag;
	int openingFound = 0;
	int closingFound = 0;
	int strLenClosingTag = strlen(closingTag);
	char startTagA[TOOLBOX_XML_ELEMENT_NAME_LENGTH] = "";
	char startTagB[TOOLBOX_XML_ELEMENT_NAME_LENGTH] = "";
	
	firstEndingTag = strstr(lookupRoot, closingTag);
	if ( firstEndingTag == NULL )
	{
		return NULL;
	}

	strcpy(startTagA, "<");
	strcpy(&startTagA[1], elementName);
	strcpy(startTagB, startTagA);
	strcat(startTagA, ">");
	strcat(startTagB, " ");

	strRet = strchr(lookupRoot, '<');
	if ( strRet < firstEndingTag )
	{
		// we have an tag inside, we need to figure it out.

		while (1)
		{
			startTagFound = NULL;
			startTagFoundA = strstr(pointer, startTagA); // find starting tag
			startTagFoundB = strstr(pointer, startTagB); // find starting tag
			if ( startTagFoundA && startTagFoundB ) 
			{
				if ( startTagFoundA < startTagFoundB ) 
				{
					startTagFound = startTagFoundA;
				}
				else 
				{
					startTagFound = startTagFoundB;
				}
			}
			else if ( startTagFoundA )
			{
				startTagFound = startTagFoundA;
			}
			else if ( startTagFoundB )
			{
				startTagFound = startTagFoundB;
			}
			lookup2 = strstr(pointer, closingTag); // find closing tag
			if ( (startTagFound && lookup2 == NULL)
			  || (startTagFound && lookup2 && startTagFound < lookup2) ) // opening one is found, and maybe closing one.
			{
				// we found opening before a closing
				// maybe there is no closing because it's a self-closing tag
				closing = strchr(startTagFound, '>');
				if ( closing && closing > pointer && closing[-1] == '/' )
				{
					// this is for empty elements, like <veryCoolEmptyTag />
					// do not count this one
					pointer = closing + 1;
					
					openingFound++;
					closingFound++;
				}
				else
				{
					openingFound++;
					pointer = startTagFound + 1; // +1 will put it inside the tag: it wont match again
				}
			}
			else if ( startTagFound == NULL && lookup2
			       || startTagFound && lookup2 && startTagFound > lookup2 ) // found a closing tag
			{
				closingFound++;
				
				if (closingFound == openingFound + 1) 
				{	// will do
					return lookup2;
				}

				pointer = lookup2 + strLenClosingTag; // move up after closing tag
			}
			else 
			{
				break;
			}
		}
		return NULL;
	}
	else 
	{
		// our job is done, let's output this finding
		return firstEndingTag;
	}
}

// recursive
// memory efficient :)
// modified (12.Feb.2011) to support multiple embedded tags with same name example (in OpenOffice docs): <text:p><text:p>value</text:p></text:p>
// buffer points to first character of a xml file or inside of a tag.
// buffer is not const, just the pointer
int xml_Slice(char * const buffer, xmlElement_T * root)
{
	int iret;
	char * pointer;
	char * opening = NULL;
	char * closing = NULL;
	char * whereLastLeftOf = buffer; // points after a '>'
	char * previousClose = NULL; // points after a '>'
	int strLen;
	char * strRet;
	char * tagChecker;
	char * value = NULL;
	char bender[TOOLBOX_XML_ELEMENT_NAME_LENGTH+3];
	int overrun1 = 0;
	xmlElement_T * elementPtr = NULL;
	xmlElement_T * prvElementPtr = NULL;
	char workbuffer[TOOLBOX_XML_WORKBUFFER_SIZE];
	int overrun2 = 0;
	unsigned int slicedTagWithValue = 0;
	unsigned int slicedStandalonesTags = 0;
	int count = 0;
	int matchedTags = 0;

	if ( buffer == 0 )
	{
		return -1;
	}
	if ( root == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	bender[0] = '<';
	bender[1] = '/';
	pointer = buffer;

	while (1)
	{
		count++;
		if ( elementPtr == NULL )
		{	elementPtr = malloc(sizeof(xmlElement_T));
			if ( elementPtr == NULL ) 
			{	return TOOLBOX_ERROR_MALLOC;
			}
		}
		xmlElement_Init(elementPtr);

		strRet = strchr(pointer, '<');
		if ( strRet == NULL )
		{
			strRet = strchr(pointer, '\0');
			if ( matchedTags >= 1 && strRet && whereLastLeftOf < strRet )
			{	// append a textOnly child element
				C_Append(&elementPtr->inlineText, &elementPtr->inlineText_buffersize, whereLastLeftOf, strRet-whereLastLeftOf, NULL);
				iret = xmlElement_AppendChild(root, elementPtr);
				if ( iret < 0 )
				{

					return iret;
				}				
				elementPtr = NULL;
			}
			break;
		}

		opening = strRet+1;
		pointer = opening;
		strRet = strchr(pointer, '>');
		if ( strRet )
		{
			closing = strRet;
			strLen = closing - opening;
			pointer = closing + 1;
			matchedTags++;
			if ( whereLastLeftOf < opening - 1 )
			{
				// append a textOnly child element
				C_Append(&elementPtr->inlineText, &elementPtr->inlineText_buffersize, whereLastLeftOf, opening-1-whereLastLeftOf, NULL);
				iret = xmlElement_AppendChild(root, elementPtr);
				if ( iret < 0 )
				{
					return iret;
				}
				
				elementPtr = malloc(sizeof(xmlElement_T));
				if ( elementPtr == NULL ) 
				{	return TOOLBOX_ERROR_MALLOC;
				}
				xmlElement_Init(elementPtr);
			}
			whereLastLeftOf = pointer; // not sure
			if ( strLen == 0 )
			{
				// too small, ingore
				// not a problem
			}
			else if ( strLen >= TOOLBOX_XML_WORKBUFFER_SIZE )
			{
				// too big, error, we cannot parse this.
#if _DEBUG
				__asm int 3;
#endif
				return TOOLBOX_ERROR_NOT_ENOUGH_MEMORY_IN_DESIGN;
			}				
			else
			{
				// We found a tag like <asd>

				// Prologue - Copy the chunk of text before this tag in the last entered entry
				if ( whereLastLeftOf < opening-1 )
				{
					// there was a gab.. append to value
					//C_Append(&root->value, &root->value_buffersize, whereLastLeftOf, opening-1-whereLastLeftOf, NULL);
				}

				memcpy(workbuffer, opening, strLen);
				workbuffer[strLen] = '\0';
				strRet = strchr(workbuffer, ' ');
				if ( strRet && strRet != &workbuffer[0] ) // we have a space after the name of element
				{
					// trim the ending, because it won't be there in the termination tag
					strLen = strRet - &workbuffer[0];
					elementPtr->attributes = (char*)malloc(strlen(strRet)+1);
					if ( elementPtr->attributes == NULL )
					{
						return TOOLBOX_ERROR_MALLOC;
					}
					strcpy(elementPtr->attributes, strRet+1);
					*strRet = '\0';
				}
				
				strLen = strlen(workbuffer); // warning strLen will be used few lines below
				if ( strLen >= TOOLBOX_XML_ELEMENT_NAME_LENGTH )
				{
					// too big, error, we cannot parse this.
#if _DEBUG
					__asm int 3;
#endif
					return TOOLBOX_ERROR_NOT_ENOUGH_MEMORY_IN_DESIGN;


				}
				
				strcpy(elementPtr->name, workbuffer);

				if ( (elementPtr->attributes != NULL && C_strendstr(elementPtr->attributes, "/")) 
			      || (elementPtr->attributes == NULL && C_strendstr(elementPtr->name, "/")) )
				{
					// this is for empty elements, like <veryCoolEmptyTag />
					elementPtr->value = g_xml_empty_string;
					strRet = C_strendstr(elementPtr->name, "/");
					if ( strRet )
					{	*strRet = '\0';
					}
					// apparently this section to trim space is never used:
					strRet = C_strendstr(elementPtr->name, " ");
					while ( strRet )
					{
						*strRet = '\0';
						strRet = C_strendstr(elementPtr->name, " ");
					}

					iret = xmlElement_AppendChild(root, elementPtr);
					if ( iret < 0 )
					{
						return iret;
					}

					prvElementPtr = NULL; // This element cannot have value, pvrElementPtr is used to append value.
					elementPtr = NULL;
					slicedStandalonesTags++;
					whereLastLeftOf = closing+1;
				}
				else
				{
					// look for closing tag
					strcpy(&bender[2], elementPtr->name);
					bender[strLen+2] = '>';
					bender[strLen+3] = '\0';

					// find closing brace (not straightforward!)
					closing = findClosingBrace(pointer, elementPtr->name, bender);

					//strRet = strstr(pointer, bender);
					if ( closing )
					{
						if ( *closing != '\0' )
						{
							*closing = '\0'; // terminate *TEMPORARILY* the string
							strLen = closing - pointer;
							strRet = strchr(closing+1, '>');
							if ( strRet )
							{
								whereLastLeftOf = strRet+1;
							}
						}

						iret = xml_Slice(pointer, elementPtr);
						if ( iret == 2 ) // 2 means it did not find child(s)
						{
							// copy inner section as value
//#if _DEBUG
							tagChecker = strchr(pointer, '<');
							if ( tagChecker )
							{
								// there is some other tags inside that needs to be parsed before.
								__asm int 3;
							}
//#endif
			
							value = malloc(strLen+1);
							if ( value == NULL )
							{
								return TOOLBOX_ERROR_MALLOC;
							}
							memcpy(value, pointer, strLen);
							value[strLen] = '\0';
							elementPtr->value = value;
							elementPtr->value_buffersize = strLen+1;
							value = NULL;
						}
						else if ( iret < 0 )
						{
							return iret;
						}

						xmlElement_AppendChild(root, elementPtr);
						prvElementPtr = elementPtr;
						elementPtr = NULL;

						*closing = '<'; // un-terminate the string
						slicedTagWithValue = 1;

						pointer = closing + 1; // here I could also do + strlen(bender), but to save function call I won't
					}
				}
			}
		}
	}

	if ( elementPtr )
	{
		if ( elementPtr->attributes )
		{
			free(elementPtr->attributes);
		}
		free(elementPtr);
		elementPtr = NULL; // I know it's useless but I'm a freak.
	}

	if ( overrun1 || overrun2 )
	{
		__asm int 3;
	}

	if ( slicedTagWithValue == 0 && slicedStandalonesTags == 0 )
	{
		return 2; // means did not find childs
	}

	return 1;

}

// level is for indentation
int xml_Print(linkedList * list, const char * filename)
{
	int iret;
	char * value = NULL;
	xmlElement_T * elementPtr;
	FILE * file;
	
	if ( list == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}

	if ( list->should_always_be_0xAFAF1111 != 0xAFAF1111 )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	file = fopen(filename, "wb");
	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	elementPtr = NULL;
	iret = linkedList_FirstItem(list, (void*)&elementPtr);

	while ( iret > 0 && elementPtr )
	{
		xml_Print2(elementPtr, file, 0);
		fwrite("\n", 1, 1, file);

		elementPtr = NULL;
		iret = linkedList_NextItem(list, (void*)&elementPtr);
	}

	fclose(file);
	
	return 1;

}

// recursive
// level is for indentation
int xml_Print2(xmlElement_T * root, FILE * file, int level)
{
	unsigned int i;
	int iret;
	char tagOpen[TOOLBOX_XML_ELEMENT_NAME_LENGTH+3];
	char tagEnd[TOOLBOX_XML_ELEMENT_NAME_LENGTH+3];
	xmlElement_T * elementPtr;
	static const char * const spaces = "          ";
	int printSpaces = 0; // and newlines

	tagOpen[0] = '<';
	tagOpen[TOOLBOX_XML_ELEMENT_NAME_LENGTH+2] = '\0';

	tagEnd[0] = '<';
	tagEnd[1] = '</';
	tagEnd[TOOLBOX_XML_ELEMENT_NAME_LENGTH+2] = '\0';

	if ( root->inlineText != NULL )
	{
		fwrite(root->inlineText, 1, strlen(root->inlineText), file);
		return 1;
	}

	// write indentation
	if ( printSpaces )
	{
	if ( level <= 10 )
	{
		fwrite(spaces, level, 1, file);
	}
	else 
	{
		for (i=0; i<level; i++)
		{
			fwrite(spaces, 1, 1, file);
		}
	}
	}

	// write opening tag
	tagOpen[1] = '\0';
	strcat(tagOpen, root->name);
	strcat(tagOpen, ">");
	fwrite(tagOpen, 1, strlen(tagOpen), file);
	
	if ( root->value )
	{
		fwrite(root->value, 1, strlen(root->value), file);
	}
	
	if ( root->childs )
	{
		elementPtr = NULL;
		iret = linkedList_FirstItem(root->childs, (void*)&elementPtr);
	
		while ( iret > 0 && elementPtr )
		{
			if ( printSpaces ) fwrite("\n", 1, 1, file);
			xml_Print2(elementPtr, file, level+1);

			elementPtr = NULL;
			iret = linkedList_NextItem(root->childs, (void*)&elementPtr);
		}
		
		if ( printSpaces ) fwrite("\n", 1, 1, file);
	}
	
	if ( root->childs != NULL && printSpaces )
	{
		// write indentation
		if ( level <= 10 )
		{
			fwrite(spaces, level, 1, file);
		}
		else 
		{
			for (i=0; i<level; i++)
			{
				fwrite(spaces, 1, 1, file);
			}
		}
	}

	// write ending tag
	tagEnd[2] = '\0';
	strcat(tagEnd, root->name);
	strcat(tagEnd, ">");
	fwrite(tagEnd, 1, strlen(tagEnd), file);

	return 1;
}



#endif
