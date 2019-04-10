//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox-xml.h
// 
// Copyright (c) 2007 Francois Oligny-Lemieux
//				
// Maintained by : Francois Oligny-Lemieux
//       Created : 09.Aug.2007
//      Modified : 12.Aug.2010 (wide-string)
//
//  Description: 
//      xml parser
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#ifndef __TOOLBOX_XML_H__
#define __TOOLBOX_XML_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox.h"

#if C_TOOLBOX_XML == 1

#include "toolbox-linkedlist.h"

#define TOOLBOX_XML_ELEMENT_NAME_LENGTH 64
#define TOOLBOX_XML_WORKBUFFER_SIZE 4096 // must be large enough to contain the biggest tag with its attributes

typedef struct xmlParser_S
{	char * buffer; // private
	unsigned int buffer_size; // private
	linkedList elements; // public
	char top_level_name[TOOLBOX_XML_ELEMENT_NAME_LENGTH];
	int should_always_be_0x78667866;
} xmlParser_T;

typedef struct xmlParserW_S
{	wchar_t * buffer; // private
	unsigned int buffer_size; // private
	linkedList elements; // public
	wchar_t top_level_name[TOOLBOX_XML_ELEMENT_NAME_LENGTH];
	int should_always_be_0x78667877;
} xmlParserW_T;

// an xmlElement_T has either value or childs non-NULL (never both non-NULL)
typedef struct xmlElement_S
{	char name[TOOLBOX_XML_ELEMENT_NAME_LENGTH];
	char * value;
	char * inlineText;
	unsigned int value_buffersize;
	unsigned int inlineText_buffersize;
	char * attributes;
	linkedList * childs;
} xmlElement_T;

// an xmlElement_T has either value or childs non-NULL (never both non-NULL)
typedef struct xmlElementW_S
{	wchar_t name[TOOLBOX_XML_ELEMENT_NAME_LENGTH];
	wchar_t * value;
	unsigned int value_buffersize;
	wchar_t * attributes;
	linkedList * childs;
} xmlElementW_T;

// constructed in xml_getFirstElement()
typedef struct xmlIterator_S
{	void * opaquePtr;
	unsigned int valid;
} xmlIterator_T;

int xml_Constructor_fromFile(xmlParser_T * xml, const char * filename);
int xml_Constructor_fromBuffer(xmlParser_T * xml, const char * buffer);
int xml_Destructor(xmlParser_T * xml);
int xml_DeleteBranch(xmlElement_T * branch);
int xml_getElement(linkedList * list, const char * name, xmlElement_T ** outElement);
int xml_getFirstElement(linkedList * list, const char * name, xmlElement_T ** outElement, xmlIterator_T * iterator);
int xml_getNextElement(linkedList * list, const char * name, xmlElement_T ** outElement, xmlIterator_T * iterator);
int xml_Print(linkedList * list, const char * filename);

int xml_Constructor_fromFileW(xmlParserW_T * xml, const wchar_t * filename);
int xml_Constructor_fromBufferW(xmlParserW_T * xml, const wchar_t * buffer);
int xml_DestructorW(xmlParserW_T * xml);
int xml_DeleteBranchW(xmlElementW_T * branch);
int xml_getElementW(linkedList * list, const wchar_t * name, xmlElementW_T ** outElement);
int xml_getFirstElementW(linkedList * list, const wchar_t * name, xmlElementW_T ** outElement, xmlIterator_T * iterator);
int xml_getNextElementW(linkedList * list, const wchar_t * name, xmlElementW_T ** outElement, xmlIterator_T * iterator);
int xml_PrintW(linkedList * list, const wchar_t * filename);

// [FUNCTION] xml_get...
// [EXAMPLE]
// (input)
// line == "";
// tag = "";
// (output)
// out_insider = "";


#endif //C_TOOLBOX_XML == 1

#ifdef __cplusplus
}
#endif

#endif //__TOOLBOX_XML_H__
