//  ~~~~~~~~~~~~~~~ C Toolbox ~~~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// toolbox.c
// 
// Copyright (c) 2006 Francois Oligny-Lemieux
//				
//        Author : Francois Oligny-Lemieux
//       Created : 15.May.2006
//      Modified : 26.Jun.2016
//
//  Description: 
//      Common functions
//
//  License: 
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include "toolbox.h"
#include "toolbox-basic-types.h"

#if C_TOOLBOX_TIMING == 1 
#	if defined(_MSC_VER)
#		if ! defined(_WINDOWS_)
#			include <windows.h>
#		endif
#		include <mmsystem.h>
#	else
#		include <sys/time.h>
#		include <sys/types.h>
#		include <dirent.h>
#	endif
#else
#	if !defined(_MSC_VER)
#      include <dirent.h>
#   endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER) && !defined(__TOOLBOX_NETWORK_H__)
#	include <windows.h>
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
FILE * g_toolbox_toolbox_log_file = NULL;
#endif

/* global variable configs */
/* your app can modify those */
int g_c_toolbox_verbose = 0;


// Escapes \ with \\
// Escapes " with \"
void EscapeDoubleQuotesInString(const char* szInName, char *szOutName, size_t nSize)
{
	size_t i, j;
	size_t nLength = strlen(szInName);

	for (i = 0, j = 0; (i < nLength) && (j < (nSize - 1)); i++, j++)
	{
		if ((szInName[i] != '\"') &&
			(szInName[i] != '\\'))
		{
			szOutName[j] = szInName[i];
		}
		else
		{
			szOutName[j++] = '\\';
			szOutName[j] = szInName[i];
		}
	}

	szOutName[j] = '\0';
}



int char_remove_left_portion(char * source, char * tag)
{
	lineParser parser;
	charArray* array_insider = NULL;
	int iret;

	if ( source == 0 )
	{
		return -1;
	}
	if ( tag == 0 )
	{
		return -2;
	}

	lineParser_Constructor(&parser);

	iret = lineParser_splitTag(&parser, source, tag, &array_insider);
	if ( iret > 0 && array_insider && array_insider->array[0][0] )
	{
		strcpy(source, array_insider->array[0][0]); // frank are you sure you are right ?
	}

	lineParser_Destructor(&parser);

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

// bug to fix in there
int wchar_remove_left_portion(wchar_t * source, wchar_t * tag)
{
	lineParserW parser;
	charArrayW * array_insider = NULL;
	int iret;

	if ( source == 0 )
	{
		return -1;
	}
	if ( tag == 0 )
	{
		return -2;
	}

	lineParser_ConstructorW(&parser);

	iret = lineParser_splitTagW(&parser, source, tag, &array_insider);
	if ( iret > 0 && array_insider && array_insider->array[0][0] )
	{
		wcscpy(source, array_insider->array[0][0]); // frank are you sure you are right ?
	}

	lineParser_DestructorW(&parser);

	return 1;
}
#endif

int char_extract_path(const char * source, char * destination, int destination_size)
{
	const char * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	strRet = strrchr(source, '\\');
	if ( strRet == NULL )
	{	strRet = strrchr(source, '/');
	}
	if ( strRet )
	{
		length = strRet - source;
	}

	if ( destination_size <= length )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}

	strncpy(destination, source, length);
	destination[length] = '\0';

	return 1;
}


int wchar_extract_path(const wchar_t * source, wchar_t * destination, int destination_size)
{
	const wchar_t * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	strRet = wcsrchr(source, L'\\');
	if ( strRet == NULL )
	{	strRet = wcsrchr(source, L'/');
	}
	if ( strRet )
	{
		length = strRet - source;
	}

	if ( destination_size <= length )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}

	wcsncpy(destination, source, length);
	destination[length] = L'\0';

	return 1;
}

int char_extract_filename(const char * source, char * destination, int destination_size)
{
	const char * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	destination[0] = '\0';

	strRet = strrchr(source, '\\');
	if ( strRet == NULL )
	{	strRet = strrchr(source, '/');
	}
	if ( strRet )
	{
		length = strlen(strRet+1);
		if ( length > 0 )
		{
			if ( destination_size <= length )
			{
				return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
			}
			strncpy(destination, strRet+1, length);
			destination[length] = '\0';
			
			return 1;
		}
	}

	return 0;
}


/// destination_length in string length
int wchar_extract_filename(const wchar_t * source, wchar_t * destination, int destination_length)
{
	const wchar_t * strRet;
	unsigned int length = 0;

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

	destination[0] = L'\0';

	strRet = wcsrchr(source, L'\\');
	if ( strRet == NULL )
	{	strRet = wcsrchr(source, L'/');
	}
	if ( strRet )
	{
		length = wcslen(strRet+1);
		if ( length > 0 )
		{
			if ( destination_length <= length )
			{
				return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
			}
			wcsncpy(destination, strRet+1, length);
			destination[length] = L'\0';
			
			return 1;
		}
	}

	return 0;
}


#if defined(_TEXT_FILE_READER_H_)
int putTextFileReader_into_charArray(textFileReader * reader, charArray * array)
{
	int iret;
	char buffer[1024];
	char ** bufferPtr = NULL;
	unsigned int * buffersize = 0;
	unsigned int read_length = 0;

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	buffer[1023] = '\0';
	
	iret = charArray_add(array, "");
	if ( iret <= 0 )
	{
		return TOOLBOX_ERROR_CANNOT_ADD_CHAR_ARRAY_ROW;
	}
	bufferPtr = &array->array[array->row_amount-1][0];
	buffersize = &array->buffersize[array->row_amount-1][0];
	iret = TextFileReader_GetLine(reader, bufferPtr, buffersize, &read_length, 1/*realloc_if_necessary*/);
	while ( iret > 0 )
	{
		iret = charArray_add(array, "");
		if ( iret <= 0 )
		{	break;
		}
		read_length = 0;
		bufferPtr = &array->array[array->row_amount-1][0];
		buffersize = &array->buffersize[array->row_amount-1][0];
		iret = TextFileReader_GetLine(reader, bufferPtr, buffersize, &read_length, 1/*realloc_if_necessary*/);
		if ( iret <= 0 )
		{
			charArray_removeLineWithIndex(array, array->row_amount-1);
		}
	}

	//frank fixme remove last line
	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1
int putTextFileReader_into_charArrayW(textFileReaderW * reader, charArrayW * array)
{
	int iret;
	wchar_t buffer[1024];
	wchar_t ** bufferPtr = NULL;
	unsigned int * buffersize = 0;
	unsigned int read_length = 0;

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	buffer[1023] = L'\0';
	
	iret = charArray_addW(array, L"");
	if ( iret <= 0 )
	{
		return TOOLBOX_ERROR_CANNOT_ADD_CHAR_ARRAY_ROW;
	}
	bufferPtr = &array->array[array->row_amount-1][0];
	buffersize = &array->buffersize[array->row_amount-1][0];
	iret = TextFileReader_GetLineW(reader, bufferPtr, buffersize, &read_length, 1/*realloc_if_necessary*/);
	while ( iret > 0 )
	{
		iret = charArray_addW(array, L"");

		if ( iret <= 0 )
		{	break;
		}
		read_length = 0;
		bufferPtr = &array->array[array->row_amount-1][0];
		buffersize = &array->buffersize[array->row_amount-1][0];
		iret = TextFileReader_GetLineW(reader, bufferPtr, buffersize, &read_length, 1/*realloc_if_necessary*/);
		if ( iret <= 0 )
		{
			charArray_removeLineWithIndexW(array, array->row_amount-1);
		}
	}

	//frank fixme remove last line
	return 1;
}
#endif

// implemented 19.May.2008
int file_to_buffer(const char * filename, char * buffer, unsigned int buffer_size)
{
	int iret = 0;
	textFileReader reader;
	char * bufferPtr = buffer;
	char * bufferTemp;
	unsigned int bufferSize = buffer_size;	
	unsigned int bufferSizeTemp;
	unsigned int bufferOffset = 0;
	int read_length = 0;

	if ( filename == NULL )
	{
		return -1;
	}
	if ( buffer == NULL )
	{
		return -2;
	}
	if ( (int)buffer_size <= 0 )
	{
		return -3;
	}

	iret = TextFileReader_Constructor(&reader, filename);
	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	iret = TextFileReader_GetLine(&reader, &bufferPtr, &bufferSize, &read_length, 0/*realloc_if_necessary*/);

	while ( iret > 0 )
	{
		bufferOffset += read_length;

		if ( buffer_size - bufferOffset > 1 )
		{
			bufferPtr[bufferOffset] = '\n';
			bufferPtr[bufferOffset+1] = '\0'; // safe because we always malloc +1 of maximum GetLine result
			bufferOffset++;
		}

		bufferTemp = bufferPtr+bufferOffset;
		bufferSizeTemp = bufferSize-bufferOffset;
		iret = TextFileReader_GetLine(&reader, &bufferTemp, &bufferSizeTemp, &read_length, 0/*realloc_if_necessary*/);
	}
	
	TextFileReader_Destructor(&reader);
	
	if ( iret == TOOLBOX_WARNING_CONTINUE_READING )
	{
		return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
	}

	//fputs(bufferPtr, stdout);

	return 1;
}

// will change all \r\n to \n
int file_to_buffer_malloc(const char * filename, char ** buffer, unsigned int * buffer_size, unsigned int * out_read_size)
{
	int iret = 0;
	textFileReader reader;
	char * bufferPtr;
	char * bufferTemp;
	unsigned int bufferSize;
	unsigned int bufferSizeTemp;
	unsigned int bufferOffset = 0;
	unsigned int read_length = 0;

	if (filename == NULL)
	{
		return -1;
	}
	if (buffer == NULL)
	{
		return -2;
	}
	if (buffer_size == NULL)
	{
		return -3;
	}

	// its better to free than to realloc since realloc will memcpy the stuff (processing for nothing)
	// also in case this new file content requires smaller a buffer size than it is right now, it will free some memory.
	if (*buffer)
	{
		free(*buffer);
	}
	*buffer = NULL;
	*buffer_size = 0;

	bufferPtr = malloc(sizeof(char) * 2048);
	if (bufferPtr == NULL)
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	bufferSize = 2048;

	iret = TextFileReader_Constructor(&reader, filename);
	if (iret <= 0)
	{
		if (TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret))
		{
			return iret;
		}
		else
		{
			return -20;
		}
	}

	iret = TextFileReader_GetLine(&reader, &bufferPtr, &bufferSize, &read_length, 0/*realloc_if_necessary*/);
	bufferTemp = bufferPtr;

	while (iret > 0 || iret == TOOLBOX_WARNING_CONTINUE_READING)
	{
		bufferOffset += read_length;
		if (iret != TOOLBOX_WARNING_CONTINUE_READING)
		{
			bufferPtr[bufferOffset] = '\n';
			bufferPtr[bufferOffset + 1] = '\0'; // safe because we always malloc +1 of maximum GetLine result
			bufferOffset++;
		}

		if (bufferSize - bufferOffset < TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE + 2)
		{	// to make sure we always give text file reader enough buffer size so it doesn't stops because of us.

			bufferPtr = realloc(bufferPtr, bufferSize + TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE + 2);
			if (bufferPtr == NULL)
			{
				return TOOLBOX_ERROR_MALLOC;
			}
			bufferSize += TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE + 2;
			bufferPtr[bufferSize - 1] = '\0';
		}

		bufferTemp = bufferPtr + bufferOffset;
		bufferSizeTemp = bufferSize - bufferOffset;

		iret = TextFileReader_GetLine(&reader, &bufferTemp, &bufferSizeTemp, &read_length, 0/*realloc_if_necessary*/);
	}

	iret = TextFileReader_Destructor(&reader);

	//fputs(bufferPtr, stdout);

	*buffer = bufferPtr;
	*buffer_size = bufferSize;
	if (out_read_size) *out_read_size = bufferOffset;

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

// will change all \r\n to \n
int file_to_buffer_mallocW(const wchar_t * filename, int textEncoding, wchar_t ** buffer, unsigned int * buffer_size)
{
	int iret = 0;
	textFileReaderW reader;
	wchar_t * bufferPtr;
	wchar_t * bufferTemp;
	unsigned int bufferSize;	
	unsigned int bufferSizeTemp;
	unsigned int bufferOffset = 0;
	int read_length = 0;

	if ( filename == NULL )
	{
		return -1;
	}
	if ( buffer == NULL )
	{
		return -2;
	}
	if ( buffer_size == NULL )
	{
		return -3;
	}

	// its better to free than to realloc since realloc will memcpy the stuff (processing for nothing)
	// also in case this new file content requires smaller a buffer size than it is right now, it will free some memory.
	if ( *buffer )
	{
		free(*buffer);
	}
	*buffer = NULL;
	*buffer_size = 0;

	bufferPtr = malloc(sizeof(wchar_t)*2048);
	if ( bufferPtr == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}
	bufferSize = sizeof(wchar_t)*2048;

	iret = TextFileReader_ConstructorW(&reader, filename, textEncoding);
	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	iret = TextFileReader_GetLineW(&reader, &bufferPtr, &bufferSize, &read_length, 0/*realloc_if_necessary*/);

	while ( iret > 0 || iret==TOOLBOX_WARNING_CONTINUE_READING )
	{
		bufferOffset += read_length;
		if ( iret != TOOLBOX_WARNING_CONTINUE_READING )
		{
			bufferPtr[bufferOffset] = L'\n';
			bufferPtr[bufferOffset+1] = L'\0'; // safe because we always malloc +1 of maximum GetLine result
			bufferOffset++;
		}

		if ( bufferSize-bufferOffset < TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE+1 )
		{	// to make sure we always give text file reader enough buffer size so it doesn't stops because of us.
			bufferPtr = realloc(bufferPtr, bufferSize+TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE+1);
			if ( bufferPtr == NULL )
			{
				return TOOLBOX_ERROR_MALLOC;
			}
			bufferSize += TOOLBOX_TEXT_FILE_READER_BUFFER_SIZE+1;
		}

		bufferTemp = bufferPtr+bufferOffset;
		bufferSizeTemp = bufferSize-bufferOffset;
		iret = TextFileReader_GetLineW(&reader, &bufferTemp, &bufferSizeTemp, &read_length, 0/*realloc_if_necessary*/);
	}

	iret = TextFileReader_DestructorW(&reader);

	//fputs(bufferPtr, stdout);

	*buffer = bufferPtr;
	*buffer_size = bufferSize;

	return 1;
}

#endif

// by default chops empty lines
int file_to_array(const char * filename, charArray * array_ptr)
{	
	int iret = 0;
	textFileReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (filename==NULL)
	{
		return -2;
	}

	if ( array_ptr->array != NULL )
	{
		charArray_Destructor(array_ptr);
	}

	charArray_Constructor(array_ptr, 1024);

	iret = TextFileReader_Constructor(&reader, filename);
	if ( iret <= 0 )
	{	
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	putTextFileReader_into_charArray(&reader, array_ptr);

	iret = TextFileReader_Destructor(&reader);

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

// by default chops empty lines
int file_to_arrayW(const wchar_t * filename, int textEncoding, charArrayW * array_ptr)
{	
	int iret = 0;
	textFileReaderW reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if ( array_ptr==NULL )
	{
		return -1;
	}
	if ( filename==NULL )
	{
		return -2;
	}

	if ( array_ptr->array != NULL )
	{
		charArray_DestructorW(array_ptr);
	}

	charArray_ConstructorW(array_ptr, 1024);

	iret = TextFileReader_ConstructorW(&reader, filename, textEncoding);
	if ( iret <= 0 )
	{	
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	putTextFileReader_into_charArrayW(&reader, array_ptr);

	iret = TextFileReader_DestructorW(&reader);

	return 1;
}
#endif

// written 11.Jun.2008
int file_to_arrayEx(const char * filename, charArray * array_ptr, int chop_empty_lines)
{	
	int iret = 0;
	textFileReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (filename==NULL)
	{
		return -2;
	}

	if ( array_ptr->array != NULL )
	{
		charArray_Destructor(array_ptr);
	}

	charArray_Constructor(array_ptr, 1024);

	iret = TextFileReader_Constructor(&reader, filename);
	if ( iret <= 0 )
	{	
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	//reader.chop_empty_lines = chop_empty_lines;

	putTextFileReader_into_charArray(&reader, array_ptr);

	iret = TextFileReader_Destructor(&reader);

	return 1;
}


// function that reads <<<EOF to EOF; into variables configArray, to support simili-PHP string variables.
// hint_max_lines_in_file is an hint for initial malloc, if more is needed, will realloc.
// support adding to an already populated configArray_obj
// limitation: if variable name is already in the array, it will not overwrite it (the opposite would be more intuitive)
int file_to_variables(const char * filename, configArray * configArray_obj, int hint_max_lines_in_file)
{
	int iret;
	int a;
	int fret = 1;
	int next_variable_size;
	textFileReader reader;
	lineParser parser;
	//charArray tmpArray;
	charArray emptyArray;
	//charArray * splitted = NULL;
	char * insider = NULL;
	char temp[256]="";
	int state = 0;  // 0 = searching
	// 1 = inside
	char line[1024];
	char * bufferTemp = &line[0];
	unsigned int bufferSizeTemp = 1024;

	if (configArray_obj==NULL)
	{
		return -1;
	}
	if (filename==NULL)
	{
		return -2;
	}

	lineParser_Constructor(&parser);

	charArray_Constructor(&emptyArray, 0);
	//charArray_Constructor(&tmpArray, hint_max_lines_in_file);

	if ( configArray_obj->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	iret = TextFileReader_Constructor(&reader, filename);
	if ( iret <= 0 )
	{	
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	//putTextFileReader_into_charArray(&reader, &tmpArray);

	a=-1; // array index in charArray
	while( 1 )
	{
		line[0] = '\0';
		iret = TextFileReader_GetLine(&reader, &bufferTemp, &bufferSizeTemp, NULL, 0/*realloc_if_necessary*/);
		if ( iret <= 0 )
		{
			break;
		}

		if ( strcmp(line, "EOF;")==0 )
		{
			state = 0;
			continue;
		}

		if ( strstr(line, "<<<EOF")!=0 )
		{
			state = 1;
			// hint, look for end of tag
			next_variable_size=50;
			a=-1;
			iret = configArray_add(configArray_obj, NULL, next_variable_size, &a);
			if ( iret <= 0 || a < 0 )
			{
				// could not add new array to the charArray
				if ( iret == TOOLBOX_ERROR_MALLOC )
				{	
					fret = iret;
					break;
				}
			}
			else
			{
				iret = lineParser_extractInsider(&parser, line, "$", "=", &insider);
				iret = lineParser_stripSpacing(&parser, insider, 1, 1, &insider);
				if ( iret > 0 && insider != NULL )
				{
					iret = configArray_add_to_array(configArray_obj, a, insider);
				}
				else
				{
					snprintf(temp,255,"__cannot_extract_variable_name_iret(%d)__",iret);
					iret = configArray_add_to_array(configArray_obj, a, temp);
				}
			}
			continue;
		}

		if ( state == 0 || a < 0 )
		{
			continue;
		}

		iret = configArray_add_to_array(configArray_obj, a, line);
	}

	TextFileReader_Destructor(&reader);
	//charArray_Destructor(&tmpArray);
	charArray_Destructor(&emptyArray);
	lineParser_Destructor(&parser);

	//printf("[file_to_configArray] produced (%d) tables\n", configArray_obj->row_amount);
	return fret;
}


#if C_TOOLBOX_WIDESTRINGS == 1

// function that reads <<<EOF to EOF; into variables configArray, to support simili-PHP string variables.
// hint_max_lines_in_file is an hint for initial malloc, if more is needed, will realloc.
// support adding to an already populated configArray_obj
// limitation: if variable name is already in the array, it will not overwrite it (the opposite would be more intuitive)
// wide version 26.Feb.2011
int file_to_variablesW(const wchar_t * filename, configArrayW * configArray_obj, int hint_max_lines_in_file)
{
	int iret;
	int a;
	int fret = 1;
	int next_variable_size;
	textFileReaderW reader;
	lineParserW parser;
	//charArray tmpArray;
	charArrayW emptyArray;
	//charArray * splitted = NULL;
	wchar_t * insider = NULL;
	wchar_t temp[256] = L"";
	int state = 0;  // 0 = searching
	// 1 = inside
	wchar_t line[1024];
	wchar_t * bufferTemp = &line[0];
	unsigned int bufferSizeTemp = 1024;

	if (configArray_obj==NULL)
	{
		return -1;
	}
	if (filename==NULL)
	{
		return -2;
	}

	lineParser_ConstructorW(&parser);

	charArray_ConstructorW(&emptyArray, 0);
	//charArray_Constructor(&tmpArray, hint_max_lines_in_file);

	if ( configArray_obj->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	iret = TextFileReader_ConstructorW(&reader, filename, TOOLBOX_TEXT_ENCODING_UTF16);
	if ( iret <= 0 )
	{	
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	//putTextFileReader_into_charArray(&reader, &tmpArray);

	a = -1; // array index in charArray
	while( 1 )
	{
		line[0] = L'\0';
		iret = TextFileReader_GetLineW(&reader, &bufferTemp, &bufferSizeTemp, NULL, 0/*realloc_if_necessary*/);
		if ( iret <= 0 )
		{
			break;
		}

		if ( wcscmp(line, L"EOF;")==0 )
		{
			state = 0;
			continue;
		}

		if ( wcsstr(line, L"<<<EOF")!=0 )
		{
			state = 1;
			// hint, look for end of tag
			next_variable_size=50;
			a=-1;
			iret = configArray_addW(configArray_obj, NULL, next_variable_size, &a);
			if ( iret <= 0 || a < 0 )
			{
				// could not add new array to the charArray
				if ( iret == TOOLBOX_ERROR_MALLOC )
				{	
					fret = iret;
					break;
				}
			}
			else
			{
				iret = lineParser_extractInsiderW(&parser, line, L"$", L"=", &insider);
				iret = lineParser_stripSpacingW(&parser, insider, 1, 1, &insider);
				if ( iret > 0 && insider != NULL )
				{
					iret = configArray_add_to_arrayW(configArray_obj, a, insider);
				}
				else
				{
					snwprintf(temp, sizeof(temp)/2-1, L"__cannot_extract_variable_name_iret(%d)__", iret);
					iret = configArray_add_to_arrayW(configArray_obj, a, temp);
				}
			}
			continue;
		}

		if ( state == 0 || a < 0 )
		{
			continue;
		}

		iret = configArray_add_to_arrayW(configArray_obj, a, line);
	}

	TextFileReader_DestructorW(&reader);
	//charArray_Destructor(&tmpArray);
	charArray_DestructorW(&emptyArray);
	lineParser_DestructorW(&parser);

	//printf("[file_to_configArray] produced (%d) tables\n", configArray_obj->row_amount);
	return fret;
}
#endif

// max_line_per_section is an hint for initial malloc, if more is needed, will realloc.
int file_to_configArray(const char * filename, configArray * configArray_obj, int hint_max_lines_in_file, int max_line_per_section)
{	
	int iret;
	int i,j,a;
	textFileReader reader;
	//lineParser parser;
	charArray tmpArray;
	charArray emptyArray;
	//charArray * splitted = NULL;

	if (configArray_obj==NULL)
	{
		return -1;
	}
	if (filename==NULL)
	{
		return -2;
	}

	charArray_Constructor(&emptyArray, 0);
	charArray_Constructor(&tmpArray, hint_max_lines_in_file);

	if ( configArray_obj->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	iret = TextFileReader_Constructor(&reader, filename);
	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	putTextFileReader_into_charArray(&reader, &tmpArray);
	charArray_splitTag(&tmpArray, "::");

	iret = TextFileReader_Destructor(&reader);

	a=-1;
	for (i=0; i<tmpArray.row_amount; i++)
	{
		if ( strcmp(tmpArray.array[i][0], "====charArray.delimiter====")==0 )
		{
			if ( i+1<tmpArray.row_amount )
			{
				a=-1;
				iret = configArray_add(configArray_obj, NULL, max_line_per_section/*hint*/, &a);
			}
			continue;
		}

		if ( a < 0 )
		{
			continue;
		}

		j=0;
		if ( tmpArray.array[i][j] )
		{	
			iret = configArray_add_to_array(configArray_obj, a, tmpArray.array[i][j]);
			//printf("[file_to_configArray] add line(%s) to array number(%d), configArray_obj->array[a]->row_amount(%d), iret(%d)\n",workarray.array[i][j], a, configArray_obj->array[a]->row_amount, iret);
		}
		for (j=1; j<tmpArray.col_amount_max; j++)
		{
			if ( tmpArray.array[i][j] )
			{	
				configArray_add_to_array_col_append(configArray_obj, a, tmpArray.array[i][j]);
				//printf("[file_to_configArray] configArray_add_to_array_col_append line(%s) to array number(%d), iret(%d)\n",workarray.array[i][j], a, iret);
			}
		}
	}

	charArray_Destructor(&tmpArray);
	charArray_Destructor(&emptyArray);

	//printf("[file_to_configArray] produced (%d) tables\n", configArray_obj->row_amount);

	return 1;
}
#endif //defined(_TEXT_FILE_READER_H_)

#if defined(_TEXT_BUFFER_READER_H_)
int putTextBufferReader_into_charArray(textBufferReader * reader, charArray * array)
{
	int iret;
	char buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE+1];

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);

	while ( iret > 0 || iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL || iret == TOOLBOX_WARNING_CONTINUE_READING )
	{	
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] ERROR_BUFFER_TOO_SMALL\n");
		}
#if TOOLBOX_DEBUG_LEVEL >= 1
		else if ( g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
#endif
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
		if ( iret != TOOLBOX_WARNING_CONTINUE_READING )
		{
			// frank fixme, in case TOOLBOX_WARNING_CONTINUE_READING I trim line... so it's not 100% success
			// frank fixme, test with sample cases
			charArray_add(array, buffer);
		}
		iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);
	}

#if TOOLBOX_DEBUG_LEVEL >= 1
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[putTextBufferReader_into_charArray] End of function\n");
	}
#endif

	return 1;
}
int putTextBufferReader_into_charArrayW(textBufferReaderW * reader, charArrayW * array)
{
	int iret;
	wchar_t buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE+1];

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	iret = TextBufferReader_GetLineW(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);

	while ( iret > 0 || iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL || iret == TOOLBOX_WARNING_CONTINUE_READING )
	{	
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] ERROR_BUFFER_TOO_SMALL\n");
		}
#if TOOLBOX_DEBUG_LEVEL >= 1
		else if ( g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n", buffer);
		}
#endif
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
		if ( iret != TOOLBOX_WARNING_CONTINUE_READING )
		{
			// frank fixme, in case TOOLBOX_WARNING_CONTINUE_READING I trim line... so it's not 100% success
			// frank fixme, test with sample cases
			charArray_addW(array, buffer);
		}
		iret = TextBufferReader_GetLineW(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);
	}

#if TOOLBOX_DEBUG_LEVEL >= 1
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[putTextBufferReader_into_charArray] End of function\n");
	}
#endif

	return 1;
}

// will append to array.
int buffer_to_array(const char * buffer, charArray * array_ptr)
{	
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

// will append to array.
int buffer_to_arrayW(const wchar_t * buffer, charArrayW * array_ptr)
{	
	int iret;
	textBufferReaderW reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",wcslen(buffer));
	}
#endif

	iret = TextBufferReader_ConstructorW(&reader, buffer, (wcslen(buffer)+1)*sizeof(wchar_t));
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArrayW(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_DestructorW(&reader);

	return 1;
}
#endif

// written 11.Jun.2008
int buffer_to_arrayEx(const char * buffer, charArray * array_ptr, int chop_empty_lines)
{
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	reader.chop_empty_lines = chop_empty_lines;

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}
#endif


#if C_TOOLBOX_WIDESTRINGS == 1

// Written 10.Aug.2010
// utf16_size is in bytes.
int TOOLBOX_charToWide(const char * original, wchar_t * utf16, unsigned int utf16_size)
{
	int iret;

	if ( (int)utf16_size <= 0 )
	{
		return -3;
	}

	if ( original[0] == '\0' )
	{
		wcscpy(utf16, L"");
		return 1;
	}
#if defined(_MSC_VER)
	iret = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, original, -1, utf16, utf16_size*sizeof(wchar_t)/sizeof(char));
	if ( iret > 0 )
	{
		return 1;
	}
#endif
	return -10;
}

// written 10.Aug.2010
// utf16_size is in bytes.
int TOOLBOX_utf8ToWide(const char * utf8, wchar_t * utf16, unsigned int utf16_size)
{
	int iret;

	if ( (int)utf16_size <= 0 )
	{
		return -3;
	}

	if ( utf8[0] == '\0' )
	{
		wcscpy(utf16, L"");
		return 1;
	}
#if defined(_MSC_VER)
	iret = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16_size*sizeof(char)/sizeof(wchar_t));
	if ( iret > 0 )
	{
		if ( iret == utf16_size )
		{
			return 2;
		}
		return 1;
	}
	else
	{
		iret = GetLastError();
		if ( iret == ERROR_INSUFFICIENT_BUFFER )
		{
			utf16[(utf16_size*sizeof(char)/sizeof(wchar_t))-1] = '\0';
			return TOOLBOX_ERROR_BUFFER_TOO_SMALL;
		}
	}
#endif
	return -10;
}

