#ifndef __GNUC_CONFIG_INI_MANAGER_H__
#define __GNUC_CONFIG_INI_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "toolbox-text-file-reader.h"
#include "toolbox-linkedlist.h"
#include "toolbox-mutex.h"

#define GNUC_CONFIG_ITEM_KEY_SIZE 64
#define GNUC_CONFIG_ITEM_VALUE_SIZE 256
#define GNUC_CONFIG_FILENAME_SIZE 1024

// public structures
typedef struct _ConfigIniManager
{
	int instance_number;
	FILE * config_file;
	textFileReader text_file_reader;
	char filename[GNUC_CONFIG_FILENAME_SIZE];
	linkedList list;
	CMutex mutex;
	int file_not_found;

} ConfigIniManager;
	
int ConfigIniManager_Constructor(ConfigIniManager * root, char * filename);
int ConfigIniManager_Destructor(ConfigIniManager * root);
int ConfigIniManager_WriteConfig(ConfigIniManager * root);
int ConfigIniManager_GetChar(ConfigIniManager * root, char * out_value, char * in_key);
int ConfigIniManager_SetChar(ConfigIniManager * root, const char * const in_value, char * in_key);
int ConfigIniManager_GetInteger(ConfigIniManager * root, int * out_value, char * in_key);
int ConfigIniManager_SetInteger(ConfigIniManager * root, int in_value, char * in_key);
int ConfigIniManager_SetInstanceNumber(ConfigIniManager * root, int instance);


#ifdef __cplusplus
}
#endif

#endif //__GNUC_CONFIG_INI_MANAGER_H__
