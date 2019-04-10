#pragma once

#if defined(_MSC_VER)
#	define baseDirectory "g:\\temp"
#	define configDirectory baseDirectory "\\config"
#	define directorySeparatorString "\\"
#	define directorySeparatorChar '\\'
#else
#	define configDirectory "/usr/share/config"
#	define directorySeparatorString "/"
#	define directorySeparatorChar '/'
#endif

#include "sessmgr_types.h"
#include "dbgtrace.h"
extern HDBG g_hDbg;

#define HAIVISION_MAX_ONVIF_TOKEN 128
#define HAIVISION_MAX_ONVIF_URL 1024
#define HAIVISION_MAX_ONVIF_ERR_STRING 2000
#define HAIVISION_MAX_ONVIF_IPV4_LENGTH 80

extern char g_public_ip[HAIVISION_MAX_ONVIF_IPV4_LENGTH];

void* hai_soap_malloc(struct soap* soap, size_t size);

const char * hai_get_card_type();
const char * hai_get_system_serial();
const char * hai_get_system_firmware();
const char * hai_get_hardware_id();

int HAIOS_getHostVersion(char ** build_version_insider, char ** build_date_insider, unsigned int * special_flags);
int HAIOS_getSystemVersion(char ** package_version_insider);
unsigned int getHardwareInfo();