// fixed 19.Sep.2011
int TOOLBOX_utf8ToWideAlloc(const char * utf8, wchar_t ** utf16, unsigned int * utf16_size)
{
	int iret;

	if ( (int)utf16_size <= 0 )
	{
		return -3;
	}

	if ( utf8[0] == '\0' && *utf16 != NULL )
	{
		wcscpy(*utf16, L"");
		return 1;
	}

	// get required size
	
#if defined(_MSC_VER)
	iret = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if ( iret > 0 )
	{
		if ( iret*2+2 > *utf16_size )
		{
			// realloc
			wchar_t * new_buffer = (wchar_t*)realloc(*utf16, iret*2+2);
			if ( new_buffer == NULL ) 
			{
				return TOOLBOX_ERROR_MALLOC;
			}
			*utf16_size = iret*2+2;
			*utf16 = new_buffer;
		}

		iret = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, *utf16, *utf16_size*sizeof(char)/sizeof(wchar_t));
		if (iret < 0)
		{
			return -12;
		}
		//*utf16[*utf16_size/2-1] = '\0';
		return 1;
	}
	else
	{
		iret = GetLastError();
		return -11;
	}
#endif
	return -10;
}

int TOOLBOX_utf16ToChar(const wchar_t * utf16, char * writeme, unsigned int writeme_size)
{
	int iret;

	if ( utf16 == NULL )
	{
		return -1;
	}
	if ( writeme == NULL )
	{
		return -2;
	}
	if ( (int)writeme_size <= 0 )
	{
		return -3;
	}


#if defined(_MSC_VER)
   iret = WideCharToMultiByte(CP_ACP, 0, utf16, -1/*means utf16 is terminated*/, writeme, writeme_size, NULL, NULL);
	if ( iret > 0 )
	{
		//writeme[iret] = '\0';
		if ( iret == writeme_size )
		{
			return 2;
		}
		return 1;
	}
	else if ( iret == 0 )
	{
#if _DEBUG && 0
		char temp[256] = "";
		iret = GetLastError();
		snprintf(temp, sizeof(temp)-1, "TOOLBOX_utf16ToChar returned 0 and last error = %d\n", iret);
		Log_C(temp);
#endif
	}
#endif
	return -20;
}

// written 11.Mar.2011
int TOOLBOX_utf16ToUtf8(const wchar_t * utf16, char * writeme, unsigned int writeme_size)
{
	int iret;

	if ( utf16 == NULL )
	{
		return -1;
	}
	if ( writeme == NULL )
	{
		return -2;
	}
	if ( (int)writeme_size <= 0 )
	{
		return -3;
	}


#if defined(_MSC_VER)
   iret = WideCharToMultiByte(CP_UTF8, 0, utf16, -1/*means utf16 is terminated*/, writeme, writeme_size, NULL, NULL);
	if ( iret > 0 )
	{
		//writeme[iret] = '\0';
		if ( iret == writeme_size )
		{
			return 2;
		}
		return 1;
	}
	else if ( iret == 0 )
	{
#if _DEBUG && 0
		char temp[256] = "";
		iret = GetLastError();
		snprintf(temp, sizeof(temp)-1, "TOOLBOX_utf16ToUtf8 returned 0 and last error = %d\n", iret);
		Log_C(temp);
#endif
	}
#endif
	return -20;
}
#endif

// temp is a temporary buffer to write to, it can point to char * temp = NULL;
int TOOLBOX_DecodeXmlTextBlobs(char * source, char ** temp, unsigned int * temp_size)
{
	char * strRet;
	char * lookup;
	int icomp;

	// zero-out
	
	if ( *temp == NULL )
	{
		C_Append(temp, temp_size, "", 1, NULL);
	}
	
	*temp[0] = '\0';

	strRet = strstr(source, "&");
	if ( strRet == NULL )
	{
		C_Append(temp, temp_size, source, strlen(source), NULL);
		return 1;
	}
	while ( strRet )
	{
		if ( strRet > source )
		{
			C_Append(temp, temp_size, source, strRet - source, NULL);
			source = strRet;
		}
		else 
		{

		}
		
		lookup = strRet;

		icomp = strncmp(lookup, "&quot;", 6);
		if ( icomp == 0 )
		{	C_Append(temp, temp_size, "\"", 1, NULL);
			source += 6;
			goto seek;
		}

		icomp = strncmp(lookup, "&amp;", 5);
		if ( icomp == 0 )
		{	C_Append(temp, temp_size, "&", 1, NULL);
			source += 5;
			goto seek;
		}

		icomp = strncmp(lookup, "&gt;", 4);
		if ( icomp == 0 )
		{	C_Append(temp, temp_size, ">", 1, NULL);
			source += 4;
			goto seek;
		}

		icomp = strncmp(lookup, "&lt;", 4);
		if ( icomp == 0 )
		{	C_Append(temp, temp_size, "<", 1, NULL);
			source += 4;
			goto seek;
		}

		icomp = strncmp(lookup, "&apos;", 6);
		if ( icomp == 0 )
		{	C_Append(temp, temp_size, "'", 1, NULL);
			source += 6;
			goto seek;
		}

		if ( lookup == source )
		{
			source++;
		}

seek:
		strRet = strstr(source, "&");
	}

	C_Append(temp, temp_size, source, -1, NULL);

	return 1;
}

