//  ~~~~~~~~~~~~~~ GNUC Toolbox ~~~~~~~~~~~~~~
//    portable data manipulation functions
//    portable socket server functions
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// config_ini_manager.c
// 
// Copyright (c) 2006 HaiVision System Inc.	
//				
// Maintained by : Francois Oligny-Lemieux
//       Created : 24.Nov.2006 (based on my C++ version)
//      Modified : 27.Nov.2006
//
//  Description: 
//      portable config.ini file manager
//
//  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "toolbox-config.h"

#if C_TOOLBOX_CONFIG_INI_MANAGER == 1
#	include "toolbox.h"
#	include "toolbox-text-file-reader.h"

#	include <limits.h>
#	include <stdio.h>
//#	include <stdlib.h>


typedef struct config_item_tag
{
	char key[GNUC_CONFIG_ITEM_KEY_SIZE];
	char value[GNUC_CONFIG_ITEM_VALUE_SIZE];

} config_item;


int ConfigIniManager_GetInteger(ConfigIniManager * root, int * out_value, char * in_key)
{
	int iret;
	int fret=-1; 
	char value[GNUC_CONFIG_ITEM_VALUE_SIZE];
	config_item * item_ptr = NULL;
    
	if ( root == NULL )
	{
		return -1;
	}
	if ( out_value == NULL )
	{
		return -2;
	}
	if ( in_key == NULL )
	{
		return -3;
	}

	CMutex_Acquire(&root->mutex);

	iret = linkedList_FirstItem(&root->list, (void*) &item_ptr);
	//Item<config_item*> * current = m_list.CurrentItem();

	while ( iret > 0 && item_ptr )
	{
		if (  strcmp(item_ptr->key,in_key) == 0  )
		{
			// match found
			strcpy(value, item_ptr->value );
			*out_value = atoi(value);
			fret=1;
			break;
		}		
		item_ptr = NULL;
		iret = linkedList_NextItem(&root->list, (void*) &item_ptr);
	}

	if ( fret == -1 && root->file_not_found )
	{
		fret = -10;
	}

	CMutex_Release(&root->mutex);

	return fret;
}


int ConfigIniManager_GetChar(ConfigIniManager * root, char * out_value, char * in_key)
{
	int iret;
	int fret=-1; 
	char value[GNUC_CONFIG_ITEM_VALUE_SIZE];
	linkedList_item * item = NULL;
	config_item * item_ptr = NULL;    

	if ( root == NULL )
	{
		return -1;
	}
	if ( out_value == NULL )
	{
		return -2;
	}
	if ( in_key == NULL )
	{
		return -3;
	}

	CMutex_Acquire(&root->mutex);

	iret = linkedList_FirstItem(&root->list, (void*) &item_ptr);

	while ( iret > 0 && item_ptr )
	{
		if (  strcmp( item_ptr->key, in_key ) == 0  )
		{
			// match found
			strcpy(value, item_ptr->value );
			strcpy(out_value, value);
			fret=1;
			break;
		}
		item_ptr = NULL;
		iret = linkedList_NextItem(&root->list, (void*) &item_ptr);
	}
	
	if ( fret == -1 && root->file_not_found )
	{
		fret = -10;
	}

	CMutex_Release(&root->mutex);

	return fret;
}

int ConfigIniManager_SetInteger(ConfigIniManager * root, int in_value, char * in_key)
{
	int iret;
	int fret=-1; 
	char value[GNUC_CONFIG_ITEM_VALUE_SIZE];	
	config_item * new_item = NULL;
	int found_entry = 0;    
	config_item * item_ptr = NULL;
		
	if ( root == NULL )
	{
		return -1;
	}

	CMutex_Acquire(&root->mutex);
	
#if defined (_MSC_VER)
	C_itoa(in_value, &value[0], 10);
#else
	C_itoa(in_value, &value[0], 10);
#endif
	
	iret = linkedList_FirstItem(&root->list, (void*) &item_ptr);

	while ( iret > 0 && item_ptr )
	{
		if (  strcmp( item_ptr->key, in_key ) == 0  )
		{
			// match found
			found_entry = 1;
			strcpy( item_ptr->value, value );
			fret=1;
			break;
		}
		item_ptr = NULL;
		iret = linkedList_NextItem(&root->list, (void*) &item_ptr);
	}

	if ( found_entry == 0 )
	{
		new_item = (config_item*) malloc(sizeof(config_item));
		strcpy(new_item->key, in_key);
		strcpy(new_item->value, value);
		linkedList_Push(&root->list, new_item);
	}

	CMutex_Release(&root->mutex);

	return fret;

}

int ConfigIniManager_SetChar(ConfigIniManager * root, const char * const in_value, char * in_key)
{	
	int iret;
	int fret=-1; 
	config_item * new_item = NULL;
	int found_entry = 0;    
	config_item * item_ptr = NULL;
		
	if ( root == NULL )
	{
		return -1;
	}
	if ( in_value == NULL )
	{
		return -2;
	}

	CMutex_Acquire(&root->mutex);
	
	iret = linkedList_FirstItem(&root->list, (void*) &item_ptr);

	while ( iret > 0 && item_ptr )
	{
		if (  strcmp( item_ptr->key, in_key ) == 0  )
		{
			// match found
			found_entry = 1;
			strcpy( item_ptr->value, in_value );
			fret=1;
			break;
		}
		item_ptr = NULL;
		iret = linkedList_NextItem(&root->list, (void*) &item_ptr);
	}

	if ( found_entry == 0 )
	{
		new_item = (config_item*) malloc(sizeof(config_item));
		strcpy(new_item->key, in_key);
		strcpy(new_item->value, in_value);
		linkedList_Push(&root->list, new_item);
	}

	CMutex_Release(&root->mutex);

	return fret;
}


