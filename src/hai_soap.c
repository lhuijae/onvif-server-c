

#include <stdio.h>
#include "mxtools.h" // MAKITO API
#include "sessmgr_api.h"
#include "hailogin.h"

#include "soapStub.h"
#include "soapH.h"
#include "toolbox.h"

static int g_static_hostVersion = -1;
static char g_static_hostBuildVersion[64] = "";
static char g_static_hostBuildDate[64] = "";
static char g_static_systemVersion[64] = "";
int g_api_inited = 0;
charArray array_result_message;

enum haiOS_flags {
	HAIOS_FLAG_SDATA = 2, 
	HAIOS_FLAG_SDATA_COT = 4 
};


#if defined(TARGET_ARM7)
SYSTEM_INFO haiversion;
CARD_INFO cardInfo;
HaiLogin_Cred g_user;

int hai_session_mngr_init()
{
   char temp[256];
   char worker[256];
   printf("hai_session_mngr_init\n");
   int ival = SessMgrLib_InitEx(SMGR_API_VERSION, NULL);
   if ( ival )
   {
     //DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "SessMgrLib_InitEx OK.\n");
     //DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Setting smgr lib UID to %d.\n", g_user.uid);
     ival = SessMgrLib_SetUID(g_user.uid);
     if( ival )
     {
        haiversion.nSize = sizeof(haiversion);
        if(GetSystemInfo(&haiversion))
        {
           cardInfo.nSize = sizeof(cardInfo);
           if(GetCardInfo(&cardInfo))
           {
              g_api_inited = 1;
              printf("hai_session_mngr_init success\n");
              printf("serial1: %s\n", haiversion.szSerialNumber);
              printf("firmware %s\n", haiversion.szFirmwareVersion);
           }
        }
     }
     else
     {
        //DBGPRINTEX(g_hDbg, DBG_LEVEL_ERROR, "SessMgrLib_SetUID failed.\n");
        printf("SessMgrLib_SetUID failed.\n");
     }
  }
  else
  {
     snprintf(worker, sizeof(worker), "Error: %s", SessMgr_GetLastErrorString(temp, sizeof(temp)));
     charArray_add(&array_result_message, worker);
     //DBGPRINTEX(g_hDbg, DBG_LEVEL_ERROR, "%s", temp);
     printf("hai_session_mngr_init error %s\n");
  }
}


const char * hai_get_card_type()
{
   return haiversion.szCardType;
}

const char * hai_get_system_serial()
{
   return haiversion.szSerialNumber;
}

const char * hai_get_system_firmware()
{
   return haiversion.szFirmwareVersion;
}

const char * hai_get_hardware_id()
{
   return haiversion.szHardwareRevision;
}

#else

int hai_session_mngr_init()
{
}

const char * hai_get_card_type()
{
   return "MXC";
}

const char * hai_get_system_serial()
{
   return "00000002";
}

const char * hai_get_system_firmware()
{
   return "1.0";
}

const char * hai_get_hardware_id()
{
   return "A";
}


#endif

void* hai_soap_malloc(struct soap* soap, size_t size)
{
    void * myptr = soap_malloc(soap, size);
    if (myptr == NULL)
    {       
       printf("soap_malloc error");
       return NULL;
    }
    memset(myptr, 0, size);
    return myptr;
}