int array_to_array(charArray * array_source, charArray * array_destination)
{	
	//int iret;
	//lineParser parser;
	//charArray * splitted = NULL;
	int i,j;

	if (array_source==NULL)
	{
		return -1;
	}
	if (array_source->array==NULL)
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( array_destination==NULL )
	{
		return -2;
	}
	if ( array_destination->array==NULL )
	{
		// we wont initialize it for you because we cannot destroy it for you.
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	for (i=0;i<array_source->row_amount;i++)
	{
		if ( array_source->array[i][0] )
		{
			//printf("[charArray_add] (%s)\n", array->array[i][0]);
			charArray_add(array_destination, array_source->array[i][0]);
		}

		for (j=1; j<array_source->col_amount; j++)
		{// frank fixme this could use optimizations, sometimes there is just one columns.
			if ( array_source->array[i][j] )
			{
				//printf("[charArray_col_append] (%s)\n", array->array[i][j]);
				charArray_col_append(array_destination, -1, array_source->array[i][j]);
			}
		}
	}

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

int array_to_arrayW(charArrayW * array_source, charArrayW * array_destination)
{	
	//int iret;
	//lineParser parser;
	//charArray * splitted = NULL;
	int i,j;

	if (array_source==NULL)
	{
		return -1;
	}
	if (array_source->array==NULL)
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( array_destination==NULL )
	{
		return -2;
	}
	if ( array_destination->array==NULL )
	{
		// we wont initialize it for you because we cannot destroy it for you.
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	for (i=0;i<array_source->row_amount;i++)
	{
		if ( array_source->array[i][0] )
		{
			//printf("[charArray_add] (%s)\n", array->array[i][0]);
			charArray_addW(array_destination, array_source->array[i][0]);
		}

		for (j=1; j<array_source->col_amount; j++)
		{// frank fixme this could use optimizations, sometimes there is just one columns.
			if ( array_source->array[i][j] )
			{
				//printf("[charArray_col_append] (%s)\n", array->array[i][j]);
				charArray_col_appendW(array_destination, -1, array_source->array[i][j]);
			}
		}
	}

	return 1;
}
#endif

int array_to_buffer(charArray * array_ptr, char * buffer, unsigned int buffer_size)
{	
	//int iret;
	int i;
	unsigned int buffer_position = 0;
	unsigned int str_len_base = 0;
	//textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}
	if (array_ptr->array==NULL)
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	buffer_position = (unsigned int) strlen(buffer);
	str_len_base = buffer_position;

	for ( i=0; i<array_ptr->row_amount; i++ )
	{
		buffer_position += (unsigned int) strlen(array_ptr->array[i][0]);
		if ( buffer_position < buffer_size )
		{
			strcpy(buffer+str_len_base, array_ptr->array[i][0]);
			strcpy(buffer+buffer_position, "\n");
			buffer_position += 1;
			str_len_base = buffer_position;
		}
	}

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

int array_to_bufferW(charArrayW * array_ptr, wchar_t * buffer, unsigned int buffer_size)
{	
	//int iret;
	int i;
	unsigned int buffer_position = 0;
	unsigned int str_len_base = 0;
	//textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}
	if (array_ptr->array==NULL)
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	buffer_position = (unsigned int) wcslen(buffer);
	str_len_base = buffer_position;

	for ( i=0; i<array_ptr->row_amount; i++ )
	{
		buffer_position += (unsigned int) wcslen(array_ptr->array[i][0]);
		if ( buffer_position < buffer_size )
		{
			wcscpy(buffer+str_len_base, array_ptr->array[i][0]);
			wcscpy(buffer+buffer_position, L"\n");
			buffer_position += 1;
			str_len_base = buffer_position;
		}
	}

	return 1;
}
#endif

// utf8 string MUST be terminated utf8_buffer_size must be valid
// utf8_buffer_size can be -1 if string is terminated
int TOOLBOX_utf8strlen(const char * utf8, int utf8_buffer_size)
{
	uint32_t * lookuper;
	uint32_t slab;

	uint32_t result;
	const char * smallGrained = utf8;
	unsigned int strlen = 0;
	char * end_of_string;
	int count = 0;
	int amtNotStarter = 0;
	if ( utf8 == NULL )
	{
		return 0;
	}

	end_of_string = utf8 + utf8_buffer_size;

	while ( utf8_buffer_size == -1 || smallGrained < end_of_string )
	{
		if ( *smallGrained == '\0' )
		{
			return (smallGrained - utf8) - amtNotStarter; // end of string
		}

		if ( (uint64_t)smallGrained % 4 == 0 ) 
		{
			//yes it's 32-bit aligned
			lookuper = (uint32_t*)smallGrained;
			break;
		}

		if ( (*smallGrained & 0xC0) == 0x80 )
		{
			amtNotStarter++;
		}

		smallGrained++;
	}

	if ( smallGrained+3 >= end_of_string )
	{
		goto finition;
	}

	lookuper = smallGrained;

	while (1)
	{
		slab = *lookuper;

		/* Exit the loop if there are any zero bytes. */
		if ((slab - 0x01010101) & (~slab) & (0x01010101 * 0x80))
			break;

		slab = *lookuper & 0x80808080;
		result = *lookuper & 0xC0C0C0C0;
		result = ((slab) >> 7) & (~(*lookuper & 0x40404040) >> 6); // matches 0b10XX XXXX
		amtNotStarter += ((result >> 24) + (result >> 16) + (result >> 8) + result)&0x000000F;

		lookuper++;
	}

	smallGrained = (char*)lookuper;

finition:
	while ( utf8_buffer_size == -1 || smallGrained < end_of_string )
	{
		if ( *smallGrained == '\0' )
		{
			return (smallGrained - utf8) - amtNotStarter; // end of string
		}

		if ( (*smallGrained & 0xC0) == 0x80 )
		{
			amtNotStarter++;
		}

		smallGrained++;
	}

	return (smallGrained - utf8) - amtNotStarter;
}

// utf8 string MUST be terminated.
const char * C_utf8EndOfString(const char * utf8, int utf8_buffer_size)
{
	const char * zero = "\0";
	if ( utf8 == NULL )
	{
		return NULL;
	}

	if ( utf8_buffer_size == -1 )
	{
		utf8_buffer_size = 0x7FFFFFFF;
	}

	return (const char *)C_memfind((unsigned char *)utf8, utf8_buffer_size, (unsigned char *)zero, 1); 
}

// written 25.Jan.2008
// will realloc the string if too small, or malloc it if NULL
// separator is optional and can be NULL
// other parameters are mandatory
// max_length is either -1 or to the maximum length that can be copied from append.
int C_Append(char ** string, unsigned int * buffersize, const char * append, int max_length, const char * separator)
{
	//unsigned int strLen;
	unsigned int newSize = 0;
	int mallocop = 0;
	char * new_string = NULL;

	if ( max_length == 0 )
	{
		return 0;
	}

	if ( *string )
	{	newSize = strlen(*string);
	}
	if ( max_length < 0 )
	{	newSize += strlen(append); // append without restriction
	}
	else
	{	newSize += max_length; // append up to max_length
	}
	if ( separator )
	{	newSize += strlen(separator);
	}

	newSize += 2;
	if ( *string == NULL )
	{
		new_string = (char*)malloc(newSize);
		mallocop = 1;
		*buffersize = 0;
	}
	else if ( newSize > *buffersize )
	{
		new_string = (char*)realloc(*string, newSize);
		mallocop = 1;
	}
	
	if ( mallocop )
	{
		if ( new_string == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		*string = new_string;
		memset(new_string+*buffersize, 0, newSize - *buffersize); // frank fixme optimize by erasing only end char and first chars
		*buffersize = newSize;
	}

	if ( separator )
	{	strcat(*string, separator);
	}
	if ( max_length < 0 )
	{
		strcat(*string, append);
	}
	else
	{
		strncat(*string, append, max_length);
	}
	 
	return 1;
}


#if C_TOOLBOX_WIDESTRINGS == 1

// will realloc the string if too small, or malloc it if NULL
// separator is optional and can be NULL
// other parameters are mandatory
// buffersize is in bytes
// max_length is either -1 or to the maximum length that can be copied from append.
// max_length is in unit wchar_t strlen.
int C_AppendW(wchar_t ** string, unsigned int * buffersize, const wchar_t * append, int max_length, const wchar_t * separator)
{
	unsigned int newSize = 0;
	int mallocop = 0;
	wchar_t * new_string = NULL;

	if ( max_length == 0 )
	{
		return 0;
	}

	if ( *string )
	{	newSize = wcslen(*string);
	}
	if ( max_length < 0 )
	{	newSize += wcslen(append); // append without restriction
	}
	else
	{	newSize += max_length; // append up to max_length
	}
	if ( separator )
	{	newSize += wcslen(separator);
	}

	newSize = newSize*sizeof(wchar_t) + 2;
	if ( *string == NULL )
	{
		new_string = (wchar_t*)malloc(newSize);
		mallocop = 1;
		*buffersize = 0;
	}
	else if ( newSize > *buffersize )
	{
		new_string = (wchar_t*)realloc(*string, newSize);
		mallocop = 1;
	}
	
	if ( mallocop )
	{
		if ( new_string == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		*string = new_string;
		memset(((char*)new_string)+*buffersize, 0, newSize - *buffersize); // frank fixme optimize by erasing only end char and first chars
		*buffersize = newSize;
		//*((char*)new_string+newSize-1) = 0xDD;
		//*((char*)new_string+newSize-2) = 0xEE;
		//*((char*)new_string+newSize-3) = 0xDD;
	}

	if ( separator )
	{	wcscat(*string, separator);
	}
	if ( max_length < 0 )
	{
        wcscat(*string, append);
	}
	else
	{
        wcsncat(*string, append, max_length);
	}
	 
	return 1;
}

// written 21.Aug.2010
// max_length is the amout of bytes to copy from append
int C_AppendUtf8toWide(wchar_t ** string, unsigned int * buffersize, const char * append, int max_length, const wchar_t * separator, unsigned int * appendedLength)
{
	unsigned int strLen;
	unsigned int strLenSeparator;
	unsigned int newSize = 0; // in string length.
	int mallocop = 0;
	wchar_t * new_string = NULL;
	int iret;

	if ( max_length == 0 )
	{
		return 0;
	}

	if ( *string )
	{	newSize = wcslen(*string);
	}

	if ( max_length < 0 )
	{	
		newSize += TOOLBOX_utf8strlen(append, -1); // append without restriction, *2 is for UTF16 
		max_length = -1;
	}
	else
	{
		newSize += max_length; // append up to max_length
	}

	if ( separator )
	{
		strLenSeparator = wcslen(separator);
		newSize += strLenSeparator;
	}

	if ( appendedLength )
	{	*appendedLength = 0;
	}

	newSize += 2;
	if ( *string == NULL )
	{
		new_string = (wchar_t*)malloc(newSize*sizeof(wchar_t));
		mallocop = 1;
		*buffersize = 0;
	}
	else if ( newSize*sizeof(wchar_t) > *buffersize )
	{
		new_string = (wchar_t*)realloc(*string, newSize*sizeof(wchar_t));
		mallocop = 1;
	}
	
	if ( mallocop )
	{
		if ( new_string == NULL )
		{
			return TOOLBOX_ERROR_MALLOC;
		}
		*string = new_string;
		memset(((char*)new_string)+*buffersize, 0, newSize*sizeof(wchar_t) - *buffersize); // frank fixme optimize by erasing only end char and first chars
		*buffersize = newSize*sizeof(wchar_t);
	}

	if ( separator )
	{	wcscat(*string, separator);
	
		if ( appendedLength )
		{	*appendedLength = strLenSeparator;
		}
	}

	strLen = wcslen(*string);

#if defined(_MSC_VER)
   iret = MultiByteToWideChar(CP_UTF8, 0, append, max_length, *string+strLen, newSize);
	if ( iret <= 0 )
	{
		// problem
		DWORD error = GetLastError();
		__asm int 3;
		return -50;
	}
	else
	{
		(*string)[strLen+iret] = L'\0';

		if ( appendedLength )
		{	*appendedLength += iret;
		}
	}
#endif	 
	return 1;
}
#endif

// written 01.May.2008
int buffer_to_file(const char * buffer, const char * filename)
{
	FILE * file;
	int iret;

	if ( buffer == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}

	file = fopen(filename, "wb");

	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	iret = fwrite((char*)buffer, 1, strlen(buffer), file);
	
	fclose(file);

	if ( iret <= 0 )
	{		
		return -10;
	}

	return 1;
}



#if C_TOOLBOX_WIDESTRINGS == 1

// FIXME, needs corrections
int buffer_to_fileW(const wchar_t * buffer, const wchar_t * filename)

{
	FILE * file;
	int iret;

	if ( buffer == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}

	file = wfopen(filename, L"wb");

	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	iret = fwrite(buffer, 1, wcslen(buffer)*sizeof(wchar_t), file);
	
	fclose(file);

	if ( iret <= 0 )
	{		
		return -10;
	}

	return 1;
}
#endif

int C_Logger(char * text)
{
	int fret = 1;
	static int firsttime = 1;

#ifdef C_LOGGER
	if ( firsttime == 1 )
	{
		g_gnuc_log = fopen(g_gnuc_log_filename, "aw");
		if ( g_gnuc_log <= 0 )
		{
			printf("<!-- error opening logger -->\n");
			g_gnuc_log=NULL;
		}
		else
		{
			//printf("<!--opening logger-->\n");
		}
		firsttime = 0;
	}
#endif

#ifdef C_LOGGER
	{
		if ( g_gnuc_log <= 0 )
		{
			fret=-1; // g_log invalid
		}
		else if ( text <= 0 )
		{
			fret=-2; // text invalid
		}
		else
		{
			if ( g_gnuc_log > 0 )
			{
				//int wrote = 
				//printf("[Logger] going to fwrite strlen(text)=%u\n", strlen(text));
				fwrite(text, 1, strlen(text), g_gnuc_log);
				//printf("[Logger] after fwrite\n");
				fflush(g_gnuc_log);
				//printf("[Logger] after fflush\n");
			}
		}
	}
#endif

	return fret;
}

 
int C_FileExist(const char * filename)
{
	FILE * test;
	int ret = 0;

	if ( filename == NULL || filename[0] == '\0' )
	{
		return -1;
	}

	test = fopen(filename, "r");
	if (test == NULL)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		fclose(test);
	}
	return ret;
}


#if C_TOOLBOX_WIDESTRINGS == 1

int TOOLBOX_FileExistW(const wchar_t * filename)
{
	FILE * test;
	int ret = 0;

	if ( filename == NULL || filename[0] == L'\0' )
	{
		return -1;
	}

	test = wfopen(filename, L"r");
	if (test == NULL)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		fclose(test);
	}
	return ret;
}
#endif

const char * C_GetFileExtension(const char * filename)
{
	int count = 5;
   const char * strLook;
	
	int strLength = strlen(filename);
	if (strLength == 0) return NULL;
	
	strLook = &filename[strLength-1];
	while (count > 0 && strLook > &filename[0])
	{
		if (*strLook == '.') return strLook+1;
		strLook--;
		count--;
	}
	
	return NULL;
}

int C_DirectoryExist(const char * directory)
{
	//FILE * test;
	//int ret = 0;
#if defined(_MSC_VER)
	DWORD attributes = GetFileAttributes(directory);
	if ( attributes != INVALID_FILE_ATTRIBUTES
	  && attributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		return 1;
	}
#	if _DEBUG
	attributes = GetLastError();
	attributes = attributes;
#	endif
	return 0;
#else
	DIR * dirPtr = opendir(directory);
	if (dirPtr == NULL)
	{
		return 0;
	}
	closedir(dirPtr);
	return 1;
#endif
}

#if C_TOOLBOX_WIDESTRINGS == 1

int TOOLBOX_DirectoryExistW(const wchar_t * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	DWORD attributes = GetFileAttributesW(directory);
	if ( attributes != INVALID_FILE_ATTRIBUTES
	  && attributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		return 1;
	}
#	if _DEBUG
	attributes = GetLastError();
	attributes = attributes;
#	endif
	return 0;
#else
	DIR * dirPtr;
	dirPtr = wopendir(directory);
	if ( dirPtr == NULL )
	{
		return 0;
	}
	wclosedir(dirPtr);
	return 1;
#endif
}

#endif

int C_CreateDirectory(const char * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	BOOL bret = CreateDirectory(directory, NULL);
	if ( bret == FALSE )
	{	
		// failure
		return -10;
	}
	return 1;
#else
	int iret;
	iret = mkdir(directory, 0777);
	if ( iret < 0 )
	{
		// failure
		return -10;
	}
	return 1;
#endif
}

#if C_TOOLBOX_WIDESTRINGS == 1

int TOOLBOX_CreateDirectoryW(const wchar_t * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	BOOL bret = CreateDirectoryW(directory, NULL);
	if ( bret == FALSE )
	{	
		// failure
		return -10;
	}
	return 1;
#else
	int iret;
	iret = mkdir(directory, 0777);
	if ( iret < 0 )
	{
		// failure
		return -10;
	}
	return 1;
#endif
}
#endif

int C_FileTouch(const char * filename)
{
	FILE * test;
	int ret = 0;
	char line[128]="Hello now";
	test = fopen(filename, "a");
	if (test == NULL)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		fwrite(line,strlen(line),1,test);
		fclose(test);
	}
	return ret;
}


int C_isFileOpened(const char * filename)
{
#if ( defined(_MSC_VER) )
	HANDLE fhandle;
	fhandle = CreateFile(filename, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if ( fhandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle(fhandle);
		return 0;
	}
	else
	{
		return 1;
	}
#else

#endif

	return 0;
}


#if C_TOOLBOX_WIDESTRINGS == 1

TOOLBOX_isFileOpenedW(const wchar_t * filename)
{
#if ( defined(_MSC_VER) )
	HANDLE fhandle;
	fhandle = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if ( fhandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle(fhandle);
		return 0;
	}
	else
	{
		return 1;
	}
#else

#endif

	return 0;
}
#endif


int C_DeleteFile(const char * filename)
{
#if ( defined(_MSC_VER) )
#else
	char command[256];
#endif

	if ( filename == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	DeleteFile(filename);
#else
	snprintf(command,255,"rm %s", filename);
	system(command);
#endif

	return 1;
}


#if C_TOOLBOX_WIDESTRINGS == 1

int C_DeleteFileW(const wchar_t * filename)
{
#if ( defined(_MSC_VER) )
#else
	wchar_t command[256];
#endif

	if ( filename == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	DeleteFileW(filename);
#else
	snwprintf(command,255,L"rm %s", filename);
	systemW(command);
#endif

	return 1;
}
#endif

// by default will overwrite file
int TOOLBOX_CopyFile(const char * source, const char * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = CopyFile(source,destination,FALSE/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#else
	snprintf(command,255,"cp -f \"%s\" \"%s\"", source,destination);
	system(command);
#endif

	return 1;
}

// written 08.Jan.2008
int C_CopyFileEx(const char * source, const char * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = 0x00000008/*COPY_FILE_ALLOW_DECRYPTED_DESTINATION*/;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )

#	if(_WIN32_WINNT >= 0x0400)
	if ( (flags&C_OVERWRITE_DESTINATION)==0 )
	{	win32_flags |= 0x00000001/*COPY_FILE_FAIL_IF_EXISTS*/;
	}
	bret = CopyFileEx(source, destination, NULL/*progress*/, NULL/*progress data*/, FALSE, win32_flags);
	if ( bret == FALSE )
	{
		// error
#	if _DEBUG
		printf("WIN32 CopyFileEx failed with error(%d)\r\n", (int)GetLastError());
#	endif
		return -10;
	}

#	else

	bret = FALSE;
	if ( (flags&C_OVERWRITE_DESTINATION)==0 )
	{	bret = TRUE;
	}
	bret = CopyFile(source,destination,bret/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#	endif

#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snprintf(command,255,"cp -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"cp \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}


#if C_TOOLBOX_WIDESTRINGS == 1

// written 08.Jan.2008
int TOOLBOX_CopyFileExW(const wchar_t * source, const wchar_t * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = 0x00000008/*COPY_FILE_ALLOW_DECRYPTED_DESTINATION*/;
#else
	wchar_t command[1024];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )

#	if(_WIN32_WINNT >= 0x0400)
	if ( (flags&TOOLBOX_OVERWRITE_DESTINATION)==0 )
	{	win32_flags |= 0x00000001/*COPY_FILE_FAIL_IF_EXISTS*/;
	}
	bret = CopyFileExW(source, destination, NULL/*progress*/, NULL/*progress data*/, FALSE, win32_flags);
	if ( bret == FALSE )
	{
		// error
#	if _DEBUG
		wprintf(L"WIN32 CopyFileEx failed with error(%d)\r\n", (int)GetLastError());
#	endif
		return -10;
	}

#	else

	bret = FALSE;
	if ( (flags&TOOLBOX_OVERWRITE_DESTINATION)==0 )
	{	bret = TRUE;
	}
	bret = CopyFileW(source,destination,bret/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#	endif

#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snwprintf(command,sizeof(command)-1,"cp -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"cp \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}
#endif

// by default will overwrite file
int C_MoveFile(const char * source, const char * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = MoveFileEx(source,destination, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH|MOVEFILE_COPY_ALLOWED);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFileEx failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	snprintf(command,255,"mv -f \"%s\" \"%s\"", source,destination);
	system(command);
#endif

	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

// by default will overwrite file
int TOOLBOX_MoveFileW(const wchar_t * source, const wchar_t * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = MoveFileExW(source,destination, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH|MOVEFILE_COPY_ALLOWED);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFileEx failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	snwprintf(command,255,L"mv -f \"%s\" \"%s\"", source, destination);
	systemw(command);
#endif

	return 1;
}
#endif

// flags are agregates of gnucFlags_E
// written 08.Jan.2008
int TOOLBOX_MoveFileEx(const char * source, const char * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = MOVEFILE_WRITE_THROUGH;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

#if ( defined(_MSC_VER) )
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{	win32_flags |= MOVEFILE_REPLACE_EXISTING;
	}
	win32_flags |= MOVEFILE_COPY_ALLOWED;
	bret = MoveFileEx(source, destination, win32_flags);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFile failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snprintf(command,255,"mv -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"mv \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}


#if C_TOOLBOX_WIDESTRINGS == 1

// flags are agregates of gnucFlags_E
// written 08.Jan.2008
int TOOLBOX_MoveFileExW(const wchar_t * source, const wchar_t * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = MOVEFILE_WRITE_THROUGH;
#else
	wchar_t command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

#if ( defined(_MSC_VER) )
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{	win32_flags |= MOVEFILE_REPLACE_EXISTING;
	}
	win32_flags |= MOVEFILE_COPY_ALLOWED;
	bret = MoveFileExW(source, destination, win32_flags);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFile failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snwprintf(command,255,L"mv -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snwprintf(command,255,L"mv \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}
#endif

// written 19.Feb.2011
int TOOLBOX_ClearDirectory(const char * directory, const char * filter_const)
{
	char filename[5000];
	char filter[128];
	int iret;
	unsigned int i;
	char * index; // index at which the filename begins
	charArray array;
	char * strRet;

	// filter search
	int ends_with = 0;
	int pass;

	filename[sizeof(filename)-1] = '\0';

	strcpy(filename, directory);
	TOOLBOX_endSlashDirectory(filename);
	index = filename + strlen(filename);

	if ( filter_const )
	{
		strRet = C_strendstr(filter_const, "$");
		if ( strRet && strRet > filter_const + 1 )
		{
			// ends with
			strncpy(filter, filter_const, strRet - filter_const);
			filter[strRet - filter_const] = '\0';
			ends_with = 1;
		}
	}

	charArray_Constructor(&array, 50000);
	iret = ls_directory_to_array(directory, &array);
	if ( iret > 0 && array.row_amount > 0 )
	{
		for (i=0; i<array.row_amount; i++)
		{
			strcpy(index, array.array[i][0]);
			if ( filter[0] != '\0' )
			{
				pass = 0;
				if ( ends_with )
				{
					strRet = C_strendstr(array.array[i][0], filter);
					if ( strRet )
					{
						// ok
						pass = 1;
					}
				}
				if ( pass == 0 )
				{
					continue;
				}
			}
			C_DeleteFile(filename);
		}
	}

	charArray_Destructor(&array);

	return 1;
}

// written 19.Feb.2011
int TOOLBOX_endSlashDirectory(char * directory_inout)
{
	int backslashes = 0;
	char * strRet;
	char * strRet2;
		
	strRet = strstr(directory_inout, "\\");
	if ( strRet ) 
	{
		backslashes = 1;
	}

	strRet = C_strendstr(directory_inout, "\\");
	strRet2 = C_strendstr(directory_inout, "/");
	if ( strRet == NULL && strRet2 == NULL )
	{
		if ( backslashes ) 
		{	strcat(directory_inout, "\\");
		}
		else
		{	strcat(directory_inout, "/");
		}
	}

	return 1;
}

#if ( defined(_MSC_VER) )

int PingHost(const char * hostname)
{
	return 0;
}

#else 

int PingHost(const char * hostname)
{
	int iret;
	struct sockaddr_in server;
	int sockfd;
	struct timeval timeout;

#if 0
	host = gethostbyname(hostname);
	if( host == NULL)
	{
		printf("Couldn't resolve host!\n");
		return -10;
	}
#endif


	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	//int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // ping
	if (sockfd < 0) 
	{
		//error("ERROR opening socket");
		printf("cannot create socket\n");
		return -20;
	}

	timeout.tv_sec  = 0;
	timeout.tv_usec = 1000 * 1500; // 1000 * ms
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(hostname);
	server.sin_port = htons(80);

#if 0
	ip->ihl = 5;
	ip->id = htons(1337);
	ip->ttl = 255;
	ip->tos = 0;
	ip->protocol = IPPROTO_ICMP;
	ip->version = 4;
	ip->frag_off = 0;
	ip->saddr = htons("1.3.3.7");
	ip->daddr = inet_ntoa(server.sin_addr);
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
	ip->check = 0;
	icmp->checksum = 0;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
#endif

	//printf("going to connect\n");
	iret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));

	//printf("connect returned iret(%d)\n", iret);
	if(iret == -1)
	{
		close(sockfd);
		//printf("connect returned iret(%d)\n", iret);
		return -30;
	}
	else if( iret < 0 )
	{
		close(sockfd);
		//printf("connect returned iret(%d)\n", iret);
		return -31;
	}

	close(sockfd);

	return 1;
}

#endif // linux



#if ( !defined(_MSC_VER) )
#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>

// linux
int ls_directory_to_array(const char * directory, charArray * array_obj)
{
	//int iret;
	DIR *dp;
	struct dirent *ep;

	if ( directory == NULL )
	{
		return -1;
	}
	if ( array_obj == NULL )
	{
		return -2;
	}

	dp = opendir (directory);

	if ( dp == NULL )
	{
		// cannot open directory
		return -10;
	}

	ep = readdir(dp);
	while ( ep )
	{ 
		charArray_add(array_obj, ep->d_name);
		ep = readdir(dp);
	}
	(void) closedir(dp);

	return 1;    	
}

#else

// windows
#include <io.h>
#include <errno.h>
int ls_directory_to_array(const char * directory, charArray * array_obj)
{
	struct _finddata_t fileInfo;
	intptr_t objPtr;
	char attribs[8];
	//char timeBuff[32];
	int iret;
	int fret;
	char * find_directory = NULL;
	int strLen;

	if ( array_obj == NULL )
	{
		return -1;
	}

	strLen = strlen(directory);
	if ( strLen <= 0 ) // arbitrarily large enough number
	{
		return -10; // input directory invalid or too big
	}
	if ( strLen > 0xFFFF ) // arbitrarily large enough number
	{
		return -11; // input directory too big
	}

	find_directory = (char*) malloc( strLen + 4 );
	if ( find_directory == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	strcpy(find_directory, directory);
	if ( directory[strLen-1] == '\\' )
	{
		strcat(find_directory, "*");
	}
	else
	{
		strcat(find_directory, "\\*");
	}

	objPtr = _findfirst(find_directory, &fileInfo);
	//printf(" checking path : %s\n",directory);
	if ( objPtr == -1L )
	{
		free(find_directory);
		return -10;
	}

	fret = 1;
	while ( 1 )
	{
		// decode the archive flag
		attribs[0] =( fileInfo.attrib & _A_ARCH )   ? 'A' : '.';
		// decode the read only flag
		attribs[1] =( fileInfo.attrib & _A_RDONLY ) ? 'R' : '.';
		// decode the hidden file flag
		attribs[2] = ( fileInfo.attrib & _A_HIDDEN ) ? 'H' : '.';
		// decode the System file flag
		attribs[3] =( fileInfo.attrib & _A_SYSTEM ) ? 'S' : '.';
		// for readability
		attribs[4] = '-';
		// is this a subdirectory 'D' or a File 'F'
		attribs[5] = (fileInfo.attrib      & _A_SUBDIR ) ? 'D' : 'F';
		attribs[6] = 0x00;
		// get the time of the last write to this file
		//ctime( timeBuff, 30, &fileInfo.time_write );
		// other times available on non FAT systems are create time
		// fileInfo.time_access and fileInfo.time_write  these are -1 on FAT disks
		
		//printf("%-32s %s %9.ld %s",fileInfo.name, attribs, fileInfo.size , timeBuff);
		charArray_add(array_obj, fileInfo.name);

		iret = _findnext(objPtr, &fileInfo);
		if ( iret < 0 )
		{
			fret = -10; // generic findnext error
			if ( errno == ENOENT )
			{
				// finished properly
				fret = 1;
			}
			break;
		}
	} 

	_findclose(objPtr);
	
	free(find_directory);
	return fret;
}

#endif

// configArray: an array of charArray
int configArray_Constructor(configArray * root, int amount_max)
{
	//int i=0;
	//int j=0;

	if ( root == NULL )
	{
		return -1;
	}

	root->index = -1;

	root->row_amount = 0;
	if ( amount_max <= 0 )
	{
		// take default
		root->row_amount_max = CONFIG_ARRAY_ROW_AMOUNT;
	}
	else
	{
		root->row_amount_max = amount_max;
	}

	root->array = (charArray**) malloc( sizeof(charArray*) * root->row_amount_max );
	if ( root->array == NULL )
	{
		return -999;
	}
	memset(root->array, 0, sizeof(charArray*) * root->row_amount_max );

	root->cell_length = 0;

	return 1;	
}

int configArray_Destructor(configArray * root)
{
	int i;//,j;

	if ( root == NULL )
	{
		return -1;
	}

	if ( root->array )
	{
		for (i=0; i<root->row_amount_max; i++)
		{
			if ( root->array[i] )
			{
				charArray_Destructor(root->array[i]);
				free(root->array[i]);
				root->array[i] = NULL;
			}
		}
	}

	if ( root->array )
	{
		free(root->array);
		root->array = NULL;
	}

	return 1;
}


int configArray_realloc(configArray * root, int newsize)
{
	int i=0;
	//int j=0;
	//int index;
	//char * new_array;
	charArray ** new_double_array;
	
	if ( g_c_toolbox_verbose >= 2 )
	{
		printf("<!-- WARNING - configArray_realloc -->\n");
	}

	// realloc
	new_double_array = (charArray**) malloc( sizeof(charArray*) * newsize );
	if ( new_double_array == NULL )
	{
		return -999;
	}

	// copy old into new
	for (i=0; i<root->row_amount_max; i++)
	{
		new_double_array[i] = root->array[i]; // copy char *
	}

	// continue with new rows
	for (i=root->row_amount_max; i<newsize; i++)
	{
		new_double_array[i] = 0;
	}

	// cleanup of old array structure (not char data)
	free(root->array);
	root->array = new_double_array;
	root->row_amount_max = newsize;

	return 1;

}

// written 
// this will malloc a new charArray and add "array" content to it
// array_index will be loaded with the index in which the new array was placed
int configArray_add(configArray * root, const charArray * array, int _hint_array_row_amount_max, int * array_index)
{
	int i=0;
	int j=0;
	//int index;
	int iret;
	int hint_array_row_amount_max = _hint_array_row_amount_max;
	charArray * workarray=NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( array == NULL )
	{
		// allow
		// return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( array && array->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( array_index )
	{
		*array_index = -1;
	}

	if ( root->row_amount >= root->row_amount_max )
	{
		// realloc
		iret = configArray_realloc(root, root->row_amount_max+CONFIG_ARRAY_ROW_AMOUNT);
		if ( iret <= 0 )
		{
			if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
			{	return iret;
			}
			else
			{	return -20;
			}
		}
	}

	root->row_amount++;
	root->array[root->row_amount-1] = (charArray*) malloc( sizeof(charArray) );
	workarray = root->array[root->row_amount-1];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	if ( array_index )
	{
		*array_index = root->row_amount-1;
	}

	if ( array && array->row_amount > hint_array_row_amount_max )
	{
		hint_array_row_amount_max = array->row_amount;
	}
	charArray_Constructor(workarray, hint_array_row_amount_max);

	if ( array )
	{
		// copy content
		for (i=0;i<array->row_amount;i++)
		{
			if ( array->array[i][0] )
			{
				//printf("[charArray_add] (%s)\n", array->array[i][0]);
				charArray_add(workarray, array->array[i][0]);
			}

			for (j=1; j<array->col_amount; j++)
			{
				if ( array->array[i][j] )
				{
					//printf("[charArray_col_append] (%s)\n", array->array[i][j]);
					charArray_col_append(workarray, -1, array->array[i][j]);
				}
			}
		}
	}

	return 1;

}


int configArray_add_to_array(configArray * root, int row, const char * line)
{
	charArray * workarray=NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	//printf("[configArray_add_to_array] adding to row(%d) and root->row_amount(%d)\n", row, root->row_amount);
	if ( row < 0 )
	{
		row = root->row_amount-1; // at the end
	}
	if ( row >= root->row_amount ) 
	{
		row = root->row_amount-1;
		printf("[configArray_add_to_array] adjusting row to(%d)\n", row);	
	}

	workarray = root->array[row];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}
	if ( workarray->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	charArray_add(workarray, line);

	return 1;
}

int configArray_add_to_array_col_append(configArray * root, int row, const char * line)
{
	charArray * workarray=NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	if ( row < 0 )
	{
		row = root->row_amount-1;
	}
	if ( row >= root->row_amount ) 
	{
		row = root->row_amount-1;
	}

	workarray = root->array[row];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}
	if ( workarray->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	//printf("[configArray_add_to_array_col_append] going to append (%s) to workarray\n", line);
	charArray_col_append(workarray, -1, line);

	return 1;
}

int configArray_getArrayWithLineThatMatches(configArray * root, char * pattern, charArray ** out_array_insider)
{	
	int a;
	int iret;
	int found=0;
	char * insider = NULL;
	charArray * workarray = NULL;

	if ( out_array_insider )
	{
		(*out_array_insider) = NULL;
	}

	if ( root == NULL )
	{
		return -1;
	}
	if ( pattern == NULL )
	{
		return -2;
	}
	if ( out_array_insider == NULL )
	{
		return -3;
	}

	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{			
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		iret = charArray_getLineThatMatches(workarray, pattern, &insider);
		if ( iret > 0 && insider && strlen(insider) > 0 )
		{
			found = 1;
			break;
		}
	}

	if ( found )
	{
		*out_array_insider = workarray;
		return 1;
	}

	return 0;

}


int configArray_getArrayWithRowThatIs(configArray * root, int row, const char * search, charArray ** out_array_insider)
{	
	int a;
	int found=0;
	charArray * workarray=NULL;

	if ( out_array_insider )
	{
		(*out_array_insider) = NULL;
	}

	if ( root == NULL )
	{
		return -1;
	}
	if ( search == NULL )
	{
		return -2;
	}
	if ( out_array_insider == NULL )
	{
		return -3;
	}

	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{	
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		if ( strcmp(workarray->array[0][0], search)==0 )
		{
			found = 1;
			break;
		}
	}

	if ( found )
	{
		*out_array_insider = workarray;
		return 1;
	}

	return 0;

}

int configArray_getArrayThatHasColumnWithKeyEqualToWanted(configArray * root, char * pattern, int column, char * wanted, charArray ** array_insider)
{	
	int a;
	int iret;
	int found=0;
	char * insider=NULL;
	charArray * workarray=NULL;

	(*array_insider) = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( pattern == NULL )
	{
		return -2;
	}
	if ( wanted == NULL )
	{
		return -3;
	}
	if ( array_insider == NULL )
	{
		return -3;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{
#if 0
		if ( g_c_toolbox_verbose > 0 )
		{
			snprintf(php_message_buffer+strlen(php_message_buffer), 255,
				"[configArray_gATHCWKETW] s o l a(%d)<br>\n",a);
		}
#endif
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		//snprintf(php_message_buffer+strlen(php_message_buffer), 255, "[configArray_gATHCWKETW] charArray_getColumnWithKey pattern(%s), column(%d)<br>\n",pattern,column);
		iret = charArray_getColumnWithKey(workarray, pattern, column, &insider);
		//snprintf(php_message_buffer+strlen(php_message_buffer), 255, "[configArray_gATHCWKETW] charArray_getColumnWithKey returned iret(%d)<br>\n",iret);
		if ( iret > 0 && insider && strlen(insider) > 0 )
		{
			iret = regexp_match(wanted, insider);			
#if 0
			if ( g_c_toolbox_verbose > 0 )
			{
				snprintf(php_message_buffer+strlen(php_message_buffer), 255,
					"[configArray_gATHCWKETW] regexp_match(%s,%s) returned iret(%d)<br>\n",wanted,insider,iret);
			}
#endif
			if ( iret > 0 )
			{
				found = 1;
				break;
			}
		}
	}

	if ( found )
	{
		(*array_insider) = workarray;
#if 0
		if ( g_c_toolbox_verbose > 0 )
		{
			snprintf(php_message_buffer+strlen(php_message_buffer), 255, 
				"[configArray_gATHCWKETW] assigning workarray(0x%X) to array_insider(0x%X)<br>\n",
				(unsigned int)workarray, (unsigned int)*array_insider);
		}
#endif
		return 1;
	}

	return 0;

}

int configArray_splitTag(configArray * root, char * tag)
{
	int a;
	charArray * workarray=NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( tag == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( strlen(tag) <= 0 )
	{
		return -3;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_splitTag\n");
		charArray_splitTag(workarray, tag);
		//printf("after to charArray_splitTag\n");
	}

	return 1;
}

int configArray_to_file(configArray * root, const char * filename)
{
	int a;
	FILE * file = NULL;
	charArray * workarray = NULL;
	char temp[256];

	if ( root == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( strlen(filename) <= 0 )
	{
		return -3;
	}

	//printf("going to open (%s)\n", filename);
	file = fopen(filename,"wb");   
	//printf("after to open (%s), file(0x%X)\n", filename, file);

	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_to_FILE\n");
		snprintf(temp,255,"====charArray.delimiter====\n");
		fwrite(temp, 1, strlen(temp), file); 
		charArray_to_FILE_joined(workarray, file, "::");
		//printf("after to charArray_to_FILE\n");
	}

	fclose(file);

	return 1;

}


int configArray_to_FILE(configArray * root,  FILE * file)
{
	int a;
	charArray * workarray=NULL;
	char temp[256];

	if ( root == NULL )
	{
		return -1;
	}
	if ( file == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_to_FILE\n");
		snprintf(temp,255,"====charArray.delimiter====\n");
		fwrite(temp, 1, strlen(temp), file); 
		charArray_to_FILE_joined(workarray, file, "::");
		//printf("after to charArray_to_FILE\n");
	}

	return 1;

}


#if C_TOOLBOX_WIDESTRINGS == 1


// configArray: an array of charArray
// wide string version 26.Feb.2011
int configArray_ConstructorW(configArrayW * root, int amount_max)
{
	//int i=0;
	//int j=0;

	if ( root == NULL )
	{
		return -1;
	}

	root->index = -1;

	root->row_amount = 0;
	if ( amount_max <= 0 )
	{
		// take default
		root->row_amount_max = CONFIG_ARRAY_ROW_AMOUNT;
	}
	else
	{
		root->row_amount_max = amount_max;
	}

	root->array = (charArrayW**) malloc( sizeof(charArrayW*) * root->row_amount_max);
	if ( root->array == NULL )
	{
		return -999;
	}
	memset(root->array, 0, sizeof(charArrayW*) * root->row_amount_max );

	root->cell_length = 0;

	return 1;	
}

int configArray_DestructorW(configArrayW * root)
{
	int i;//,j;

	if ( root == NULL )
	{
		return -1;
	}

	if ( root->array )
	{
		for (i=0; i<root->row_amount_max; i++)
		{
			if ( root->array[i] )
			{
				charArray_DestructorW(root->array[i]);
				free(root->array[i]);
				root->array[i] = NULL;
			}
		}
	}

	if ( root->array )
	{
		free(root->array);
		root->array = NULL;
	}

	return 1;
}

static int configArray_reallocW(configArrayW * root, int newsize)
{
	int i=0;
	//int j=0;
	//int index;
	//char * new_array;
	charArrayW ** new_double_array;
	
	if ( g_c_toolbox_verbose >= 2 )
	{
		printf("<!-- WARNING - configArray_realloc -->\n");
	}

	// realloc
	new_double_array = (charArrayW**) malloc( sizeof(charArrayW*) * newsize );
	if ( new_double_array == NULL )
	{
		return -999;
	}

	// copy old into new
	for (i=0; i<root->row_amount_max; i++)
	{
		new_double_array[i] = root->array[i]; // copy char *
	}

	// continue with new rows
	for (i=root->row_amount_max; i<newsize; i++)
	{
		new_double_array[i] = 0;
	}

	// cleanup of old array structure (not char data)
	free(root->array);
	root->array = new_double_array;
	root->row_amount_max = newsize;

	return 1;
}

// written 
// this will malloc a new charArray and add "array" content to it
// array_index will be loaded with the index in which the new array was placed
int configArray_addW(configArrayW * root, const charArrayW * array, int _hint_array_row_amount_max, int * array_index)
{
	int i=0;
	int j=0;
	//int index;
	int iret;
	int hint_array_row_amount_max = _hint_array_row_amount_max;
	charArrayW * workarray = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( array == NULL )
	{
		// allow
		// return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( array && array->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( array_index )
	{
		*array_index = -1;
	}

	if ( root->row_amount >= root->row_amount_max )
	{
		// realloc
		iret = configArray_reallocW(root, root->row_amount_max+CONFIG_ARRAY_ROW_AMOUNT);
		if ( iret <= 0 )
		{
			if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
			{	return iret;
			}
			else
			{	return -20;
			}
		}
	}

	root->row_amount++;
	root->array[root->row_amount-1] = (charArrayW*) malloc( sizeof(charArrayW) );
	workarray = root->array[root->row_amount-1];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	if ( array_index )
	{
		*array_index = root->row_amount-1;
	}

	if ( array && array->row_amount > hint_array_row_amount_max )
	{
		hint_array_row_amount_max = array->row_amount;
	}
	charArray_ConstructorW(workarray, hint_array_row_amount_max);

	if ( array )
	{
		// copy content
		for (i=0;i<array->row_amount;i++)
		{
			if ( array->array[i][0] )
			{
				//printf("[charArray_add] (%s)\n", array->array[i][0]);
				charArray_addW(workarray, array->array[i][0]);
			}

			for (j=1; j<array->col_amount; j++)
			{
				if ( array->array[i][j] )
				{
					//printf("[charArray_col_append] (%s)\n", array->array[i][j]);
					charArray_col_appendW(workarray, -1, array->array[i][j]);
				}
			}
		}
	}

	return 1;
}


int configArray_add_to_arrayW(configArrayW * root, int row, const wchar_t * line)
{
	charArrayW * workarray = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	//printf("[configArray_add_to_array] adding to row(%d) and root->row_amount(%d)\n", row, root->row_amount);
	if ( row < 0 )
	{
		row = root->row_amount-1; // at the end
	}
	if ( row >= root->row_amount ) 
	{
		row = root->row_amount-1;
		wprintf(L"[configArray_add_to_array] adjusting row to(%d)\n", row);	
	}

	workarray = root->array[row];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}
	if ( workarray->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	charArray_addW(workarray, line);

	return 1;
}

int configArray_add_to_array_col_appendW(configArrayW * root, int row, const wchar_t * line)
{
	charArrayW * workarray = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( line == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	if ( row < 0 )
	{
		row = root->row_amount-1;
	}
	if ( row >= root->row_amount ) 
	{
		row = root->row_amount-1;
	}

	workarray = root->array[row];

	if ( workarray == NULL )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_PROBLEM_NULLPOINTER;
	}
	if ( workarray->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	//printf("[configArray_add_to_array_col_append] going to append (%s) to workarray\n", line);
	charArray_col_appendW(workarray, -1, line);

	return 1;
}

int configArray_getArrayWithLineThatMatchesW(configArrayW * root, wchar_t * pattern, charArrayW ** out_array_insider)
{	
	int a;
	int iret;
	int found=0;
	wchar_t * insider = NULL;
	charArrayW * workarray = NULL;

	if ( out_array_insider )
	{
		(*out_array_insider) = NULL;
	}

	if ( root == NULL )
	{
		return -1;
	}
	if ( pattern == NULL )
	{
		return -2;
	}
	if ( out_array_insider == NULL )
	{
		return -3;
	}

	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{			
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		iret = charArray_getLineThatMatchesW(workarray, pattern, &insider);
		if ( iret > 0 && insider && wcslen(insider) > 0 )
		{
			found = 1;
			break;
		}
	}

	if ( found )
	{
		*out_array_insider = workarray;
		return 1;
	}

	return 0;
}


int configArray_getArrayWithRowThatIsW(configArrayW * root, int row, const wchar_t * search, charArrayW ** out_array_insider)
{	
	int a;
	int found=0;
	charArrayW * workarray = NULL;

	if ( out_array_insider )
	{
		(*out_array_insider) = NULL;
	}

	if ( root == NULL )
	{
		return -1;
	}
	if ( search == NULL )
	{
		return -2;
	}
	if ( out_array_insider == NULL )
	{
		return -3;
	}

	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{	
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		if ( wcscmp(workarray->array[0][0], search)==0 )
		{
			found = 1;
			break;
		}
	}

	if ( found )
	{
		*out_array_insider = workarray;
		return 1;
	}

	return 0;

}

int configArray_getArrayThatHasColumnWithKeyEqualToWantedW(configArrayW * root, wchar_t * pattern, int column, wchar_t * wanted, charArrayW ** array_insider)
{	
	int a;
	int iret;
	int found=0;
	wchar_t * insider = NULL;
	charArrayW * workarray = NULL;

	(*array_insider) = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( pattern == NULL )
	{
		return -2;
	}
	if ( wanted == NULL )
	{
		return -3;
	}
	if ( array_insider == NULL )
	{
		return -3;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	if ( root->row_amount <= 0 )
	{
		return TOOLBOX_ERROR_INTERNAL_USAGE_EMPTY_ARRAY;
	}

	for (a=0; a<root->row_amount; a++)
	{
#if 0
		if ( g_c_toolbox_verbose > 0 )
		{
			snprintf(php_message_buffer+strlen(php_message_buffer), 255,
				"[configArray_gATHCWKETW] s o l a(%d)<br>\n",a);
		}
#endif
		workarray = root->array[a];

		if ( workarray == NULL )
		{
			continue;
		}

		//snprintf(php_message_buffer+strlen(php_message_buffer), 255, "[configArray_gATHCWKETW] charArray_getColumnWithKey pattern(%s), column(%d)<br>\n",pattern,column);
		iret = charArray_getColumnWithKeyW(workarray, pattern, column, &insider);
		//snprintf(php_message_buffer+strlen(php_message_buffer), 255, "[configArray_gATHCWKETW] charArray_getColumnWithKey returned iret(%d)<br>\n",iret);
		if ( iret > 0 && insider && wcslen(insider) > 0 )
		{
			iret = regexp_matchW(wanted, insider);			
#if 0
			if ( g_c_toolbox_verbose > 0 )
			{
				snwprintf(php_message_buffer+strlen(php_message_buffer), 255,
					"[configArray_gATHCWKETW] regexp_match(%s,%s) returned iret(%d)<br>\n",wanted,insider,iret);
			}
#endif
			if ( iret > 0 )
			{
				found = 1;
				break;
			}
		}
	}

	if ( found )
	{
		(*array_insider) = workarray;
#if 0
		if ( g_c_toolbox_verbose > 0 )
		{
			snprintf(php_message_buffer+strlen(php_message_buffer), 255, 
				"[configArray_gATHCWKETW] assigning workarray(0x%X) to array_insider(0x%X)<br>\n",
				(unsigned int)workarray, (unsigned int)*array_insider);
		}
#endif
		return 1;
	}

	return 0;

}

int configArray_splitTagW(configArrayW * root, wchar_t * tag)
{
	int a;
	charArrayW * workarray = NULL;

	if ( root == NULL )
	{
		return -1;
	}
	if ( tag == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( wcslen(tag) <= 0 )
	{
		return -3;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_splitTag\n");
		charArray_splitTagW(workarray, tag);
		//printf("after to charArray_splitTag\n");
	}

	return 1;
}

int configArray_to_fileW(configArrayW * root, const wchar_t * filename)
{
	int a;
	FILE * file = NULL;
	charArrayW * workarray = NULL;
	wchar_t temp[256];

	if ( root == NULL )
	{
		return -1;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}
	if ( filename == NULL )
	{
		return -2;
	}
	if ( wcslen(filename) <= 0 )
	{
		return -3;
	}

	//printf("going to open (%s)\n", filename);
	file = wfopen(filename, L"wb");   
	//printf("after to open (%s), file(0x%X)\n", filename, file);

	if ( file == NULL )
	{
		return TOOLBOX_ERROR_CANNOT_OPEN_FILE;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_to_FILE\n");
		snwprintf(temp, sizeof(temp)/2-1, L"====charArray.delimiter====\n");
		fwrite(temp, 1, wcslen(temp)*2, file);
		charArray_to_FILE_joinedW(workarray, file, L"::");
		//printf("after to charArray_to_FILE\n");
	}

	fclose(file);

	return 1;
}

int configArray_to_FILEW(configArrayW * root, FILE * file)
{
	int a;
	charArrayW * workarray = NULL;
	wchar_t temp[256];

	if ( root == NULL )
	{
		return -1;
	}
	if ( file == NULL )
	{
		return -2;
	}
	if ( root->array == NULL )
	{
		return TOOLBOX_ERROR_NOT_INITIALIZED;
	}

	for (a=0; a<root->row_amount; a++)
	{
		workarray = root->array[a];
		if ( workarray == NULL )
		{
			continue;
		}
		//printf("going to charArray_to_FILE\n");
		snwprintf(temp, sizeof(temp)/2-1, L"s====charArray.delimiter====\n");
		fwrite(temp, 1, wcslen(temp)*2, file); 
		charArray_to_FILE_joinedW(workarray, file, L"::");
		//printf("after to charArray_to_FILE\n");
	}

	return 1;

}

#endif

#if C_TOOLBOX_CONFIG_MD5 == 1
int md5_encrypt(const char * original, char ** insider_encrypted)
{
	int runlength=32;
	static const char hex[]="0123456789abcdef";
	static char buffer[1024]="";
	MD5_CTX ctx;
	int i;
	const unsigned char * _original = (unsigned char *) original;
	unsigned char digest[1024]; // digest can be as small as runlength

	if ( original == NULL )
	{
		return -1;
	}
	if ( insider_encrypted == NULL )
	{
		return -1;
	}
	if ( runlength > 1024 )
	{
		return -10;
	}

	*insider_encrypted = NULL;

	our_MD5Init(&ctx);
	our_MD5Update(&ctx, _original, strlen(original) );
	our_MD5Final(digest, &ctx);

	for (i=0; i<runlength; i++)
	{
		buffer[i*2] = hex[digest[i]>>4];
		buffer[i*2+1] = hex[digest[i]&0x0f];
	}
	buffer[runlength] = '\0';

	*insider_encrypted = &buffer[0];

	return 1;
}

#endif



int C_FileExists(const char * filename)
{
	FILE * test;
	int ret = 0;

	if ( filename == NULL || filename[0] == '\0' )
	{
		return -1;
	}

	test = fopen(filename, "r");
	if (test == NULL)
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		fclose(test);
	}
	return ret;
}


int C_DirectoryExists(const char * directory)
{
	//FILE * test;
	//int ret = 0;
#if ( defined(_MSC_VER) )
	DWORD attributes = GetFileAttributes(directory);
	if ( attributes != INVALID_FILE_ATTRIBUTES
	  && attributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		return 1;
	}
#	if _DEBUG
	attributes = GetLastError();
	attributes = attributes;
#	endif
	return 0;
#else
	DIR * dirPtr;
	dirPtr = opendir(directory);
	if ( dirPtr == NULL )
	{
		return 0;
	}
	closedir(dirPtr);
	return 1;
#endif
}



// by default will overwrite file
int C_CopyFile(const char * source, const char * destination)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -1;
	}

#if ( defined(_MSC_VER) )
	bret = CopyFile(source,destination,FALSE/*bFailIfExists*/);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		printf("WIN32 CopyFile failed with error(%d)\r\n", (int)GetLastError());
#endif
		return -10;
	}
#else
	snprintf(command,255,"cp -f \"%s\" \"%s\"", source,destination);
	system(command);
#endif

	return 1;
}

// flags are agregates of gnucFlags_E
// written 08.Jan.2008
int C_MoveFileEx(const char * source, const char * destination, int flags)
{
#if ( defined(_MSC_VER) )
	BOOL bret;
	DWORD win32_flags = MOVEFILE_WRITE_THROUGH;
#else
	char command[256];
#endif

	if ( source == NULL )
	{
		return -1;
	}
	if ( destination == NULL )
	{
		return -2;
	}

#if ( defined(_MSC_VER) )
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{	win32_flags |= MOVEFILE_REPLACE_EXISTING;
	}
	win32_flags |= MOVEFILE_COPY_ALLOWED;
	bret = MoveFileEx(source, destination, win32_flags);
	if ( bret == FALSE )
	{
		// error
#if _DEBUG
		int win32_error = GetLastError();
		printf("WIN32 MoveFile failed with error(%d)\r\n", win32_error);
#endif
		return -10;
	}
#else
	if ( flags & TOOLBOX_OVERWRITE_DESTINATION )
	{
		snprintf(command,255,"mv -f \"%s\" \"%s\"", source,destination);
	}
	else
	{
		snprintf(command,255,"mv \"%s\" \"%s\"", source,destination);
	}
	system(command);
#endif

	return 1;
}



// written 19.Feb.2011
int C_endSlashDirectory(char * directory_inout)
{
	int backslashes = 0;
	char * strRet;
	char * strRet2;
		
	strRet = strstr(directory_inout, "\\");
	if ( strRet ) 
	{
		backslashes = 1;
	}

	strRet = C_strendstr(directory_inout, "\\");
	strRet2 = C_strendstr(directory_inout, "/");
	if ( strRet == NULL && strRet2 == NULL )
	{
		if ( backslashes ) 
		{	strcat(directory_inout, "\\");
		}
		else
		{	strcat(directory_inout, "/");
		}
	}

	return 1;
}




int C_GetTempFilename(const char * path, char * out_filename)
{
#if defined(_MSC_VER)
	char temp[256]="";
#endif


	if ( out_filename == NULL )
	{
		return -1;
	}

#if defined(_MSC_VER)
	// windows
	if ( tmpnam(temp)==NULL )
	{
		return -12;
	}
	if ( temp[0] == '\\' )
	{
		int length = strlen(temp);
		memmove(temp, temp+1, length); // copy the last byte '\0'
		temp[length-1] = '\0'; // redundant
	}
	out_filename[0] = '\0';
	if ( path )
	{
		strcpy(out_filename, path);
		if ( path[strlen(path)-1] != '/'
			&& path[strlen(path)-1] != '\\'
			)
		{
			// append slash
			strcat(out_filename, "/");
		}
	}
	strcat(out_filename, temp);
#else
	// linux
	int outfd; 
	if ( path )
	{
		strcpy(out_filename, path);
		if ( path[strlen(path)-1] != '/'
			&& path[strlen(path)-1] != '\\'
			)
		{
			// append slash
			strcat(out_filename, "/");
		}
	}
	strcat(out_filename, "XXXXXX");
	outfd = mkstemp(out_filename);
	if (outfd == -1) 
	{
		printf("<!-- mkstemp on filename(%s) failed -->\n", out_filename);
		return -10;
	}
	close(outfd);
	/* Fix the permissions */
	if (chmod(out_filename, 0600) != 0)
	{
		unlink(out_filename);
		return -11;
	}
#endif
	return 1;
}


int TOOLBOX_GetTempFilenameW(const wchar_t * path, wchar_t * out_filename)
{
#if defined(_MSC_VER)
	wchar_t temp[256]=L"";
#endif

	if ( out_filename == NULL )
	{
		return -1;
	}

#if defined(_MSC_VER)
	// windows
	if ( _wtmpnam(temp)==NULL )
	{
		return -12;
	}
	if ( temp[0] == L'\\' )
	{
		int length = wcslen(temp);
		memmove(temp, temp+2, length*2); // copy the last byte '\0'
		temp[length-1] = L'\0'; // redundant
	}
	out_filename[0] = L'\0';
	if ( path )
	{
		wcscpy(out_filename, path);
		if ( path[wcslen(path)-1] != L'/'
		  && path[wcslen(path)-1] != L'\\'
			)
		{
			// append slash
			wcscat(out_filename, L"/");
		}
	}
	wcscat(out_filename, temp);
#else
	// linux
	// todo: wide string version
	int outfd; 
	if ( path )
	{
		strcpy(out_filename, path);
		if ( path[strlen(path)-1] != '/'
			&& path[strlen(path)-1] != '\\'
			)
		{
			// append slash
			strcat(out_filename, "/");
		}
	}
	strcat(out_filename, "XXXXXX");
	outfd = mkstemp(out_filename);
	if (outfd == -1) 
	{
		//printf("<!-- mkstemp on filename(%s) failed -->\n", out_filename);
		return -10;
	}
	close(outfd);
	/* Fix the permissions */
	if (chmod(out_filename, 0600) != 0)
	{
		unlink(out_filename);
		return -11;
	}
#endif
	return 1;
}


int C_itoa( unsigned int i, char * toLoad, unsigned int toLoad_size )
{
	char temp[22];
	char *p = &temp[21];

	*p-- = '\0';

	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);

	strncpy(toLoad, p+1, toLoad_size-1);
	toLoad[toLoad_size-1] = '\n';

	return 1;
}

// this function has been obtained somewhere on the web.
int C_axtoi(const char * hex)
{
	int n = 0; // position in string
	int m = 0; // position in digit[] to shift
	int count; // loop index
	int intValue = 0; // integer value of hex string
	int digit[32]; // hold values to convert
	if ( strstr(hex, "0x")==hex )
	{	hex+=2;
	}
	while (n < 32)
	{
		if (hex[n] == '\0')
		{	break; }
		if (hex[n] >= 0x30 && hex[n] <= 0x39 )	//if 0 to 9
		{	digit[n] = hex[n] & 0x0f; }	//convert to int
		else if (hex[n] >= 'a' && hex[n] <= 'f') //if a to f
		{	digit[n] = (hex[n] & 0x0f) + 9; } //convert to int
		else if (hex[n] >= 'A' && hex[n] <= 'F') //if A to F
		{	digit[n] = (hex[n] & 0x0f) + 9; } //convert to int
		else
		{	break; }
		n++;
	}
	count = n;
	m = n - 1;
	n = 0;
	while (n < count)
	{
		// digit[n] is value of hex digit at position n
		// (m << 2) is the number of positions to shift
		// OR the bits into return value
		intValue = intValue | (digit[n] << (m << 2));
		m--; // adjust the position to set
		n++; // next digit to process
	}
	return (intValue);
}

// will replace in place
char* C_strreplace(char * source, const char token, const char replacement)
{
	char * strRet;
	
	if (source == NULL) return NULL;
	if (token == replacement) return source;
	
	strRet = strchr(source, token);
	while (strRet)
	{	*strRet = replacement;
		strRet = strchr(source, token);
	}
	return source;
}

// written 04.Feb.2009
// will write at most dst_max_length and always NULL-terminate the string.
int C_strncpy(char * destination, const char * source, int dst_max_length)
{
	if ( destination == NULL ) 
	{	return -1;
	}
	if ( source == NULL ) 
	{	return -2;
	}
	if ( dst_max_length <= 0 ) 
	{	return -3;
	}

	while ( *source != '\0' )
	{
		*destination = *source++;
		dst_max_length--;
		if ( dst_max_length == 0 )
		{
			*destination = '\0';
			break;
		}		
		destination++;
	}

	return 1;
}

// returns NULL on error or not found
// returns pointer from inside string when found, at beginning of needle in string.
char * C_strcasestr(const char * string, const char * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = strlen(string);
	needleLen = strlen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (char*) string+i;
			}
			if ( tolower(string[i+j]) != tolower(needle[j]) )
			{
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}

// returns NULL on error or not found
// returns pointer from inside string when found, at beginning of needle in string.
wchar_t * C_strcasestrW(const wchar_t * string, const wchar_t * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = wcslen(string);
	needleLen = wcslen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (wchar_t*) string+i;
			}
			if ( towlower(string[i+j]) != towlower(needle[j]) )
			{
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}


char * C_strncasestr(const char * string, int string_length, const char * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = string_length;
	needleLen = strlen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (char*) string+i;
			}
			if ( string[i+j] == '\0' 
				|| ( tolower(string[i+j]) != tolower(needle[j]) )
				)
			{
				// string is over.
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}



#if C_TOOLBOX_WIDESTRINGS == 1

// Written 26.Feb.2011
wchar_t * TOOLBOX_strncasestrW(const wchar_t * string, int string_length, const wchar_t * needle)
{
	int i;
	int j;
	int stringLen;
	int needleLen;
	int searchLen;
    
	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLen = string_length;
	needleLen = wcslen(needle);
	searchLen = stringLen - needleLen + 1;

	for (i=0; i<searchLen; i++)
	{
		for (j=0; ; j++)
		{
			if ( j == needleLen )
			{
				return (wchar_t*) string+i;
			}
			if ( string[i+j] == L'\0' 
			  || ( tolower(string[i+j]) != tolower(needle[j]) ) // frank fixme with wide strings
				)
			{
				// string is over.
				// we are not interested in this i
				break;
			}
		}
	}
	
	return NULL; // not found
}

#endif

// Author: Francois Oligny-Lemieux
// Created: 22.Mar.2007
// if string ends with needle, return pointer to first char of needle in string
// else return NULL
char * C_strendstr(const char * string, const char * needle)
{ 	
	unsigned int stringLength;
	unsigned int needleLength;
	char * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = strlen(string);
	needleLength = strlen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = (char*)strstr(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}



// Author: Francois Oligny-Lemieux
// Created: 19.Jul.2011 (based on char version)
// if string ends with needle, return pointer to first char of needle in string
// else return NULL
wchar_t * C_strendstrW(const wchar_t * string, const wchar_t * needle)
{ 	
	unsigned int stringLength;
	unsigned int needleLength;
	wchar_t * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = wcslen(string);
	needleLength = wcslen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = wcsstr(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}
// Author: Francois Oligny-Lemieux
// Created: 22.Mar.2007
// if string ends with needle (case insensitive), return pointer to first char of needle in string
// else return NULL
char * C_striendstr(const char * string, const char * needle)
{
	unsigned int stringLength;
	unsigned int needleLength;
	char * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = strlen(string);
	needleLength = strlen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = C_strcasestr(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}


#if C_TOOLBOX_WIDESTRINGS == 1

// Author: Francois Oligny-Lemieux
// Created: 22.Mar.2007
// if string ends with needle (case insensitive), return pointer to first char of needle in string
// else return NULL
wchar_t * TOOLBOX_striendstrW(const wchar_t * string, const wchar_t * needle)
{
	unsigned int stringLength;
	unsigned int needleLength;
	wchar_t * strRet;

	if ( string == NULL )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}

	stringLength = wcslen(string);
	needleLength = wcslen(needle);

	if ( needleLength > stringLength )
	{
		return NULL;
	}

	// string[stringLength] represents '\0'
	// string[stringLength-needleLength] represents theorical start of needle in string
	strRet = C_strcasestrW(&string[stringLength-needleLength], needle);
	if ( strRet )
	{
		return strRet;
	}

	return NULL;
}
#endif


// Author: Francois Oligny-Lemieux
// Created: 18.May.2007
// Binary-safe strlen
int C_strlen(const char * string, int string_buffer_size)
{
	uint8_t success = 0;
	int i = 0;
	if ( string == NULL )
	{
		return -1;
	}

	while ( i<string_buffer_size )
	{
		if ( string[i] == '\0' )
		{
			success = 1;
			break;
		}
		i++;
	}

	if ( success == 0 )
	{
		return -1;
	}
	return i;
}



// Author: Francois Oligny-Lemieux
// Created: 15.Oct.2010 (based on char version)
// Binary-safe strlen
int TOOLBOX_strlenW(const wchar_t * string, int string_buffer_size)
{
	uint8_t success = 0;
	int i = 0;
	if ( string == NULL )
	{
		return -1;
	}

	while ( i<string_buffer_size/2 ) 
	{
		if ( string[i] == L'\0' )
		{
			success = 1;
			break;
		}
		i++;
	}

	if ( success == 0 )
	{
		return -1;
	}
	return i;
}
// written 04.Feb.2009, returns end of string
const char * C_eos(const char * string)
{
	if ( string == NULL )
	{
		return NULL;
	}

	while ( *string != '\0' )
	{
		string++;
	}

	return string;
}

// written 20.Aug.2007
void * C_memfind(const unsigned char * buffer, int buffer_length, const unsigned char * needle, int needle_length)
{
	unsigned int i;
	unsigned int j; 
	//unsigned int found;
	void * out_value = NULL;

	if ( buffer == NULL )
	{
		return NULL;
	}
	if ( buffer_length <= 0 )
	{
		return NULL;
	}
	if ( needle == NULL )
	{
		return NULL;
	}
	if ( needle_length <= 0 )
	{
		return NULL;
	}

	for (i=0; i<(unsigned int)buffer_length; i++)
	{
		for (j=0; j<(unsigned int)needle_length; j++)
		{
			if ( buffer[i+j] != needle[j] )
			{
				break;
			}
		}
		if ( j == needle_length )
		{
			out_value = (void*)&buffer[i];
			return out_value;
		}
	}

	return NULL;
}


#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
int putTextBufferReader_into_charArray(textBufferReader * reader, charArray * array)
{
	int iret;
	char buffer[TEXT_BUFFER_READER_MAX_LINE_SIZE+1];

	if ( reader == 0 )
	{
		return -1;
	}
	if ( array == 0 )
	{
		return -1;
	}

	iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);

	while ( iret > 0 || iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL || iret == TOOLBOX_WARNING_CONTINUE_READING )
	{	
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] ERROR_BUFFER_TOO_SMALL\n");
		}
#if TOOLBOX_DEBUG_LEVEL >= 1
		else if ( g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
#endif
		if ( iret == TOOLBOX_ERROR_BUFFER_TOO_SMALL && g_c_toolbox_verbose >= 1 )
		{
			printf("[putTextBufferReader_into_charArray] got line buffer(%s)\n",buffer);
		}
		if ( iret != TOOLBOX_WARNING_CONTINUE_READING )
		{
			// frank fixme, in case TOOLBOX_WARNING_CONTINUE_READING I trim line... so it's not 100% success
			// frank fixme, test with sample cases
			charArray_add(array, buffer);
		}
		iret = TextBufferReader_GetLine(reader, buffer, TEXT_BUFFER_READER_MAX_LINE_SIZE);
	}

#if TOOLBOX_DEBUG_LEVEL >= 1
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[putTextBufferReader_into_charArray] End of function\n");
	}
#endif

	return 1;
}


// will append to array.
int buffer_to_array(const char * buffer, charArray * array_ptr)
{	
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}

// written 11.Jun.2008
int buffer_to_arrayEx(const char * buffer, charArray * array_ptr, int chop_empty_lines)
{
	int iret;
	textBufferReader reader;
	//lineParser parser;
	//charArray * splitted = NULL;

	if (array_ptr==NULL)
	{
		return -1;
	}
	if (buffer==NULL)
	{
		return -2;
	}

	//charArray_Destructor(array_ptr);
	//charArray_Constructor(array_ptr, 1024);

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to TextBufferReader_Constructor with strlen(buffer)=(%u)\n",strlen(buffer));
	}
#endif

	iret = TextBufferReader_Constructor(&reader, buffer, strlen(buffer)+1);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] TextBufferReader_Constructor returned iret(%d)\n",iret);
	}
#endif

	if ( iret <= 0 )
	{
		if ( TOOLBOX_IS_TOOLBOX_SPECIFIC_ERROR(iret) )
		{	return iret;
		}
		else
		{	return -20;
		}
	}

	reader.chop_empty_lines = chop_empty_lines;

#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] going to putTextBufferReader_into_charArray\n");
	}
#endif

	putTextBufferReader_into_charArray(&reader, array_ptr);
	
#if TOOLBOX_DEBUG_LEVEL >= 2
	if ( g_c_toolbox_verbose >= 1 )
	{
		printf("[buffer_to_array] putTextBufferReader_into_charArray returned iret(%d)\n",iret);
	}
#endif

	iret = TextBufferReader_Destructor(&reader);

	return 1;
}
#endif


// Modified 27.Jun.2007 for multi-thread safety
#define RESULT_SIZE 1024*512
int C_System2(const char * command, char * loadme, unsigned int loadme_size, int * out_status)
{
#if !defined(_MSC_VER)
	char temp[256];
#endif
	FILE * file;
	FILE * fileErr;
	char tmpFilename[256] = "";
	char tmpFilenameErr[256] = "";
	char full_command[256];
	int status;
	int iret;
	int fret = 1;
	unsigned int result_written = 0;
	unsigned int read_size = 24;
	int size_read = 0;

#if C_TOOLBOX_LOG_COMMAND == 1
	static unsigned int log_fileSize = 0;
	uint64_t fileSize = 0;
#endif

	if ( command == NULL )
	{
		return -1;
	}

	if ( loadme == NULL )
	{
		return -2;
	}

	if ( loadme_size == 0 )
	{
		return -3;
	}

	loadme[loadme_size-1] = '\0';

#if ( defined(_MSC_VER) )
	iret = C_GetTempFilename(tmpPath,tmpFilename);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -10;
	}	
#else
	snprintf(temp, 255, "%s/tempio/", tmpPath);
	iret = C_DirectoryExists(temp);
	if ( iret <= 0 )
	{
		snprintf(temp, 255, "mkdir %s/tempio/", tmpPath);
		system(temp);
		snprintf(temp, 255, "%s/tempio/", tmpPath);
	}

	iret = C_GetTempFilename(temp,&tmpFilename[0]);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -11;
	}
	iret = C_GetTempFilename(temp,&tmpFilenameErr[0]);
	if ( iret <= 0 )
	{
		//printf("<!-- C_GetTempFilename returned iret(%d) -->\n",iret);
		return -12;
	}
#endif

	strcpy(full_command,command);
	/* pipe output in file */
	strcat(full_command," > ");
	strcat(full_command,tmpFilename);
#if ( !defined(_MSC_VER) )
	strcat(full_command," 2> ");
	strcat(full_command, tmpFilenameErr);
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
	if ( g_toolbox_toolbox_log_file == NULL )
	{
		iret = C_GetFileSize(C_TOOLBOX_LOG_FILENAME, &fileSize);
		if ( iret > 0 && fileSize > 1024*1024 )
		{
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "w");
		}
		else
		{
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "aw");
		}
	}

	if ( g_toolbox_toolbox_log_file )
	{
		if ( log_fileSize > 1024*1024 )
		{
			fclose(g_toolbox_toolbox_log_file);
			g_toolbox_toolbox_log_file = fopen(C_TOOLBOX_LOG_FILENAME, "w");
		}
		fwrite(full_command, 1, strlen(full_command), g_toolbox_toolbox_log_file);
		fwrite("\n", 1, 1, g_toolbox_toolbox_log_file);
		fflush(g_toolbox_toolbox_log_file);
		log_fileSize += strlen(full_command)+1;
	}
#endif

	status = system(full_command);
	if (out_status) *out_status = status;

#if defined(_MSC_VER)
	file = fopen(tmpFilename, "r");
	if ( file == NULL )
	{
		return -12;
	}
#else
	fileErr = fopen(tmpFilenameErr, "r");
	file = fopen(tmpFilename, "r");
	if ( file == NULL && fileErr == NULL )
	{
		return -12;
	}
#endif

	if (file)
	{
		if ( loadme_size-1-result_written < read_size )
		{	read_size = loadme_size-1-result_written;
		}
		size_read = fread(loadme+result_written, 1, read_size, file);
		//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
		while ( size_read > 0 )
		{
			result_written += size_read;
			*(loadme+result_written) = '\0';
	
			if ( loadme_size-1-result_written < read_size )
			{	read_size = loadme_size-1-result_written;
			}
	
			if ( read_size == 0 )
			{
				fret=-13; // buffer is too small
				break;
			}
			
			size_read = fread(loadme+result_written, 1, read_size, file);
		}
		fclose(file);
		
		if (fileErr)
		{
			if ( loadme_size-1-result_written < read_size )
			{	read_size = loadme_size-1-result_written;
			}
			size_read = fread(loadme+result_written, 1, read_size, fileErr);
			//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
			while ( size_read > 0 )
			{
				result_written += size_read;
				*(loadme+result_written) = '\0';
		
				if ( loadme_size-1-result_written < read_size )
				{	read_size = loadme_size-1-result_written;
				}
		
				if ( read_size == 0 )
				{
					fret=-13; // buffer is too small
					break;
				}
				
				size_read = fread(loadme+result_written, 1, read_size, fileErr);
			}
			fclose(fileErr);
		}
	}
	
	// delete temporary file
#if ( defined(_MSC_VER) )
	C_DeleteFile(tmpFilename);
#else
	unlink(tmpFilename);
	unlink(tmpFilenameErr);
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
	if ( g_toolbox_toolbox_log_file )
	{
		snprintf(temp, 255, " ... command passed through ... ");
		fwrite(temp, 1, strlen(temp), g_toolbox_toolbox_log_file);
		fwrite("\n", 1, 1, g_toolbox_toolbox_log_file);
		fflush(g_toolbox_toolbox_log_file);
	}
#endif

	return fret;
}

// we supply the char buffer from static memory (not thread safe)
int C_System(const char * command, char ** insider, int * status)
{
	//char temp[256];
	int fret = 1;
	static char result[RESULT_SIZE];
	unsigned int result_written = 0;

	result[0] = '\0';
	result_written = 0;

	result[RESULT_SIZE-1] =- '\0';

	if ( insider )
	{
		*insider = &result[0];
	}
	else
	{
		return -2;
	}

	if ( command == NULL )
	{
		return -1;
	}

	fret = C_System2(command, result, RESULT_SIZE, status);

	return fret;
}


int C_GetFileSize(const char * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#else
	struct stat statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _stati64(filename, &statObject);
#else
	iret = stat(filename, &statObject);
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_size;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}

#if C_TOOLBOX_WIDESTRINGS == 1

int TOOLBOX_GetFileSizeW(const wchar_t * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#elif defined(C_TOOLBOX_32BIT)
	struct stat statObject;
#else
	struct stati64 statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _wstati64(filename, &statObject);
#else
	iret = wstat(filename, &statObject); // never tested
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_size;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}
#endif


int C_GetFileCreationTime(const char * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#else
	struct stat statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _stati64(filename, &statObject);
#else
	iret = stat(filename, &statObject);
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_ctime;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}


#if C_TOOLBOX_WIDESTRINGS == 1

int TOOLBOX_GetFileCreationTimeW(const wchar_t * filename, uint64_t * loadme)
{
	int iret;
#if defined(_MSC_VER)
	struct _stati64 statObject;
#elif defined(C_TOOLBOX_32BIT)
	struct stat statObject;
#else
	struct stati64 statObject;
#endif

	if (loadme==NULL)
	{
		return -1;
	}

#if defined(_MSC_VER)
	iret = _wstati64(filename, &statObject);
#else
	iret = statW(filename, &statObject);
#endif

	if ( iret < 0 )
	{
		return -10;
	}
	
	*loadme = statObject.st_ctime;
	//printf("fileSize("I64u")\n",statObject.st_size);
	return 1;
}
#endif


#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
int C_GetNetworkInformation(char * machine_ip,
							   char * subnet,
							   char * broadcast,
							   char * mac,
							   char * RX_packets,
							   char * RX_errors,
							   char * RX_bytes,
							   char * TX_packets,
							   char * TX_errors,
							   char * TX_bytes,
							   char * collisions,
							   char * ifconfig, /*INOUT*/
							   int ifconfig_size,
							   const char * which_interface)
{
	char temp[256];
	char command[256];
	char _line[256];
	char * line = &_line[0];
	charArray array;
	lineParser parser;
	char * insider = NULL;
	char * borrowed = NULL;
	char * position = NULL;
	int iret;
	int length;
	int status;

	temp[255] = '\0';

#if 0
	if ( machine_ip == NULL )
	{
		return -1;
	}
	if ( subnet == NULL )
	{
		return -2;
	}
	if ( broadcast == NULL )
	{
		return -3;
	}
	if ( mac == NULL )
	{
		return -4;
	}
	if ( RX_packets == NULL )
	{
		return -5;
	}
	if ( RX_errors == NULL )
	{
		return -6;
	}
	if ( RX_bytes == NULL )
	{
		return -6;
	}
	if ( TX_packets == NULL )
	{
		return -7;
	}
	if ( TX_errors == NULL )
	{
		return -8;
	}
	if ( TX_bytes == NULL )
	{
		return -8;
	}
	if ( collisions == NULL )
	{
		return -8;
	}
#endif
	
	if (ifconfig && ifconfig[0] != '\0')
	{
		insider = ifconfig;
		iret = 1;
	}
	else
	{
#if ( defined(_MSC_VER) )
		strcpy(command,"ipconfig /all");
		iret = C_System(command, &insider, &status);
#else
		strcpy(command,"ifconfig");
		iret = C_System(command, &insider, &status);
#endif
	}

	if ( iret <= 0 )
	{
		return -10;
		// C_System error
	}

	if ( insider == NULL )
	{
		return -11;
		// C_System error
	}
	
	if (ifconfig && ifconfig[0] == '\0')
	{
		strncpy(ifconfig, insider, ifconfig_size-1);
		ifconfig[ifconfig_size-1] = '\0';
	}

	if ( machine_ip ) machine_ip[0] = '\0';
	if ( subnet ) subnet[0] = '\0';
	if ( broadcast ) broadcast[0] = '\0';
	if ( mac ) mac[0] = '\0';
	if ( RX_packets ) RX_packets[0] = '\0';
	if ( RX_errors ) RX_errors[0] = '\0';
	if ( RX_bytes ) RX_bytes[0] = '\0';
	if ( TX_packets ) TX_packets[0] = '\0';
	if ( TX_errors ) TX_errors[0] = '\0';
	if ( TX_bytes ) TX_bytes[0] = '\0';
	if ( collisions ) collisions[0] = '\0';

	if ( strlen(insider)==0 )
	{
		return -12;
		// probably the command was good or there is no network adaptor.
	}
	
	if (which_interface != NULL && which_interface[0] != '\0')
	{
		printf("will seek to %s\n", which_interface);
		strcpy(line, "\n");
		strcat(line, which_interface);
		position = strstr(insider, line);
		if (position)
		{
			insider = position;
		}
		else
		{
			return 0; // not found
		}
	}

	charArray_Constructor(&array,0);
	lineParser_Constructor(&parser);
	
#if ( defined(_MSC_VER) )
	buffer_to_array(insider, &array);
	//file_to_array("g:\\tmp\\ifconfig.txt", &array); 

	iret = charArray_getLineThatMatches(&array, "IP Address", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, ": ");
		length = position - line + 2;
		if ( position != NULL )
		{
			strncpy(temp, line+length, 255);
			position = strstr(temp, " ");
			if ( position )
			{
				position[0]='\0';
			}
			if ( machine_ip ) strcpy(machine_ip, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "Physical Address", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, ": ");
		length = position - line + 2;
		if ( position != NULL )
		{
			strncpy(temp, line+length, 255);
			position = strstr(temp, " ");
			if ( position )
			{
				position[0]='\0';
			}
			if ( mac ) strcpy(mac, temp);
		}
	}
#else
	buffer_to_array(insider, &array);
	
	iret = charArray_getLineThatMatches(&array, "HWaddr", &borrowed);
	if ( iret && borrowed )
	{
		position = strstr(borrowed, "HWaddr");
		if ( position != NULL )
		{
			strcpy(temp, position+6);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( mac ) strcpy(mac, temp);
		}
	}
    
	iret = charArray_getLineThatMatches(&array, "inet addr:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "inet addr:");
		if ( position )
		{
			length = position - line + 10;

			position = strstr(line, "Bcast");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( machine_ip ) strcpy(machine_ip, temp);
			}
		}
	}
    
	iret = charArray_getLineThatMatches(&array, "Bcast:", &borrowed);
	if ( iret && borrowed )
	{	
		strcpy(line,borrowed);
		position = strstr(line, "Bcast:");
		if ( position )
		{
			length = position - line + 6;

			position = strstr(line, "Mask");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( broadcast ) strcpy(broadcast, temp);
			}
		}
	}
	
	iret = charArray_getLineThatMatches(&array, "Mask:", &borrowed);
	if ( iret && borrowed )
	{
		position = strstr(borrowed, "Mask:");
		if ( position != NULL )
		{
			strcpy(temp, position+5);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( subnet ) strcpy(subnet, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "RX packets:");
		if ( position )
		{
			length = position - line + 11;
			position = strstr(line, "errors:");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_packets ) strcpy(RX_packets, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "errors:");
		if ( position )
		{
			length = position - line + 7;
			position = strstr(line, "dropped:");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_errors ) strcpy(RX_errors, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "RX bytes:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "RX bytes:");
		if ( position )
		{
			length = position - line + 9;
			position = strstr(line, "(");
			if ( position != NULL )
			{
				position[0]='\0';
				strcpy(temp, line+length);
				iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
				if ( iret > 0 && insider )
				{
					strcpy(temp, insider );
				}
				if ( RX_bytes ) strcpy(RX_bytes, temp);
			}
		}
	}

	iret = charArray_getLineThatMatches(&array, "TX bytes:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "TX bytes:");
		length = position - line + 9;

		position = strstr(position, "(");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_bytes ) strcpy(TX_bytes, temp);
		}
	}

	iret = charArray_getLineThatMatches(&array, "TX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "TX packets:");
		length = position - line + 11;

		position = strstr(line, "errors:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_packets ) strcpy(TX_packets, temp);
		}
	}
    	
	iret = charArray_getLineThatMatches(&array, "TX packets:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "errors:");
		length = position - line + 7;

		position = strstr(line, "dropped:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( TX_errors ) strcpy(TX_errors, temp);
		}
	}
	
	iret = charArray_getLineThatMatches(&array, "collisions:", &borrowed);
	if ( iret && borrowed )
	{
		strcpy(line,borrowed);
		position = strstr(line, "collisions:");
		length = position - line + 11;

		position = strstr(line, "txqueuelen:");
		if ( position != NULL )
		{
			position[0]='\0';
			strcpy(temp, line+length);
			iret = lineParser_stripSpacing(&parser, temp, 1,1, &insider);
			if ( iret > 0 && insider )
			{
				strcpy(temp, insider );
			}
			if ( collisions ) strcpy(collisions, temp);
		}
	}
#endif
    
	charArray_Destructor(&array);
	lineParser_Destructor(&parser);

	return 1;
}
#endif

#if defined(__TOOLBOX_NETWORK_H__)
int C_GetHostname(char * hostname, int hostname_bufsize)
{
	int iret;
	int status;
	char * insider = NULL;

	if ( hostname == NULL )
	{
		return -1;
	}

	if ( hostname_bufsize <= 0 )
	{
		return -2;
	}

	hostname[0] = '\0';

#if defined(_MSC_VER)
	iret = gethostname(hostname, hostname_bufsize-1);
	if ( iret == SOCKET_ERROR )
	{
		iret = WSAGetLastError();
		if ( iret == WSANOTINITIALISED )
		{
			Network_Initialize();
			iret = gethostname(hostname, hostname_bufsize-1);
		}
	}
	if ( iret == 0 )
	{
		return 1;
	}
	return -10; // failed ?? :(
#else
	iret = C_System("hostname", &insider, &status);
	if ( iret > 0 && insider )
	{
		char * strRet;
		strncpy(hostname, insider, hostname_bufsize-1);
		hostname[hostname_bufsize-1] = '\0';
		strRet = strchr(hostname, '\r');
		if ( strRet )
		{
			*strRet = '\0';
		}
		strRet = strchr(hostname, '\n');
		if ( strRet )
		{
			*strRet = '\0';
		}
	}
#endif
	return 1;
}
#endif

#if defined(__C_TOOLBOX_TEXT_BUFFER_READER_H__)
// written 17.May.2007
int C_GetDefaultGateway(char * gateway, int gateway_bufsize)
{
	int iret;
	int status;
	int fret = -20; // general not succeeded
	char * line;
	char * insider = NULL;
	char * strRet;
	int row;
	int count = 0;
	charArray array;
	lineParser parser;

	if ( gateway == NULL )
	{
		return -1;
	}

	if ( gateway_bufsize <= 0 )
	{
		return -2;
	}

	gateway[0] = '\0';
	charArray_Constructor(&array, 20);
	lineParser_Constructor(&parser);

#if defined(_MSC_VER)
	iret = C_System("route PRINT", &insider, &status);
#else
	iret = C_System("route -n", &insider, &status);
#endif

	if ( iret > 0 && insider )
	{
		buffer_to_array(insider, &array);
		row = -1;
		iret = charArray_getLineThatMatchesEx(&array, "0.0.0.0", 0, -1, &row, &insider);
		while ( iret > 0 && insider && count < 100 )
		{
			line = insider;
			insider = NULL;
			iret = lineParser_stripSpacing(&parser, line, 1, 0, &insider);
			if ( iret <= 0 || insider == NULL )
			{
				insider = line;
			}
			if ( strstr(insider, "0.0.0.0")==insider ) // matches begining
			{
				// there must be two 0.0.0.0 on the line to be the wanted line
				strRet = strstr(insider+7, "0.0.0.0");
				if ( strRet )
				{
					// it's my line
					insider = NULL;
					iret = lineParser_replaceAll(&parser, line, "0.0.0.0", "", &insider);
					if ( insider )
					{
						iret = lineParser_stripSpacing(&parser, insider, 1, 0, &insider);
						if ( iret > 0 && insider )
						{
							strRet = strchr(insider, ' ');
							if ( strRet )
							{
								*strRet = '\0';
								if ( strlen(insider) >= (unsigned int)gateway_bufsize )
								{
									printf("ERROR - Found gateway is bigger than supplied buffer size(%d)\n", gateway_bufsize);
								}
								else
								{
									strcpy(gateway, insider);
									fret = 1;
									break;
								}
							}
						}
					}
					
				}
				else
				{
					// not default gateway line
				}
			}
			charArray_removeLineWithIndexAndAllBefore(&array, row);
			count++;
			row = -1;
			iret = charArray_getLineThatMatchesEx(&array, "0.0.0.0", 0, -1, &row, &insider);
		}

	}
	else
	{
		fret = -10; // executing command failed.
	}
	
	lineParser_Destructor(&parser);
	charArray_Destructor(&array);
	return fret;
}
#endif

int C_Sleep(int milliseconds)
{
#if ( defined(_MSC_VER) )
	Sleep((DWORD)milliseconds);
#else
	usleep(1000*milliseconds);
#endif
	return 1;
}

static const char * const g_rand_source_alphanum = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char * const g_rand_source_alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char * const g_rand_source_num = "0123456789";
static const char * const g_rand_source_num_nozero = "123456789123456789";
static int g_srand_done = 0;
// will write length char in toload + '\0', thus toload must be at least length+1 big
int C_Random_alpha(int length, char * toload, int buffer_size)
{
	int random_value = 0;
	unsigned int srandom_value = 0;
	char temp[256]="";
	int count=0;
	//int iret;
	int i;
	int number;
	char letter[3]="";

	if ( toload == NULL )
	{
		return -1;
	}
	
	if ( length <= 0 )
	{
		return -2;
	}

	if ( length >= buffer_size || length >= 127 )
	{
		return -3;
	}

	temp[254]='\0';
	temp[255]='\0';
    
	if ( g_srand_done == 0 )
	{
		srandom_value = (unsigned int) time(NULL);
        srand(srandom_value);
		g_srand_done = 1;
	}
	random_value = rand();

	//printf("<!-- rand(%d) -->\n",random_value);

	snprintf(temp, 255, "%d",random_value);

	while ( count < 200 )
	{
		random_value = rand();
		snprintf(temp+strlen(temp),255-strlen(temp),"%d",random_value);
		if ( temp[254] != '\0' )
		{
			// done
			break;
		}
		count++;
	}

	for (i=0; i<length; i++)
	{
		//number = atoi(&temp[i]);
		letter[0] = temp[i];
		letter[1] = temp[i+1];
		letter[2] = '\0';
		number = atoi(letter);
		if ( strlen(g_rand_source_alpha) > (unsigned int) number )
		{
			toload[i] = g_rand_source_alpha[number];
		}
		else
		{
			letter[1] = '\0';
			number = atoi(letter);
			toload[i] = g_rand_source_alpha[number];
		}
	}
	if ( length < buffer_size )
	{
		toload[length]='\0';
	}
	else
	{
		toload[buffer_size-1]='\0';
	}

	return 1;
}

int C_Random_numeric(int length, char * toload, int buffer_size)
{
	int random_value = 0;
	unsigned int srandom_value = 0;
	char temp[256]="";
	int count=0;
	int i;
	int number;
	char letter[2]="";

	if ( toload == NULL )
	{
		return -1;
	}
	
	if ( length <= 0 )
	{
		return -2;
	}
    
	if ( length > 255 )
	{
		return -3;
	}
    
	memset(&temp[0], 0, 256); // important for snprintf below since I don't add '\0' everytime
	
	if ( g_srand_done == 0 )
	{
		srandom_value = (unsigned int) time(NULL);
        srand(srandom_value);
		g_srand_done = 1;
	}
	random_value = rand();

	snprintf(temp,255,"%d",random_value);

	while ( count < 20 )
	{	
		if ( strlen(temp) >= (unsigned int) length )
		{
			break;
		}
		random_value = rand();
		snprintf(temp+strlen(temp),255-strlen(temp),"%d",random_value);
		count++;
	}
	
	if ( strlen(temp) < (unsigned int) length )
	{
		length = strlen(temp);
	}

	for (i=0; i<length; i++)
	{
		letter[0] = temp[i];
		letter[1] = '\0';
		number = atoi(letter);
		if ( number>0 && (unsigned int) number < strlen(g_rand_source_num)  )
		{
			toload[i] = g_rand_source_num[number];
		}
		else
		{
			toload[i] = '0';
		}
	}
	if ( length < buffer_size )
	{
		toload[length]='\0';
	}
	else
	{
		toload[buffer_size-1]='\0';
	}

	return 1;
}

int C_Tolower(char * source)
{
	unsigned int i=0;
	if ( source == NULL )
	{
		return -1;
	}

	for (i=0; i<strlen(source); i++)
	{
		if ( source[i]=='\0' )
		{
			break;
		}

		source[i]=tolower(source[i]);
	}

	return 1;
}

int C_memFind(unsigned char * input, unsigned int inputLength, unsigned char * needle, unsigned int needleLength, unsigned char ** out_position_in_input)
{
	unsigned int inputLengthRemaining = inputLength;
	unsigned int inputOffset = 0;
	int iret;

	if ( input == NULL )
	{
		return -1;
	}
	if ( inputLength == 0 )
	{
		return -2;
	}
	if ( needle == NULL )
	{
		return -3;
	}
	if ( needleLength == 0 )
	{
		return -4;
	}
	if ( inputLength < needleLength )
	{
		return 0;
	}

	while (inputLengthRemaining >= needleLength)
	{
		iret = memcmp(input+inputOffset, needle, needleLength);
		if ( iret == 0 )
		{
			// we have a match
			if ( out_position_in_input )
			{
				*out_position_in_input = input + inputOffset;
			}
			return 1;
		}
		inputOffset += 1;
		inputLengthRemaining -= 1;
	}
	
	return 0;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		 
// C_duplicateString
//				
// Written by : Francois Oligny-Lemieux
//    Created : 17.Apr.2007
//   Modified : 
//
//  Description: 
//    > malloc a new string and copy the input content into it.
//    > WARNING: you need to free yourself the string.
//  
//   input (IN) original string
//   output (OUT) will be assigned the new buffer pointer
//   output_size (OUT) will be filled with the new buffer size
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int C_duplicateString(const char * input, char ** output, unsigned int * output_size)
{
	unsigned int strLen;
	unsigned int ouputSize;
	if ( input == NULL )
	{
		return -1;
	}
	if ( output == NULL )
	{
		return -2;
	}

	*output = NULL;
	if ( output_size )
	{
		*output_size = 0;
	}

	strLen = strlen(input);

	// 25mb in hex is 0x1900000 bytes
	if ( strLen > 0x1900000 )
	{
		return -10; // way to big
	}

	ouputSize = strLen+1;

	*output = (char*)malloc(sizeof(char) * ouputSize);
	if ( *output == NULL )
	{
		return TOOLBOX_ERROR_MALLOC;
	}

	if ( output_size )
	{
		*output_size = ouputSize;
	}

	memcpy(*output, input, strLen);
	(*output)[strLen] = '\0';

	return 1;
}

#if C_TOOLBOX_TIMING == 1
unsigned int C_Timestamp()
{
	unsigned int retval=0;
#if ( defined(_MSC_VER) )
	retval = (unsigned int) timeGetTime();
#else
	struct timeval tv_date;
	mtime_t delay; /* delay in msec, signed to detect errors */

	/* see mdate() about gettimeofday() possible errors */
	gettimeofday( &tv_date, NULL );

	/* calculate delay and check if current date is before wished date */
	delay = (mtime_t) tv_date.tv_sec * 1000000 + (mtime_t) tv_date.tv_usec;
	delay = delay/1000;
	retval = (unsigned int) delay;

#endif
	return retval;
}

#endif // end C_TOOLBOX_TIMING


// written 30.Jul.2007
int C_getCompileDate(char * string, int string_size)
{
	char temp[32];
	char date[32] = __DATE__;
	char * month = NULL;
	char * day = NULL;
	char * year = NULL;
	char * strRet;

	if ( string == NULL )
	{
		return -1;
	}
	if ( string_size < 31 )
	{
		return TOOLBOX_ERROR_YOU_PASSED_A_BUFFER_TOO_SMALL;
	}

	temp[0] = '\0';
	temp[31] = '\0';
	string[0] = '\0';
	string[31] = '\0';
	strRet = strchr(date, ' ');
	month = &date[0];

	if ( strRet )
	{
		*strRet = '\0';
		if ( *(strRet+1) == ' ' )
		{	*(strRet+1) = '0';
		}
		day = strRet+1;
		strRet = strchr(strRet+1, ' ');
		if ( strRet )
		{
			*strRet = '\0';
			year = strRet+1;
		}
	}
	snprintf(string, 31, "%s.%s.%s", day, month, year);
	//sprintf(temp, "Build Date: %s", frank_date);

	return 1;
}


int C_isDigit(const char * string)
{
	if (string == NULL)
	{
		return 0;
	}

	if (strspn(string, "0123456789") == strlen(string))
	{
		// entirely numbers
		return 1;
	}
	else
	{
		return 0;
	}
}

int C_isMulticast(const char * string)
{
	lineParser parser;
	charArray * insider = NULL;
	int iret;
	int fret = 0;
	int a;
	if (string == NULL)
	{
		return 0;
	}

	lineParser_Constructor(&parser);

	iret = lineParser_splitTag(&parser, string, ".", &insider);
	if (iret > 0 && insider)
	{
		// range 224.0.0.0 to 224.0.0.255 are non-routable
		if (insider->array[0][0] && insider->array[3][0])
		{
			a = atoi(insider->array[0][0]);
			if (a >= 224)
			{
				a = atoi(insider->array[1][0]);
				if (a > 0)
				{
					fret = 1;
				}
				else
				{
					a = atoi(insider->array[2][0]);
					if (a > 0)
					{
						fret = 1;
					}
				}
			}
		}
	}

	lineParser_Destructor(&parser);

	return fret;
}

int C_Destructor()
{
#ifdef C_LOGGER
	if (g_gnuc_log)
	{
		fclose(g_gnuc_log);
		g_gnuc_log = NULL;
	}
#endif

#if C_TOOLBOX_LOG_COMMAND == 1
	if (g_gnuc_toolbox_log_file)
	{
		fclose(g_gnuc_toolbox_log_file);
		g_gnuc_toolbox_log_file = NULL;
	}
#endif

	return 1;
}



#ifdef TOOLBOX_STANDALONE
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests
// unit tests

int TOOLBOX_TIMEOUT = 1800; //seconds before timeout

configArray HTML_Variables;

int logged;
int successReadingAccounts;

int validaccount;
int g_test_number = 1;

char php_message_buffer[200000] = "";
char php_message_buffer_overrun[256] = "";
//char login_database_filename[256]="g:\\tmp\\login.dat";
//char pre_login_database_filename[256]="g:\\tmp\\pre.login.dat";
//#include "constant_strings.c"
//#include "constant_strings_tasman.c"

const char * const html_sample_string = "</td>\
  </tr>\
</table>\
</form><!-- id_form_main -->\
<script language=\"JavaScript1.1\">\
//refreshBothStatus();\
</script>";


const char * const html_sample_string2 = "  Session created - ID : 1\n<br>";
const char * const html_sample_string3 = "  Session created - ID : 1\r\n<br>";

int test_toolbox_system_file_reader(const char * filename, char * loadme, unsigned int loadme_size);


// structures for self tests

typedef struct msString_S
{	char * buffer;
	unsigned int buffersize;
	uint64_t unique_id;
} msString_T;

typedef struct simpleItem_S
{
	unsigned int number;
} simpleItem_T;

static int linkedlist_self_test_sort_callback_01(void * a, void * b)
{
	unsigned int itemA = (unsigned int)a;
	unsigned int itemB = (unsigned int)b;

	if ( itemA > itemB )
	{
		return 1;
	}

	if ( itemA < itemB )
	{
		return -1;
	}

	return 0;
}


static int linkedlist_self_test_sort_callback(void * a, void * b)
{
	simpleItem_T * itemA = (simpleItem_T*)a;
	simpleItem_T * itemB = (simpleItem_T*)b;

	if ( itemA->number > itemB->number )
	{
		return 1;
	}

	if ( itemA->number < itemB->number )
	{
		return -1;
	}

	return 0;
}

// this one is for the unit test only
// will insert into childs of opaque1 treeItem_T
static int fileEntryCallback(const char *name, const WEB_FILESYSTEM_ENTRY *entry, void * opaque1, void * opaque2)
{
   int iret;
   treeItem_T * newElement = NULL;
   treeItem_T * level = (treeItem_T *)opaque1;
   genericTree_T * tree;

   if (opaque1 == NULL) return -3;

   tree = (genericTree_T *)level->tree;

   printf("name:%s\n", name);
   printf("entry->stuff:" I64u "\n", entry->size);

   iret = genericTree_Insert(tree, level, name, 0, NULL, &newElement);
   if ( iret > 0 && newElement )
   {
      newElement->client = malloc(sizeof(WEB_FILESYSTEM_ENTRY));
      if ( newElement->client == NULL )
      {
         genericTree_Delete(tree, newElement);
         return TOOLBOX_ERROR_MALLOC;
      }

      memcpy(newElement->client, entry, sizeof(WEB_FILESYSTEM_ENTRY));
      newElement->parent = level;

      if ( entry->isDirectory )
      {
         traverseDir(name, fileEntryCallback, newElement, NULL);      
      }
      return 1;
   }

   return -10;


}
   

int test_toolbox_GetFileExtension(const char * filename, const char * expected)
{
   char * result = C_GetFileExtension(filename);
   if (result == NULL && expected != NULL) 
   {
        printf("Failed to get extension for filename(%s), got NULL but expected(%s)\n", filename, expected);
        return -1;
   }
   
   if (result == NULL && expected == NULL) 
   {
        printf("Success get extension for filename(%s), got NULL and expected NULL\n", filename);
        return 1;
   }

   if (strcmp(result, expected) != 0)
   {
        printf("Failed get extension for filename(%s), got (%s) but expected(%s)\n", filename, result, expected);
        return -1;
   }

   printf("Success get extension for filename(%s), got (%s) and expected(%s)\n", filename, result, expected);
   return 1;
}


//#include "login.c"

int main(int argc, char *argv[])
{
	int iret, i;
	int a;
	int errors = 0;
	char temp[512] = "";
	char buffer[1024] = "";
	wchar_t bufferW[1024] = L"";
	char document[256] = "";
	char content_string[256] = "";
	char * char_ptr = NULL;
	char * cookie = NULL;
	char * insider = NULL;
	uint64_t fileSize = 0;
	uint64_t fileSize2 = 0;
	MD5_CTX ctx;
	char password[256] = "manager";
	unsigned char digest[32];
	static const char hex[] = "0123456789abcdef";
	char string_with_tag1[1024] = "frank::manager::HASH(21333213)::OBJ(12333)";
	char string_with_tag2[] = "::manager::HASH(21333213)::OBJ(12333)";
	char string_with_tag3[] = "::manager::HASH(21333213)::OBJ(12333)::";
	char string_with_tag4[] = "::manager";
	char string_with_tag5[] = "lonely";
	char string_with_delimiters[] = "frank::manager,:,HASH(21333213)::OBJ(12333)";
	char preset_filename[] = "configuration.audiovideo.001.txt";
	char * tag_ptr;
	char * out_ptr = NULL;
	int strLen;
	char text_buffer[] = "Unsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nCould not find slot=1) data!\nGroup data Invalid SPI=1/1/4Could not find slot=1) data!\nGroup data Invalid SPI=1/1/5\n";
	wchar_t text_bufferW[] = L"Unsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nUnsupported Card Type=4 for GET\nCould not find slot=1) data!\nGroup data Invalid SPI=1/1/4Could not find slot=1) data!\nGroup data Invalid SPI=1/1/5\n";
	char bigbuf[25000] = "";
	charArray entries;
	charArray html;
	charArray array;
	charArrayW arrayUTF16;
	charArrayW wideArray;
	textFileReader reader;
	textBufferReader breader;
	lineParser parser;
	charArray * splitted = NULL;
	charArray * array_inside = NULL;
	charArray * array_insider = NULL;
	char phrase[10]="";
	//char toBeEscaped[256]="hello how\'s it \"g\"oing ?&&?@\" well i hope";
	char toBeEscaped[256] = "hello \"\"\" oki";
	char toExtractDoubleQuotes[256] = "-sdp_filename \"\"";
	char * strRet;

	unsigned int timeStart = 0;
	unsigned int timeEnd = 0;
	
   g_c_toolbox_verbose = 3;

	iret = lineParser_Constructor(&parser);
	charArray_Constructor(&array,0);

	printf("<pre>\n");

	//SANDBOX BEGINs (experimenting stuff unrelated to TOOLBOX_Toolbox)
#if 0
	TOOLBOX_System("ipconfig", &insider);
	return 0;
#endif
	
#if 0
	phrase[9] = '\0';
	snprintf(phrase, 9, "Very long string that will overflow");
	fputs(phrase, stdout);
	return 0;
#endif
	//SANDBOX ENDs

   iret = test_toolbox_GetFileExtension("test.txt", "txt");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.ts", "ts");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.mp4", "mp4");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.jpeg", "jpeg");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test.", "");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("test", NULL);
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("a.mp4", "mp4");
   if (iret <= 0) return -1;
   iret = test_toolbox_GetFileExtension("a.b", "b");
   if (iret <= 0) return -1;

#if 1 
   {
      genericTree_T genericTree;
      genericTree_Constructor(&genericTree);
      traverseDir("c:/temp", fileEntryCallback, &genericTree.top, NULL);
      genericTree_Destructor(&genericTree);
   }
#endif

	// buffer_to_array test
#if 1
	errors = 0;
	iret = buffer_to_array(html_sample_string, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 1 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 1\n", array.row_amount);
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array\n");
	}

	charArray_Zero(&array);
	
	errors = 0;
	iret = buffer_to_array(html_sample_string2, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 2 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 2\n", array.row_amount);
		errors++;
	}
	if ( strcmp(array.array[0][0], "  Session created - ID : 1")!=0 )
	{
		printf("ERROR - buffer_to_array first line didn't match\n");
		errors++;
	}
	if ( strcmp(array.array[1][0], "<br>")!=0 )
	{
		printf("ERROR - buffer_to_array second line didn't match\n");
		errors++;
	}
	if ( array.array[2][0] == NULL || array.array[2][0][0] == '\0' )
	{// ok
	}
	else
	{
		printf("ERROR - buffer_to_array third line has text and shouldn't\n");
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array on sample 2\n");
	}
	
	charArray_Zero(&array);

	errors = 0;
	iret = buffer_to_array(html_sample_string3, &array);
	if ( iret <= 0 )
	{
		printf("ERROR - buffer_to_array returned iret(%d)\n", iret);
		errors++;
	}
	if ( array.row_amount != 2 )
	{
		printf("ERROR - buffer_to_array row_amount(%d) != 2\n", array.row_amount);
		errors++;
	}
	if ( strcmp(array.array[0][0], "  Session created - ID : 1")!=0 )
	{
		printf("ERROR - buffer_to_array first line didn't match\n");
		errors++;
	}
	if ( strcmp(array.array[1][0], "<br>")!=0 )
	{
		printf("ERROR - buffer_to_array second line didn't match\n");
		errors++;
	}
	if ( array.array[2][0] == NULL || array.array[2][0][0] == '\0' )
	{// ok
	}
	else
	{
		printf("ERROR - buffer_to_array third line has text and shouldn't\n");
		errors++;
	}
	if ( errors == 0 )
	{
		printf("SUCCESS - buffer_to_array on sample 3\n");
	}
	charArray_Destructor(&array);
#endif

	// TextFileReader test 
#if 1
	//iret = TextFileReader_Constructor(&reader, "/home/flemieux/TOOLBOX_ToolBox.SVN/hostname_list.txt");
	charArray_Constructor(&entries,0);
	iret = TextFileReader_Constructor(&reader, "regression_test_data\\nightflight.txt");
	printf("TextFileReader_Constructor iret(%d)\n", iret);
	putTextFileReader_into_charArray(&reader, &entries);
	
	if ( entries.row_amount != 355 )
	{
		printf("ERROR - TextFileReader, row_amount(%d) not 355\n", entries.row_amount);
		charArray_print(&entries);
	}

	i=1;
	if ( entries.array[i-1][0]==NULL || entries.array[0][0][0] != '\0' )
	{	printf("ERROR - TextFileReader, first line do not match\n");errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "           Entity Begins")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Name: nightflight00@206.162.164.36:5060")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Index: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Encoder Card: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Encoder Port: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Decoder Card: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "Entity Decoder Port: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     ~~~~~ Session/Call ~~~~~~~~~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call ID: 104b71c0-0-13c4-45014-105d-6241bc51-105d")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call Index: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Call Status: Terminated")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Local Entity Name: nightflight00@206.162.164.36:5060")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Local Entity isRTSP: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Remote Entity Name: 8888@64.71.156.103")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Remote Entity isRTSP: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Conference started since: not started")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Time since last event: 1020502 ms")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "     Terminated: 1")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ~~~~~~ RTP Map Local Begins ~~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: MP4A-LATM")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 32000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpAddress: 206.162.164.36")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpPort: 60000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: H264")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 90000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpAddress: 206.162.164.36")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       RtpPort: 60002")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Profile IDC: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Level IDC: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ConstraintSet012: 0")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       ~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "very long line with special case \\r\\n not in same buffer adsf lookout:tfrAAA1 adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsf adsf adsf  adsfE")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);
		printf("strlen(%d) and should be 1022\n", strlen(entries.array[i-1][0]));errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "~~~~~ RTP Map Remote Begins ~~~~")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	
	i=318;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Name: H264")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "       Codec Rate: 90000")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;

	i=353;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "-----------------------------------")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;
	if ( entries.array[i-1][0]==NULL || strcmp(entries.array[i-1][0], "")!=0 )
	{	printf("ERROR - TextFileReader, line %d do not match\n", i);errors++;
	}i++;

	if ( entries.row_amount != 355 )
	{
		printf("ERROR - TextFileReader, row_amount(%d) not 355\n", entries.row_amount);errors++;
	}

	if ( errors == 0 )
	{
		printf("SUCCESS - TextFileReader\n");
	}

	TextFileReader_Destructor(&reader);
	charArray_Destructor(&entries);

	//goto end_of_self_tests;