int ConfigIniManager_WriteConfig(ConfigIniManager * root)
{
	int iret;
	char temp[1024];
	int fret=1;
	FILE * file;
	config_item * item_ptr = NULL;
		
	if ( root == NULL )
	{
		return -1;
	}

	CMutex_Acquire(&root->mutex);

	TextFileReader_Destructor(&root->text_file_reader);

	file = fopen(root->filename, "wb");
	if ( file == NULL )
	{
		fret = -10;
		goto end_write_config;
	}

	iret = linkedList_FirstItem(&root->list, (void*) &item_ptr);

	while ( iret > 0 && item_ptr )
	{
        snprintf(temp, 1023, "%s=%s\r\n", item_ptr->key, item_ptr->value );
        fwrite(temp, 1, strlen(temp), file);
		iret = linkedList_NextItem(&root->list, (void*) &item_ptr);
	}

	fclose(file);

end_write_config:;
	
	CMutex_Release(&root->mutex);

	return fret;
}


//constructor
int ConfigIniManager_Constructor(ConfigIniManager * root, char * filename)
{
	char buffer[1024];
	char * const buffer_ptr = &buffer[0];
	char * const buffer_ptr_end = &buffer[1023];
	char * buffer_ptr_temp = &buffer[0];
	char * str_ret;
	unsigned int buffersize = 1024;
	int iret = 0;
	int length;
	config_item * new_item = NULL;
	int found;
		
	if ( root == NULL )
	{
		return -1;
	}
	
	CMutex_Acquire(&root->mutex);
	
	root->file_not_found = 0;

	linkedList_Constructor(&root->list);

	memset(&root->text_file_reader, 0, sizeof(textFileReader) );

	root->filename[0] = '\0';
	root->filename[GNUC_CONFIG_FILENAME_SIZE-1] = '\0';

	if ( filename )
	{
		if ( strlen(filename) < GNUC_CONFIG_FILENAME_SIZE )
		{
            strcpy(root->filename, filename);
		}
		else
		{
			printf("GNUC [ConfigIniManager] filename strlen is bigger than buffer size (%d)\n", GNUC_CONFIG_FILENAME_SIZE-1);
			goto end_of_func;
		}
	}

	iret = TextFileReader_Constructor(&root->text_file_reader, filename);
	if ( iret <= 0 )
	{
		// means file not found
		root->file_not_found = 1;
	}
	
	iret = TextFileReader_GetLine(&root->text_file_reader, &buffer_ptr, &buffersize, NULL, 0);
	
	while ( iret > 0 )
	{
		str_ret = strchr(buffer_ptr, '=');

		if ( str_ret == NULL )	
		{
			str_ret = strchr(buffer_ptr, ' ');
		}
		if ( str_ret == NULL )
		{
			str_ret = strchr(buffer_ptr, '\t');
		}

		if ( str_ret )
		{
			new_item = (config_item*) malloc(sizeof(config_item));
			// we got a delimiter
			length = str_ret - buffer_ptr;
			strncpy(new_item->key, buffer_ptr, length);
			new_item->key[length] = '\0';
		}
		else
		{
			goto continue_read_line;
		}

		buffer_ptr_temp = str_ret;
		buffer_ptr_temp++;
		if ( buffer_ptr_temp >= buffer_ptr_end )
		{
			free(new_item);
			goto continue_read_line;
		}

		found=1;
continue_through_space:;	
		if ( *buffer_ptr_temp == ' ' || *buffer_ptr_temp == '\t' )
		{
			buffer_ptr_temp++;
			if ( buffer_ptr_temp < buffer_ptr_end )
			{
				goto continue_through_space;
			}
			else
			{
				found = 0;
			}
		}

		if ( found )
		{
			strcpy(new_item->value, buffer_ptr_temp);
			linkedList_Push(&root->list, new_item);
		}
		else
		{
			free(new_item);
			new_item = NULL;
		}

continue_read_line:;
		iret = TextFileReader_GetLine(&root->text_file_reader, &buffer_ptr, &buffersize, NULL, 0);
	}
	
end_of_func:
	CMutex_Release(&root->mutex);

	return 1;

}

int ConfigIniManager_Destructor(ConfigIniManager * root)
{	
	if ( root == NULL )
	{
		return -1;
	}
	CMutex_Acquire(&root->mutex);

	TextFileReader_Destructor(&root->text_file_reader);

	CMutex_Release(&root->mutex);

	return 1;
}



int ConfigIniManager_SetInstanceNumber(ConfigIniManager * root, int instance)
{
	if ( root == NULL )
	{
		return -1;
	}

	root->instance_number = instance;

	return 1;
}




#endif // GNUC_TOOLBOX_CONFIG_INI_MANAGER == 1