int HAIOS_getHostVersion(char ** build_version_insider, char ** build_date_insider, unsigned int * special_flags)
{
   char * insider = NULL;
   charArray array;
   charArray * entry = NULL;
   lineParser parser;
   int ival, status;
   int hostVersion=2;
   char temp[256]="";
   char * str_ret;

   charArray_Constructor(&array, 0);
   lineParser_Constructor(&parser);

#if defined(_MSC_VER)
   if ( special_flags ) *special_flags = HAIOS_FLAG_SDATA;
#else
   if ( special_flags ) *special_flags = 0;
#endif

   if ( g_static_hostVersion == -1 )
   {
      ival = C_System("cat /proc/cpuinfo", &insider, &status);
      if ( ival > 0 && insider )
      {
         printf("insider: %s\n", insider);
         buffer_to_array(insider,&array);
         ival = charArray_getLineThatMatches(&array,"machine", &insider);
         if ( ival > 0 && insider )
         {
            ival = lineParser_splitTag(&parser, insider, ": ", &entry);
            if ( ival > 0 && entry && entry->row_amount >= 2 )
            {
               if (  strstr(entry->array[1][0],"MXHOST2")!=0  )
               {
                  hostVersion = 2;
               }
               else if (  strstr(entry->array[1][0],"MXHOST3")!=0  )
               {
                  hostVersion = 3;
               }
               else if (  strstr(entry->array[1][0],"MXHOST4")!=0  )
               {
                  hostVersion = 4;
               }
               else if (  strstr(entry->array[1][0],"MXHOST5")!=0  )
               {
                  hostVersion = 5;
               }
               else if (  strstr(entry->array[1][0],"MXHOST6")!=0  )
               {
                  hostVersion = 6;
               }
               else if (  strstr(entry->array[1][0],"MXHOST7")!=0  )
               {
                  hostVersion = 7;
               }
               else if (  strstr(entry->array[1][0],"MXHOST8")!=0  )
               {
                  hostVersion = 8;
               }
               else if (  strstr(entry->array[1][0],"MXHOST9")!=0  )
               {
                  hostVersion = 9;
               }
               else if (  strstr(entry->array[1][0],"HOST2")!=0  )
               {
                  hostVersion = 2;
               }
               else if (  strstr(entry->array[1][0],"HOST3")!=0  )
               {
                  hostVersion = 3;
               }
               else if (  strstr(entry->array[1][0],"HOST4")!=0  )
               {
                  hostVersion = 4;
               }
               else if (  strstr(entry->array[1][0],"HOST5")!=0  )
               {
                  hostVersion = 5;
               }
               else if (  strstr(entry->array[1][0],"HOST6")!=0  )
               {
                  hostVersion = 6;
               }
               else if (  strstr(entry->array[1][0],"HOST7")!=0  )
               {
                  hostVersion = 7;
               }
               else if (  strstr(entry->array[1][0],"HOST8")!=0  )
               {
                  hostVersion = 8;
               }
               else if (  strstr(entry->array[1][0],"HOST9")!=0  )
               {
                  hostVersion = 9;
               }
               //printf("decctrl->GopResync(%d)\n",decctrl->GopResync);
               g_static_hostVersion = hostVersion;
            }
         }
      }
   }
   else
   {
      hostVersion = g_static_hostVersion;
   }

   if ( g_static_hostBuildVersion[0] == '\0' )
   {
      ival = C_System("cat /proc/version", &insider, &status);
      if ( ival > 0 && insider )
      {
         buffer_to_array(insider,&array);
         ival = charArray_getLineThatMatches(&array,"Linux version ", &insider);
         if ( ival > 0 && insider )
         {
            str_ret = strstr(insider, "#");
            if ( str_ret )
            {
               strncpy(temp, str_ret, 255);
               temp[255]='\0';
               //printf("temp(%s)\n",temp );
               ival = lineParser_splitTag(&parser, temp, " ", &entry);
               if ( ival > 0 && entry && entry->row_amount >= 7 )
               {
                  if ( atoi(entry->array[6][0]) >= 2006 )
                  {
                     // build number == [0]
                     // day name == [1]
                     // month name == [2]
                     // day number == [3]
                     // hour number == [4]
                     str_ret = strstr(entry->array[4][0],":");
                     if ( str_ret )
                     {
                        str_ret[0] = 'h';
                     }

                     if ( strlen(entry->array[3][0]) == 1 )
                     {
                        entry->array[3][0][1] = entry->array[3][0][0];
                        entry->array[3][0][2] = '\0';
                        entry->array[3][0][0] = '0';
                     }
                     // year number == [6]
                     //printf("build number (%s)\n", entry->array[0][0]);
                     snprintf(g_static_hostBuildDate, 63,
                     "%s.%s.%s %s", entry->array[3][0], entry->array[2][0], entry->array[6][0], entry->array[4][0] );
                  }
                  else
                  {
                     strcpy(g_static_hostBuildDate, "N/A <!-- cannot parse date -->");
                  }
               }
            }
         }

         insider = NULL;
         ival = charArray_getLineThatMatches(&array,"Linux version ", &insider);
         if ( ival > 0 && insider )
         {
            ival = lineParser_splitTag(&parser, insider, " ", &entry);
            if ( ival > 0 && entry && entry->row_amount >= 3 )
            {
               //printf("entry->array[2][0] value (%s)\n", entry->array[2][0]);
               str_ret = strstr(entry->array[2][0],"L.");
               if ( str_ret )
               {
                  //printf("str_ret(%s)\n", str_ret);
                  strncpy(temp, str_ret, 255);
                  temp[255]='\0';
                  str_ret = strstr(temp,"_");
                  if ( str_ret )
                  {
                     str_ret[0]='\0';
                     strncpy(g_static_hostBuildVersion, temp, 63);
                     g_static_hostBuildVersion[63]='\0';
                  }
               }
            }
         }
      }
   }

   ival = C_FileExists("/dev/ttySTDRV002_0");
   if ( ival >= 1 && special_flags )
   {
      *special_flags |= HAIOS_FLAG_SDATA;
      ival = C_FileExists("/etc/haios.ini");
      if ( ival >= 1 )
      {
         charArray_Zero(&array);
         ival = file_to_array("/etc/haios.ini", &array);
         if ( ival > 0 )
         {
            ival = charArray_getLineThatMatches(&array, "CoTSupportEnabled=1", &insider);
            if ( ival > 0 && insider )
            {
               *special_flags |= HAIOS_FLAG_SDATA_COT;
            }
         }
      }
   }

   if ( build_version_insider )
   {
      *build_version_insider = &g_static_hostBuildVersion[0];
   }

   if ( build_date_insider )
   {
      *build_date_insider = &g_static_hostBuildDate[0];
   }

   charArray_Destructor(&array);
   lineParser_Destructor(&parser);

   return hostVersion;

}