#endif

	// file_to_array test
#if 1
	charArray_Constructor(&entries,0);
	iret = file_to_array("regression_test_data\\substitution.htm", &entries);
	if ( entries.row_amount != 6 )
	{
		printf("ERROR - substitution.htm, row_amount(%d) not 6\n", entries.row_amount);
		charArray_print(&entries);
	}
	else
	{
		iret = charArray_substituteInLineThatMatches(&entries, "REPLACE_IP_ADDRESS", "127.0.0.1", TOOLBOX_ALL);
		if ( iret <= 0 )
		{		
			printf("ERROR - substitution.htm, charArray_substituteInLineThatMatches returned iret(%d)\n", iret);
		}

		if ( strcmp(entries.array[3][0], "var g_address = \"127.0.0.1\";")==0 )
		{		
			printf("Success - substitution.htm, charArray_substituteInLineThatMatches\n");
		}
		else
		{		
			printf("Error - substitution.htm, charArray_substituteInLineThatMatches\n");
		}
	}
	charArray_Destructor(&entries);
#endif
	

	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00";
		iret = TOOLBOX_utf8strlen(utf8string, 11);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test1) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test1)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0x46";
		iret = TOOLBOX_utf8strlen(utf8string, 12);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test2) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test2)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0xC0\0x80";
		iret = TOOLBOX_utf8strlen(utf8string, 13);
		if ( iret != 4 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test3) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test3)\n");	
		}
	}
	{
		const char * utf8string = "\xC0\x80\xE0\x80\x80\xF0\x80\x80\x80\x45\x00\0xC0\0x80";
		iret = TOOLBOX_utf8strlen(utf8string+1, 12);
		if ( iret != 3 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test4) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test4)\n");
		}
	}
	{
		const char * utf8string = "\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x00";
		iret = TOOLBOX_utf8strlen(utf8string, 10);
		if ( iret != 9 )
		{
			printf("ERROR - TOOLBOX_utf8strlen (test5) output wrong result\n");	
		}
		else
		{
			printf("Success - TOOLBOX_utf8strlen (test5)\n");
		}
	}


#if 1
	{// Test of charArrayW
		wchar_t wBuffer01[70000];
		wchar_t wBuffer02[70000];
		
		memset(wBuffer01, 0, sizeof(wBuffer01));
		memset(wBuffer02, 0, sizeof(wBuffer02));
		charArray_ConstructorW(&arrayUTF16, 100);
		iret = file_to_arrayW(L"regression_test_data\\UTF8_sample_01.txt", TOOLBOX_TEXT_ENCODING_UTF8, &arrayUTF16);
		if ( iret >= 1 )
		{
			if ( arrayUTF16.row_amount != 212 )
			{
				printf("ERROR - file_to_arrayW on UTF8_sample_01.txt didn't read correct line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);	
			}
			else
			{
				printf("Success - file_to_arrayW on UTF8_sample_01.txt, line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF8_sample_01.txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&arrayUTF16, wBuffer01, sizeof(wBuffer01));

		charArray_ConstructorW(&wideArray, 100);

		iret = file_to_arrayW(L"regression_test_data\\UTF16_sample_02 (little endian).txt", TOOLBOX_TEXT_ENCODING_UTF16, &wideArray);
		if ( iret >= 1 )
		{
			if ( wideArray.row_amount != 212 )
			{
				printf("ERROR - file_to_arrayW on UTF16_sample_01 (big endian).txt didn't read correct line amount expected(212) actual(%d)\n", wideArray.row_amount);	 // this one fails with row_amount == 211
			}
			else
			{
				printf("Success - file_to_arrayW on UTF16_sample_01 (big endian).txt, line amount expected(212) actual(%d)\n", wideArray.row_amount);
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF16_sample_01 (big endian).txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&wideArray, wBuffer02, sizeof(wBuffer02));

		iret = charArray_cmpW(&arrayUTF16, &wideArray);
		if ( iret != 1 )
		{
			printf("ERROR - Failed char array compare of both UTF file. They should be the same\n");
		}

		charArray_DestructorW(&arrayUTF16);
		charArray_DestructorW(&wideArray);

		a = wcslen(wBuffer01);
		i = wcslen(wBuffer02);
		if ( a != i )
		{
			printf("ERROR - Failed UTF8 and UTF16 reading do not give the same length.\n");
		}

		//memset(wBuffer01+1005, 0, sizeof(wBuffer01)-2000);
		//memset(wBuffer02+1005, 0, sizeof(wBuffer02)-2000);
		iret = wcscmp(wBuffer01, wBuffer02);
		if ( iret != 0 )
		{
			printf("ERROR - Failed UTF8 and UTF16 reading differ.\n");
		}
		else
		{
			printf("Success - UTF8 and UTF16 reading produce same results.\n");
		}
	}
	
	{// Test of charArrayW
		wchar_t wBuffer01[70000];
		wchar_t wBuffer02[70000];
		
		memset(wBuffer01, 0, sizeof(wBuffer01));
		memset(wBuffer02, 0, sizeof(wBuffer02));
		charArray_ConstructorW(&arrayUTF16, 3000);
		// test an file that ends with no newline (has caused a bug in the past)
		iret = file_to_arrayW(L"regression_test_data\\object.htt", TOOLBOX_TEXT_ENCODING_UTF8, &arrayUTF16);
		if ( iret >= 1 )
		{
			if ( arrayUTF16.row_amount != 2930 )
			{
				printf("ERROR - file_to_arrayW on object.htt didn't read correct line amount expected(2930) actual(%d)\n", arrayUTF16.row_amount);	
			}
			else
			{
				if ( wcscmp(arrayUTF16.array[2929][0], L"</html>") == 0 )
				{
					printf("Success - file_to_arrayW on object.htt, line amount expected(212) actual(%d)\n", arrayUTF16.row_amount);
				}
				else
				{
					printf("Error - last line was not good\n");
				}
			}
		}
		else
		{
			printf("ERROR - test_toolbox_system_file_reader on UTF8_sample_01.txt failed with iret(%d)\n", iret);
		}
		charArray_to_bufferW(&arrayUTF16, wBuffer01, sizeof(wBuffer01));

		charArray_ConstructorW(&wideArray, 100);

	}
#endif

#if 1 // utf8 to wide oveflow test
	{
		char * original = "one two three";
		wchar_t result[13];
		result[12] = L'A';
		iret = TOOLBOX_utf8ToWide(original, result, sizeof(result));
		if ( iret > 0 )
		{
			if ( result[12] != L'\0' )
			{
				printf("ERROR - TOOLBOX_utf8ToWide didn't terminate buffer.\n");
			}
			else
			{
				printf("Success - TOOLBOX_utf8ToWide\n");
			}
		}
		else
		{
			if ( result[12] != L'\0' )
			{
				printf("ERROR - TOOLBOX_utf8ToWide didn't terminate buffer and failed with iret(%d).\n", iret);
			}
			else
			{
				printf("PASS - TOOLBOX_utf8ToWide failed with iret(%d) but terminated buffer.\n", iret);
			}
		}
	}
#endif


#if 1 // snwprintf() with wchar_t
	{
		wchar_t lead[8] = L"a";
		wchar_t result[8] = L"";
		wchar_t bound[8] = L"b";
		int strlen;

		memset(lead, 4, sizeof(lead));
		memset(bound, 5, sizeof(bound));
		memset(result, 6, sizeof(lead));
		//result[sizeof(result)/2-1] = '\0';
		snwprintf(result, sizeof(result)/2-1, L"this is a very long string that will likely overflow");
		strlen = wcslen(result);
		if ( strlen > 7 ) 
		{
			printf("ERROR - snwprintf() bad usage\n");
		}
		wprintf(result);

	}
#endif

	
#if 1 // snwprintf() with wchar_t
	{
		const char * firstPart = "Calavamous moucho coucho one two three jackie go";
		const char * secondPart = " super long artefact that will reach above 30 char, hopeing it will work out ok at the end";
		const wchar_t * firstPartW = L"Calavamous moucho coucho one two three jackie go";
		const wchar_t * secondPartW = L" super long artefact that will reach above 30 char, hopeing it will work out ok at the end";
		wchar_t expectedResult[1024] = L"";
		unsigned int destination_size = 30;
		wchar_t * destination = malloc(30);

		memset(destination, 0, 30);
		C_AppendUtf8toWide(&destination, &destination_size, firstPart, strlen(firstPart), NULL, NULL);
		C_AppendUtf8toWide(&destination, &destination_size, secondPart, strlen(secondPart), NULL, NULL);

		wcscpy(expectedResult, firstPartW);
		wcscat(expectedResult, secondPartW);

		iret = wcscmp(expectedResult, destination);
		if (iret != 0)
		{
			printf("ERROR - Failed C_AppendUtf8toWide\n");
		}
		else 
		{
			printf("Success - C_AppendUtf8toWide\n");
		}
	}
#endif


	// TOOLBOX_CalculateBroadcast test
#if 1
	temp[0] = '\0';
	iret = TOOLBOX_CalculateBroadcast("10.64.10.66", "255.255.0.0", temp, 256);
	if ( iret != 1 || strcmp(temp,"10.64.255.255")!=0 )
	{
		printf("ERROR - TOOLBOX_CalculateBroadcast failed with output(%s) and iret(%d)\n", temp, iret);
	}
	else
	{
		printf("Success - TOOLBOX_CalculateBroadcast output(%s)\n", temp);
	}
#endif


#if 1
	iret = TOOLBOX_GetDefaultGateway(temp, 256);
	printf("TOOLBOX_GetDefaultGateway returned iret(%d) with gateway(%s)\n", iret, temp);
	//return 0;
#endif

	// lineParser_extractDashedParameter test
#if 1
	iret = lineParser_extractDashedParameter(&parser, toExtractDoubleQuotes, "sdp_filename", &insider);
	if ( iret > 0 && insider )
	{	
		printf("SUCCESS - insider(%s)", insider);
	}
	else
	{
		printf("ERROR - Failed extractDashedParameter [line %d]", g_test_number);
	}
	//return 0;
#endif

#if 0
	charArray_Constructor(&entries,0);
	file_to_array("g:\\tmp\\configuration.output.003.txt", &entries);
	printf("%d", entries.row_amount);	
	charArray_Destructor(&entries);
#endif

	// C_strendstr test
#if 1
	strcpy(temp, "abcdefg.264");
	strRet = C_strendstr(temp,".264");
	if ( strRet == NULL )
	{
		printf("FAILED - C_strendstr returned NULL\n");
	}
	else
	{
		printf("SUCCESS - C_strendstr\n");
	}
	g_test_number++;
#endif

	// charArray_to_buffer
#if 1
	errors = 0;
	memset(bigbuf, 0, sizeof(bigbuf));
	charArray_Constructor(&entries, 1024);
	charArray_add(&entries, ".");
	charArray_add(&entries, "..");
	charArray_add(&entries, "VEncSomething1   : some file .txt");
	charArray_add(&entries, "VEncSomething2   : some file .txt");
	charArray_add(&entries, "VEncSomething3   : some file .txt");
	charArray_add(&entries, "VEncSomething4   : some file .txt");
	iret = charArray_to_buffer(&entries, bigbuf, sizeof(bigbuf));
	if ( iret > 0 )
	{
		strLen = strlen(bigbuf);
		if ( strLen != 141 )
		{
			errors++;
			printf("ERROR - charArray_to_buffer output has strLen(%d)\n", strLen);
		}
		else
		{
			printf("SUCCESS - charArray_to_buffer\n");
		}
	}
	else
	{
		errors++;
		printf("ERROR - charArray_to_buffer returned iret(%d)\n", iret);
	}

	charArray_Destructor(&entries);
	g_test_number++;
#endif
	
	// HTTP parsing algorithm test.
#if 0
	{
		char * strRet;
		char charBackup;
		int value_valid;
		strcpy(temp, "b%20x");
		value_valid = strlen(temp);

		if ( strRet = strstr(temp, "%") )
		{
			if ( strRet+2 <= temp+value_valid )
			{
				charBackup = *(strRet+3);
				*(strRet+3) = '\0';
				iret = TOOLBOX_axtoi(strRet+1);
				*strRet = iret;
				*(strRet+3) = charBackup;
				iret = temp-strRet;
				memmove(strRet+1, strRet+3, (value_valid+1)-(strRet-temp+2) ); // (buffer_size) - (consumed)
				printf(temp);
			}
		}
	}
#endif

#if 0	
	iret = changePassword(account_filename,"admin", NULL, password);

	return 0;
#endif

	// TOOLBOX_GetNetworkInformation test
#if 1
	{
		char ip[256];
		char mac[256];
		char subnet[256];
		char broadcast[256];
		char RX_packets[256];
		char RX_errors[256];
		char RX_bytes[256];
		char TX_packets[256];
		char TX_errors[256];
		char TX_bytes[256];
		char collisions[256];
		TOOLBOX_GetNetworkInformation(ip, subnet, broadcast, mac, RX_packets, RX_errors, RX_bytes, TX_packets, TX_errors, TX_bytes, collisions);
		printf("ip(%s)\n",ip);
		printf("subnet(%s)\n",subnet);
		printf("broadcast(%s)\n",broadcast);
		printf("mac(%s)\n",mac);
		printf("RX_packets(%s)\n",RX_packets);
		printf("RX_errors(%s)\n",RX_errors);
		printf("RX_bytes(%s)\n",RX_bytes);
		printf("TX_packets(%s)\n",TX_packets);
		printf("TX_errors(%s)\n",TX_errors);
		printf("TX_bytes(%s)\n",TX_bytes);
		printf("collisions(%s)\n",collisions);
	}
#endif

	// snprintf usage test against overflow.
#if 0
	snprintf(phrase, 10,"123812390123403214453253424342");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"6");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"7");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"8");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"9");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"0");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");

	strcpy(phrase,"");

	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"6");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"7");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"8");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"9");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"0");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"1");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"2");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"3");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"4");
	snprintf(phrase+strlen(phrase), 10-strlen(phrase)-1,"5");