// this is the version from the ugly hack to read the version in a text file.
int HAIOS_getSystemVersion(char ** package_version_insider)
{
   char * insider = NULL;
   //charArray array;
   //charArray * entry = NULL;
   //lineParser parser;
   int ival, status;
   //int hostVersion=2;
   //char temp[256]="";
   char * strRet;

   if ( package_version_insider )
   {
      *package_version_insider = NULL;

      if ( g_static_systemVersion[0] != '\0' )
      {
         *package_version_insider = g_static_systemVersion;
         return 1;
      }

      ival = C_System("/usr/bin/system get revision", &insider, &status);

      if ( ival > 0 && insider && strstr(insider, "Invalid")==NULL && strstr(insider, "UNKNOWN")==NULL )
      {
         strRet = strchr(insider, '\n');
         if ( strRet )
         {
            *strRet = '\0';
         }
         strRet = strchr(insider, '\r');
         if ( strRet )
         {
            *strRet = '\0';
         }

         strRet = strstr(insider, ": ");
         if ( strRet == NULL )
         {
            strRet = insider;
         }
         else
         {
            strRet += 2;
         }
         strncpy(g_static_systemVersion, strRet, 63);
         g_static_systemVersion[63] = '\0';

         if ( package_version_insider )
         {
            *package_version_insider = g_static_systemVersion;
         }
         return 1;
      }
   }

   return 0;
}



unsigned int getHardwareInfo()
{
   static unsigned int i = 0;
   int ival;

   charArray textArray;

   if ( i != 0 )
   {
      return i;
   }

   charArray_Constructor(&textArray, 2);

   ival = file_to_array(CARD_ID_SYSFS_FNAME, &textArray);
   if ( ival > 0 && textArray.row_amount > 0 )
   {
      i = C_axtoi(textArray.array[0][0]);
   }

   charArray_Destructor(&textArray);

   return i;
}