#endif 

	// TOOLBOX_GetFileSize test
#if 0
	iret = TOOLBOX_GetFileSize("g:\\videoFiles\\CrazyBoss.mpg", &fileSize);
	printf("fileSize("I64u")\n", fileSize);
	//return 0;
#endif

	// TOOLBOX_isMulticast test
#if 1
	iret = TOOLBOX_isMulticast("24.1.1.3");
	if ( iret > 0 )
	{		
		printf("ERROR TOOLBOX_isMulticast returned iret(%d) on 24.1.1.3\n");
	}
	else
	{
		printf("SUCCESS TOOLBOX_isMulticast\n");
	}
	//return 0;
#endif

	TOOLBOX_Random_numeric(2, &bigbuf[0], 256);
	
	
	// lineParser base tests
	iret = lineParser_extractTag(&parser, string_with_tag1, "HASH", &tag_ptr);
	if ( iret <= 0 )
	{
		printf("extractTag returned iret(%d)\n",iret);
	}
	printf("tag_ptr(%s)\n", tag_ptr);

	//char string_with_tag1[1024]="frank::manager::HASH(21333213)::OBJ(12333)";
	//char string_with_tag2[]="::manager::HASH(21333213)::OBJ(12333)";
	//char string_with_tag3[]="::manager::HASH(21333213)::OBJ(12333)::";
	//char string_with_tag4[]="::manager";
	//char string_with_delimiters[]="frank::manager,:,HASH(21333213)::OBJ(12333)";

	printf("======test case 1======\n");
	iret = lineParser_splitTag(&parser, string_with_tag1, "::", &array_inside);
	if ( array_inside->row_amount != 4 )
	{	printf("ERROR lineParser_splitTag on string_with_tag1\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "frank")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag1\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag1\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 2======\n");
	iret = lineParser_splitTag(&parser, string_with_tag2, "::", &array_inside);	
	if ( array_inside->row_amount != 4 )
	{	printf("ERROR lineParser_splitTag on string_with_tag2\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag2\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag2\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 3======\n");
	iret = lineParser_splitTag(&parser, string_with_tag3, "::", &array_inside);
	if ( array_inside->row_amount != 5 )
	{	printf("ERROR lineParser_splitTag on string_with_tag3\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[3][0], "OBJ(12333)")!=0
		  || strcmp(array_inside->array[4][0], "")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag3\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag3\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 4======\n");
	iret = lineParser_splitTag(&parser, string_with_tag4, "::", &array_inside);
	if ( array_inside->row_amount != 2 )
	{	printf("ERROR lineParser_splitTag on string_with_tag4\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "")!=0 
		  || strcmp(array_inside->array[1][0], "manager")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag4\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag4\n");
		}
	}
	charArray_print(array_inside);

	printf("======test case 5======\n");
	iret = lineParser_splitTag(&parser, string_with_tag5, "::", &array_inside);
	if ( array_inside->row_amount != 1 )
	{	printf("ERROR lineParser_splitTag on string_with_tag5\n");
	}
	else
	{
		if ( strcmp(array_inside->array[0][0], "lonely")!=0 )
		{	printf("ERROR lineParser_splitTag on string_with_tag5\n");
		}
		else
		{	printf("Success - lineParser_splitTag on string_with_tag5\n");
		}
	}
	charArray_print(array_inside);

	iret = lineParser_replaceTag(&parser, string_with_tag1, "HASH", "13", &insider);
	if ( iret <= 0 )
	{
		printf("replaceTag returned iret(%d)\n",iret);
	}
	if ( insider )
	{
		printf("insider(%s)\n", insider);
	}
	else
	{
		printf("there is no insider from replaceTag\n");
	}


	// lineParser_extractFirstWord test
#if 0
	iret = lineParser_extractFirstWord(&parser, "COMMANDO alwoone2 233\n", &insider);

	if ( iret > 0 && insider )
	{
		printf("command(%s)\n",insider);
	}
	return 1;
#endif

	// file_to_variables benchmark
#if 0
	
	timeStart = TOOLBOX_Timestamp();
	configArray_Constructor(&HTML_Variables, 2000);
	file_to_variables("g:\\tmp\\content_dynamic.dat", &HTML_Variables);

	configArray_Destructor(&HTML_Variables);

	timeEnd = TOOLBOX_Timestamp();

	printf("HTML_Variables amount(%d)\n", HTML_Variables.row_amount );
	printf("done, elapsed(%u)\n", timeEnd-timeStart );

	TOOLBOX_Sleep(2000);

	goto end_of_self_tests;

#endif

	// lineParser_setGreediness test
#if 0
	strcpy(temp,"Fri Sep  8 13:23:42 UTC 2006");
	a = lineParser_setGreediness(1);
	iret = lineParser_splitTag(&parser, temp, " ", &array_insider);
	if ( iret > 0 && array_insider )
	{
		charArray_print(array_insider);
	}

	if ( iret && array_insider && array_insider->row_amount >= 6 )
	{
		snprintf(temp,255,"%s.%s.%s", array_insider->array[2][0], array_insider->array[1][0], array_insider->array[5][0]);
	}
	
	lineParser_setGreediness(a);

	goto end_of_self_tests;
#endif

	// lineParser_setGreediness test 
#if 0
	strcpy(temp,"Fri Sep  8 13:23:42 UTC 2006");
	a = lineParser_setGreediness(0);
	iret = lineParser_splitTag(&parser, temp, " ", &array_insider);
	if ( iret > 0 && array_insider )
	{
		charArray_print(array_insider);
	}

	if ( iret && array_insider && array_insider->row_amount >= 6 )
	{
		snprintf(temp,255,"%s.%s.%s", array_insider->array[2][0], array_insider->array[1][0], array_insider->array[5][0]);
	}
	
	lineParser_setGreediness(a);

	goto end_of_self_tests;
#endif

	// lineParser_replaceAll
#if 0
	iret = lineParser_replaceAll(&parser, toBeEscaped, "\"","\\\"", &insider);
	printf("iret(%d) output(%s)\n",iret,insider);
	
	strcpy(toBeEscaped, insider);
	
	iret = lineParser_replaceAll(&parser, toBeEscaped, "'", "\\'", &insider);
	printf("iret(%d) output(%s)\n",iret,insider);				

	goto end_of_self_tests;

#endif



	// MD5 library usage
	{
		our_MD5Init(&ctx);
		our_MD5Update(&ctx, password, strlen(password) );
		our_MD5Final(digest, &ctx);

		for (i=0; i<32; i++)
		{
			buffer[i*2] = hex[digest[i]>>4];
			buffer[i*2+1] = hex[digest[i]&0x0f];
		}
		buffer[32] = '\0';

		printf("digest(%s)\n",buffer);
	}

	// HTTP_Request test
#if 0
	Network_Initialize();
	
	strcpy(document,"/index.html");
	strcpy(document,"/index.cgi");
	//strcpy(document,"/cgi-bin/tasman.cgi");
	//snprintf(content_string, 255, "nickname=&login=admin&password=manager&action_login=Login");
	snprintf(content_string, 255, "username=admin&password=manager");
	iret = HTTP_Request("10.64.1.141", document, content_string, NULL, GET, &insider);
	//iret = HTTP_Request("10.64.10.66", document, content_string, GET, &insider);
	cookie = NULL;
	if ( iret > 0 && insider )
	{
		textBufferReader reader;
		TextBufferReader_Constructor(&reader, insider, strlen(insider)+1);
			
		iret = TextBufferReader_GetLine(&reader, temp, 255);
		while ( iret > 0 )
		{
			if ( TOOLBOX_strcasestr(temp, "Set-Cookie")
				&& strlen(temp)>12 )
			{
				strcpy(buffer, temp+12);
				cookie = buffer;
				break;
			}			
			iret = TextBufferReader_GetLine(&reader, temp, 255);
		}
		TextBufferReader_Destructor(&reader);
		fputs(insider, stdout);
	}

	strcpy(document,"/decoder.cgi");
	snprintf(content_string, 255, "page=stream&apply=1&streamtype=multicast&streamaddress=228.1.1.44&streamport=4568");
	iret = HTTP_Request("10.64.1.141", document, content_string, cookie, GET, &insider);
	if ( iret > 0 && insider )
	{
		fputs(insider, stdout);
	}

	charArray_Constructor(&html, 100);
	charArray_add(&html, "view=audiovideo");
	charArray_add(&html, "VEncBitrate=2424");
	charArray_add(&html, "action=write_these_config");
	charArray_splitTag(&html, "=");
	//HTTP_Request("10.64.1.191","/main.php", &html, POST);
	iret = HTTP_Request_with_charArray("10.64.2.19","/cgi-bin/tasman.cgi", &html, NULL, POST, &insider);
	charArray_Destructor(&html);
#endif

	
#if 0
	a=0;
	
	charArray_Constructor(&entries, 1024);
	file_to_array("g:\\tmp\\sessions.txt", &entries);

#	if 0
	for (i=0; i<99; i++)
	{
		//iret = charArray_removeLineThatMatchesEx(&entries, "Session ID", a, &a);
		iret = charArray_removeLineThatMatchesEx(&entries, "Activation mode", a, &a);
		iret = charArray_removeLineThatMatches(&entries, "Transfer Type");
		iret = charArray_removeLineThatMatches(&entries, "Card ID");
		iret = charArray_removeLineThatMatches(&entries, "Port ID");
		iret = charArray_removeLineThatMatches(&entries, "Remote Host name");
		iret = charArray_removeLineThatMatches(&entries, "Local Host name");
		iret = charArray_removeLineThatMatches(&entries, "Max TS in Packet");
		iret = charArray_removeLineThatMatches(&entries, "Reordering Time");
		iret = charArray_removeLineThatMatchesEx(&entries, "SSRC", a, &a);
		if ( iret <= 0 )
		{
			break;
		}
	}
#	endif
	charArray_to_file(&entries, "g:\\tmp\\outsessions.txt");
	charArray_Destructor(&entries);

	goto end_of_self_tests;
#endif

#if 0 // test lineParser_extractInsider
	lineParser_extractInsider(&parser, preset_filename, "configuration.audiovideo.", ".txt", &insider);
	printf("insider(%s)\n", insider);
	return 0;
#endif


#if 0 // test lineParser_stripSpacing
	lineParser_extractInsider(&parser, "$ something_is_equal_to = <<<EOF", "$", "=", &insider);
	strcpy(insider, "  ");
	lineParser_strpSpacing(&parser, insider, 1, 1, &insider);
	printf("insider(%s)\n", insider);
	return 0;
#endif

#if 0 // test lineParser_stripSpacing
	lineParser_splitTag(&parser, "10:54:14 up  1:29, load average: 0.74, 0.33, 0.17", " ", &array_insider);
	charArray_print(array_insider);

	lineParser_splitTag(&parser, "Tue Jul 4 11:12:22 UTC 2006", " ", &array_insider);
	charArray_print(array_insider);

	goto end_of_self_tests;
#endif


	// charArray test 01 BEGINs
	charArray_Constructor(&html, 100);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_insert(&html, 2, "new insertion");
	charArray_Zero(&html);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_insert(&html, 2, "new insertion");/**/
	charArray_Destructor(&html);
	// charArray test 01 ENDs

	// charArray test 02 BEGINs
	charArray_Constructor(&html, 100);
	charArray_Constructor(&entries, 100);
	charArray_add(&html, ".");
	charArray_add(&html, "line two with REPLACE_TAG asdl oo hihihi");
	charArray_add(&html, "..");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_add(&entries, "VERY LONG DUAL REPLACEMENT");
	charArray_substituteArrayInLineThatMatches(&html, "REPLACE_TAG", &entries);
	charArray_Destructor(&entries);
	charArray_Destructor(&html);
	// charArray test 02 ENDs

	// charArray test 03 BEGINs
	charArray_Constructor(&entries, 1024);
	charArray_add(&entries, ".");
	charArray_add(&entries, "..");
	charArray_add(&entries, "VEncSomething1   : some file .txt");
	charArray_add(&entries, "VEncSomething2   : some file .txt");
	charArray_add(&entries, "VEncSomething3   : some file .txt");
	charArray_add(&entries, "VEncSomething4   : some file .txt");

#if 0 // test realloc
	//g_c_toolbox_verbose=1;
	for (i=0; i<2000; i++)
	{
		charArray_add(&entries, text_buffer);
	}

	iret = charArray_substituteArrayInLineThatMatches(&html, "REPLACE_TAG", &entries);

	charArray_to_buffer(&entries, bigbuf, 25000);
#endif

	iret = charArray_getLineThatMatches(&entries, "Video Select|VEncSomething", &insider);
	if ( iret > 0 && insider )
	{
		printf("insider(%s)\n",insider);
	}

	printf("=========before=======\n");
	charArray_cout(&entries);

	printf("=========afterremove=======\n");

	//charArray_removeLineWithIndex(&entries,0);
	charArray_removeLineWithIndexAndAllBefore(&entries,3);

	charArray_cout(&entries);

	printf("=========end=======\n");

	iret = TextFileReader_Constructor(&reader, "/home/flemieux/TOOLBOX_ToolBox.SVN/hostname_list.txt");
	//iret = TextFileReader_Constructor(&reader, "C:\\Tasman\\hostname_list.txt");
	printf("TextFileReader_Constructor iret(%d)\n", iret);

	//iret = TextFileReader_GetLine(&reader, buffer, 1024);
	//printf("TextFileReader_Constructor iret(%d), buffer(%s)\n", iret, buffer);

	putTextFileReader_into_charArray(&reader, &entries);

	iret = TextFileReader_Destructor(&reader);
	printf("TextFileReader_Destructor iret(%d)\n", iret);
	charArray_Destructor(&entries);	
	// charArray test 03 ENDs
	
#if 1
	{
		uint64_t last_member;
		msString_T * string;
		binaryTree_T binaryTree;
		bItem_T * bItem;
		bItem_T * bIterator = NULL;

		binaryTree_Constructor(&binaryTree);
		binaryTree.accept_duplicate_names = 1;
		binaryTree.accept_duplicate_unique_ids = 1;

		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemA", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}
	
		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemC", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}

		for (i=0; i<20; i++)
		{
			string = (msString_T*)malloc(sizeof(msString_T));
			memset(string, 0, sizeof(msString_T));
			C_Append(&string->buffer, &string->buffersize, "itemB", -1, NULL);
			iret = binaryTree_Insert(&binaryTree, string->buffer, 0, string, &bItem);
			if ( iret <= 0 )
			{
				printf("ERROR - binaryTree_Insert returned iret(%d) on string(%s)\n", iret, string->buffer);
			}
		}

		i = 0;
		string = NULL;
		iret = binaryTree_FirstItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			i++;

			string = NULL;
			iret = binaryTree_NextItem(&binaryTree, &string, &bItem, &bIterator);
		}
		
		i = 0;
		string = NULL;
		iret = binaryTree_FirstItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			if ( i == 25 )
			{
				break;
			}
			
			i++;

			string = NULL;
			iret = binaryTree_NextItem(&binaryTree, &string, &bItem, &bIterator);
		}

		string = (msString_T*) bItem->client;
		last_member = bItem->unique_id;
		iret = binaryTree_Delete(&binaryTree, bItem);

		iret = binaryTree_Insert(&binaryTree, "dude", last_member, string, &bItem);

		last_member = 0;
		i = 0;
		string = NULL;
		iret = binaryTree_FirstiItem(&binaryTree, &string, &bItem, &bIterator);
		while ( iret > 0 && string )
		{
			printf("item:%s\n", string->buffer);

			if ( last_member > bItem->unique_id )
			{
				printf("ERROR - binaryTree got corrupted\n");
				__asm int 3;
				break;
			}
			last_member = bItem->unique_id;

			string = NULL;
			iret = binaryTree_NextiItem(&binaryTree, &string, &bItem, &bIterator);
		}

		binaryTree_Destructor(&binaryTree);
	}
#endif

	iret = charArray_getLineThatMatches(&entries, "machine_ip::10.64.1.131", &out_ptr);
	if ( iret && out_ptr )
	{
		lineParser_splitTag(&parser, out_ptr, ",:,", &splitted);
		printf("======splitted======\n");
		charArray_print(splitted);
		printf("====================\n");
	}

	charArray_splitTag(splitted, "::");

	printf("======re-splitted======\n");
	charArray_print(splitted);
	printf("====================\n");

	charArray_qsort(splitted, 1, 0, 0, -2);
	printf("======sorted======\n");
	charArray_print(splitted);
	printf("====================\n");

	{
		TextBufferReader_ConstructorEx(&breader, text_buffer, strlen(text_buffer), 1);

		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);
		TextBufferReader_GetLine(&breader, buffer, 1023);

		TextBufferReader_Destructor(&breader);
	}

	{
		TextBufferReader_ConstructorExW(&breaderW, text_bufferW, wcslen(text_bufferW), 1);

		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);
		TextBufferReader_GetLineW(&breaderW, bufferW, 1023);

		TextBufferReader_DestructorW(&breaderW);
	}

	// C XML
#if 1
	if (0) 
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_test_alternate_text_tag.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "p", &element, &iterator);
			if ( iret > 0 && element )
			{
				// WARNING incomplete test
				iret = xml_getFirstElement(element->childs, "text", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
	if (0)
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_from_odt_01.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "p", &element, &iterator);
			if ( iret > 0 && element )
			{
				iret = xml_getFirstElement(element->childs, "text", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
	if (1)
	{
		xmlParser_T xml;
		xmlElement_T * element = NULL;
		xmlElement_T * element2 = NULL;
		xmlIterator_T iterator;
		int success = 0;
		iret = xml_Constructor_fromFile(&xml, "regression_test_data\\xml_from_odt_02.txt");
		if ( iret > 0 )
		{
			iret = xml_getFirstElement(&xml.elements, "text:p", &element, &iterator);
			if ( iret > 0 && element )
			{
				iret = xml_getFirstElement(element->childs, "text:span", &element2, &iterator);
				if ( iret > 0 && element2 )
				{
					if ( element2->value && strcmp(element2->value, "hello how are you  cool") == 0 )
					{
						// ok
						printf("SUCCESS - XML parsing of alternate text n' tags\n");
						success = 1;
					}
				}
			}
		}
		if ( success == 0 )
		{
			printf("ERROR - XML parsing of alternate text n' tags\n");
		}
		xml_Destructor(&xml);
	}
#endif


#if 1 // linkedList TESTs
	{
		linkedList myList;
		simpleItem_T simpleItem[200];
		simpleItem_T * curItem;
		unsigned int lastItem = 0;

		memset(&simpleItem, 0, sizeof(simpleItem));

		linkedList_Constructor(&myList);
		
		for(i=0; i<10; i++)
		{
			simpleItem[i].number = 10-i;
			linkedList_Push(&myList, 10-i); //warning ok
		}
		linkedList_qsort(&myList, linkedlist_self_test_sort_callback_01, 0, 1, -2);

		i = 0;
		errors = 0;
		curItem = NULL;
		iret = linkedList_FirstItem(&myList, &curItem);
		while ( iret > 0 )
		{
			//printf("curItem(%d) = %d\n", i, (unsigned int)curItem);

			if ( lastItem > (unsigned int)curItem )
			{
				printf("ERROR - linkedList, first test sorting assending\n");
				errors++;
				break;
			}
			lastItem = (unsigned int)curItem;
			curItem = NULL;
			iret = linkedList_NextItem(&myList, &curItem);
			i++;
		}
		if ( myList.itemAmount != 10 )
		{	
			printf("ERROR - linkedList, first test has items missing after sort\n");			
		}
		else if ( errors == 0 )
		{
			printf("Success - linkedList, first test\n");
		}

		linkedList_Destructor(&myList);
		linkedList_Constructor(&myList);

		// linkedList TEST 2
		for(i=0; i<200; i++)
		{
			simpleItem[i].number = 200-i;
			linkedList_Push(&myList, &simpleItem[i]);
		}
		linkedList_qsort(&myList, linkedlist_self_test_sort_callback, 0, 1, -2);

		i = 0;
		errors = 0;
		lastItem = 0;
		curItem = NULL;
		iret = linkedList_FirstItem(&myList, &curItem);
		while ( iret > 0 && curItem )
		{
			//printf("curItem(%d) = %d\n", i, curItem->number);
			if ( lastItem >= curItem->number )
			{
				printf("ERROR - linkedList, second test sorting assending\n");
				errors++;
				break;
			}
			lastItem = curItem->number;
			curItem = NULL;
			iret = linkedList_NextItem(&myList, &curItem);
			i++;
		}
		if ( myList.itemAmount != 200 )
		{	
			printf("ERROR - linkedList, second test has items missing after sort\n");			
		}
		else if ( errors == 0 )
		{
			printf("Success - linkedList, second test\n");
		}

		linkedList_Destructor(&myList);
	}
#endif // linkedList TESTs
	
	
end_of_self_tests:
	iret = lineParser_Destructor(&parser);

	printf("finished.\n");
	printf("</pre>\n");

	return 0;

	TOOLBOX_Logger("(startup.c) main(), Starting up.\n");
	TOOLBOX_Logger("   version 2.340, timeout 1.5 sec, retry_max 3\n");

#ifdef TOOLBOX_LOGGER
	if ( g_toolbox_log > 0 )
	{
		fclose(g_toolbox_log);
		g_toolbox_log=NULL;
	}
#endif

	return 0;
}



int test_toolbox_system_file_reader(const char * filename, char * loadme, unsigned int loadme_size)
{
	int fret = 1;
	int size_read = 0;
	unsigned int result_written = 0;
	unsigned int read_size = 24;
	FILE * file;

	file = fopen(filename, "r");

	if ( file == NULL )
	{
		return -13;
	}

	if ( loadme_size-1-result_written < read_size )
	{	read_size = loadme_size-1-result_written;
	}
	size_read = fread(loadme+result_written, 1, read_size, file);
	//printf("after initial fgets, charRet(0x%X) and feof=%d\n", (unsigned int)charRet, feof(outf));
	while ( size_read > 0 )
	{
		result_written += size_read;
		*(loadme+result_written) = '\0';

		if ( loadme_size-1-result_written < read_size )
		{	read_size = loadme_size-1-result_written;
		}

		if ( read_size == 0 )
		{
			fret=-14; // buffer is too small
			break;
		}
		
		size_read = fread(loadme+result_written, 1, read_size, file);	
	}
	fclose(file);

	return fret;
}



#endif// C_TOOLBOX_STANDALONE

