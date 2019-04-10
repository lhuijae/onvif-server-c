/*
 --------------------------------------------------------------------------
 ServiceDevice.cpp
 
 Implementation of functions (methods) for the service:
 ONVIF devicemgmt.wsdl server side
-----------------------------------------------------------------------------
*/

//#include "soapDeviceBindingService.h"
#include "ServiceContext.h"
#include "smacros.h"
#include "hai_soap.h"

#define HAI_SHORT_STRING_LENGTH 32
#define ONVIF_SCOPE_NAME "onvif://www.onvif.org/type/video_encoder onvif://www.onvif.org/type/audio_encoder onvif://www.onvif.org/type/ptz onvif://www.onvif.org/location/country/canada onvif://www.onvif.org/name/NVTCYH onvif://www.onvif.org/hardware/NVT20130806-01"

#define START_TIME_YEAR 1900
#define ONE_HOUR_MIN 60
#define ONE_MIN_SEC 60
#define ONE_HOUR_SEC ONE_HOUR_MIN * ONE_MIN_SEC

int __tds__GetServices(struct soap *soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "starting\n");

    ServiceContext* ctx = (ServiceContext*)soap->user;

    char XAddr[128];
    char XAddrRoot[128];
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "will get xaddr\n");
    int iret = ServiceContext_getXAddr(ctx, soap, XAddr, sizeof(XAddr));
    strcpy(XAddrRoot, XAddr);
    printf("getXaddr %s\n", XAddr);
    
    if (tds__GetServicesResponse->__sizeService > 0)
    {
       DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "!!! tds__GetServicesResponse->__sizeService: %d\n", tds__GetServicesResponse->__sizeService);
    }
    
    tds__GetServicesResponse->__sizeService = 2;
    tds__GetServicesResponse->Service = hai_soap_malloc(soap, sizeof(struct tds__Service)*tds__GetServicesResponse->__sizeService);
    if (!tds__GetServicesResponse->Service)
    {
       // FIXME error malloc
       DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "malloc error");
       return SOAP_ERR;
    }
    
    //Device Service
    tds__GetServicesResponse->Service[0].Namespace  = "http://www.onvif.org/ver10/device/wsdl";
    strcat(XAddr, "/onvif/devices");
    tds__GetServicesResponse->Service[0].XAddr      = soap_strdup(soap, XAddr);
    
    tds__GetServicesResponse->Service[0].Version = (struct tt__OnvifVersion *) hai_soap_malloc(soap, sizeof(struct tt__OnvifVersion));
    tds__GetServicesResponse->Service[0].Version->Major = 2;
    tds__GetServicesResponse->Service[0].Version->Minor = 20; // FIXME(flemieux) what is this version represents?
    
    if (tds__GetServices->IncludeCapability)
    {
       tds__GetServicesResponse->Service[0].Capabilities = NULL;//hai_soap_malloc(soap, sizeof(struct _tds__Service_Capabilities));
    //   FillDeviceCapabilities(soap, ctx, tds__GetServicesResponse->Service[0].Capabilities);
    }
   
    // Media Service
    tds__GetServicesResponse->Service[1].Namespace  = "http://www.onvif.org/ver10/media/wsdl";
    strcpy(XAddr, XAddrRoot);
    strcat(XAddr, "/onvif/media");
    tds__GetServicesResponse->Service[1].XAddr      = soap_strdup(soap, XAddr);
    printf("returning media service xaddr %s\n", tds__GetServicesResponse->Service[1].XAddr);
    
    tds__GetServicesResponse->Service[1].Version = (struct tt__OnvifVersion *) hai_soap_malloc(soap, sizeof(struct tt__OnvifVersion));
    tds__GetServicesResponse->Service[1].Version->Major = 2;
    tds__GetServicesResponse->Service[1].Version->Minor = 7; // FIXME(flemieux) what is this version represents?
    
    if (tds__GetServices->IncludeCapability)
    {
        DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Was asked to IncludeCapability\n");
        tds__GetServicesResponse->Service[1].Capabilities = hai_soap_malloc(soap, sizeof(struct _tds__Service_Capabilities));
        struct trt__Capabilities *capabilities = ServiceContext_getMediaServiceCapabilities(ctx, soap);
        soap_elt_set(&tds__GetServicesResponse->Service[1].Capabilities->__any, NULL, "trt:Capabilities");
        soap_elt_node(&tds__GetServicesResponse->Service[1].Capabilities->__any, capabilities, SOAP_TYPE_trt__Capabilities);
    }
    
    printf("returning two services\n");

    return SOAP_OK;
}



int __tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    ServiceContext* ctx = (ServiceContext*)soap->user;
    tds__GetServiceCapabilitiesResponse->Capabilities = ServiceContext_getDeviceServiceCapabilities(ctx, soap);

    return SOAP_OK;
}


int __tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    ServiceContext* ctx = (ServiceContext*)soap->user;
    tds__GetDeviceInformationResponse->Manufacturer    = ctx->manufacturer;
    tds__GetDeviceInformationResponse->Model           = ctx->model;
    tds__GetDeviceInformationResponse->FirmwareVersion = ctx->firmware_version;
    tds__GetDeviceInformationResponse->SerialNumber    = ctx->serial_number;
    tds__GetDeviceInformationResponse->HardwareId      = ctx->hardware_id;

    return SOAP_OK;
}


int __tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


struct tt__DateTime * getSystemDateAndTimeDateTimeValue(struct soap* soap, struct tm* timeValue)
{
	struct tt__DateTime * result = (struct tt__DateTime *) hai_soap_malloc(soap, sizeof(struct tt__DateTime));
	result->Date = (struct tt__Date *) hai_soap_malloc(soap, sizeof(struct tt__Date));
	result->Date->Year = timeValue->tm_year + START_TIME_YEAR;
	result->Date->Month = timeValue->tm_mon + 1;
	result->Date->Day = timeValue->tm_mday;
	result->Time = (struct tt__Time *) hai_soap_malloc(soap, sizeof(struct tt__Time));
	result->Time->Hour = timeValue->tm_hour;
	result->Time->Minute = timeValue->tm_min;
	result->Time->Second = timeValue->tm_sec;
	return result;
}

struct tt__SystemDateTime* getSystemDateAndTimeSystemTimeInfo(struct soap* soap) 
{
   time_t rawtime;
   struct tm * timeinfo;
   time (&rawtime);
   struct tt__SystemDateTime* result = (struct tt__SystemDateTime *) hai_soap_malloc(soap, sizeof(struct tt__SystemDateTime));
   result->DateTimeType = tt__SetDateTimeType__Manual;
   //if (info->ntpSet) {
   //   result->DateTimeType = tt__SetDateTimeType__NTP;
   //   return result;
   //}
   result->DaylightSavings = xsd__boolean__false_;
   struct tm* tm1;
   tm1 = localtime(&rawtime);
   result->LocalDateTime = getSystemDateAndTimeDateTimeValue(soap, tm1);
   tm1 = gmtime(&rawtime);
   result->UTCDateTime = getSystemDateAndTimeDateTimeValue(soap, tm1);
   result->TimeZone = (struct tt__TimeZone *) hai_soap_malloc(soap, sizeof(struct tt__TimeZone));
   result->TimeZone->TZ = (char*) hai_soap_malloc(soap, HAI_SHORT_STRING_LENGTH);
   sprintf(result->TimeZone->TZ, "GMT+%d", 5); // FIXME
   return result;
}


int __tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    const time_t  timestamp = time(NULL);
    struct tm    *tm        = gmtime(&timestamp);

    if (tds__GetSystemDateAndTimeResponse->SystemDateAndTime == NULL)
    {
       DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: SystemDateAndTime is NULL\n");
       //return SOAP_ERR;
    }
  
    tds__GetSystemDateAndTimeResponse->SystemDateAndTime = getSystemDateAndTimeSystemTimeInfo(soap);
    
    return SOAP_OK;
}



int __tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__GetSystemSupportInformation(struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
    size_t i = 1;
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    ServiceContext* sc = (ServiceContext*)soap->user;

    if (sc->scopes.itemAmount > 0)
    {
       tds__GetScopesResponse->Scopes = hai_soap_malloc(soap, sizeof(struct tt__Scope)*sc->scopes.itemAmount+1);
       tds__GetScopesResponse->__sizeScopes = sc->scopes.itemAmount;

       tds__GetScopesResponse->Scopes[0].ScopeItem = soap_strdup(soap, ONVIF_SCOPE_NAME);
       tds__GetScopesResponse->Scopes[0].ScopeDef = tt__ScopeDefinition__Fixed;
       char *scope = NULL;
       int iret = linkedList_FirstItem(&sc->scopes, (void**)&scope);
       while (iret >= 1 && scope)
       {
           tds__GetScopesResponse->Scopes[i].ScopeItem = soap_strdup(soap, scope);
           tds__GetScopesResponse->Scopes[i].ScopeDef = tt__ScopeDefinition__Fixed;
           iret = linkedList_NextItem(&sc->scopes, (void**)&scope);
           i++;
       }
    }

    return SOAP_OK;
}



int __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDiscoveryMode(struct soap *soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetDiscoveryMode(struct soap *soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    ServiceContext* sc = (ServiceContext*)soap->user;

    if (sc->user)
    {
        tds__GetUsersResponse->__sizeUser = 1;
        tds__GetUsersResponse->User = hai_soap_malloc(soap, sizeof(struct tt__User));
        tds__GetUsersResponse->User[0].Username = soap_strdup(soap, sc->user);
    }

    return SOAP_OK;
}



int __tds__CreateUsers(struct soap* soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}

int getCapabilitiesResponseExtensionDeviceIO(struct soap* soap, struct tt__CapabilitiesExtension *capabilitiesExtension, char* address) 
{
   capabilitiesExtension->DeviceIO = (struct tt__DeviceIOCapabilities*) hai_soap_malloc(soap, sizeof(struct tt__DeviceIOCapabilities));
   capabilitiesExtension->DeviceIO->XAddr = soap_strdup(soap, address);
   int vSource = 1;
   int ret = 1;// TODO getVideoCount(&vSource);
   if (!ret)
   {
      return -10;
   }
   
   int aSource = 1;
   ret = 1; // TODO getAudioCount(&aSource);
   if (!ret)
   {
      return -20;
   }
   
   capabilitiesExtension->DeviceIO->VideoSources = vSource;
   capabilitiesExtension->DeviceIO->VideoOutputs = 0;
   capabilitiesExtension->DeviceIO->AudioSources = aSource;
   capabilitiesExtension->DeviceIO->AudioOutputs = 0;
   capabilitiesExtension->DeviceIO->RelayOutputs = 0;
   capabilitiesExtension->DeviceIO->__size = 0;
   capabilitiesExtension->DeviceIO->__any = NULL;
   return SOAP_OK;
}

static SOAP_FMAC5 int SOAP_FMAC6 getCapabilitiesResponseExtension(struct soap* soap, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse, char* address) 
{
   tds__GetCapabilitiesResponse->Capabilities->Extension = (struct tt__CapabilitiesExtension*) hai_soap_malloc(soap, sizeof(struct tt__CapabilitiesExtension));
   int result = getCapabilitiesResponseExtensionDeviceIO(soap, tds__GetCapabilitiesResponse->Capabilities->Extension, address);
   if (SOAP_OK != result) 
   {
      return result;
   }
   tds__GetCapabilitiesResponse->Capabilities->Extension->Display = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->Recording = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->Search = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->Replay = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions = NULL;
   tds__GetCapabilitiesResponse->Capabilities->Extension->__size = 0;
   tds__GetCapabilitiesResponse->Capabilities->Extension->__any = NULL;
   return SOAP_OK;
}


static void getCapabilitiesResponseMedia(struct soap* soap, struct tt__Capabilities *capabilities, char* address) 
{
	capabilities->Media = hai_soap_malloc(soap, sizeof(struct tt__MediaCapabilities));
    char XAddr[128];
    strcpy(XAddr, address);
    strcat(XAddr, "/onvif/media");
	capabilities->Media->XAddr = soap_strdup(soap, XAddr);
	capabilities->Media->StreamingCapabilities = hai_soap_malloc(soap, sizeof(struct tt__RealTimeStreamingCapabilities));
	
	enum xsd__boolean bvalue = xsd__boolean__true_;
	capabilities->Media->StreamingCapabilities->RTPMulticast = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
	
	bvalue = xsd__boolean__false_;
	capabilities->Media->StreamingCapabilities->RTP_USCORETCP = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
	
	bvalue = xsd__boolean__true_;
	capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP =  soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
	
	capabilities->Media->StreamingCapabilities->Extension = NULL;
	capabilities->Media->Extension = NULL;
	capabilities->Media->__size = 0;
	capabilities->Media->__any = 0;
}

void FillDeviceCapabilities(struct soap *soap, ServiceContext* ctx, struct _tds__Service_Capabilities * serviceCapabilities)
{
// not needed
}

int  __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
    char XAddr[128];
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);

    ServiceContext* sc = (ServiceContext*)soap->user;
    
    int iret = ServiceContext_getXAddr(sc, soap, XAddr, sizeof(XAddr));

    printf("GOT DEVICE XAddr %s\n", XAddr);

    
    int category;
//    char IPv4Address[HAIVISION_MAX_ONVIF_IPV4_LENGTH];

    // Are we being asked a specific category ? If not use All.
    if (tds__GetCapabilities->Category == NULL || tds__GetCapabilities->__sizeCategory == 0)
    {
        tds__GetCapabilities->Category = hai_soap_malloc(soap, sizeof(enum tt__CapabilityCategory));
        *tds__GetCapabilities->Category = tt__CapabilityCategory__All;
        category = tt__CapabilityCategory__All;
    }
    else
    {
        category = *tds__GetCapabilities->Category;
    }
    
    tds__GetCapabilitiesResponse->Capabilities = hai_soap_malloc(soap, sizeof(struct tt__Capabilities));
    if (!tds__GetCapabilitiesResponse->Capabilities)
    {
      // FIXME error malloc
      return SOAP_ERR;
    }
    
    memset(tds__GetCapabilitiesResponse->Capabilities, 0, sizeof(struct tt__Capabilities));
    
    tds__GetCapabilitiesResponse->Capabilities->Analytics = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Events = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Imaging = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Media = NULL;
    tds__GetCapabilitiesResponse->Capabilities->PTZ = NULL;
    
    //int result = getCapabilitiesResponseExtension(soap, tds__GetCapabilitiesResponse, XAddr);
    //if (SOAP_OK != result)
    //{
    //    return result;
    //}
    
    
    tds__GetCapabilitiesResponse->Capabilities->Device = hai_soap_malloc(soap, sizeof(struct tt__DeviceCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = soap_strdup(soap, XAddr);
    tds__GetCapabilitiesResponse->Capabilities->Device->Extension = NULL;

    tds__GetCapabilitiesResponse->Capabilities->Device->Network = hai_soap_malloc(soap, sizeof(struct tt__NetworkCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = hai_soap_malloc(soap, sizeof(enum xsd__boolean));
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = (int *) hai_soap_malloc(soap, sizeof(enum xsd__boolean));
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = hai_soap_malloc(soap, sizeof(enum xsd__boolean));
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = hai_soap_malloc(soap, sizeof(enum xsd__boolean));

    *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = xsd__boolean__false_;
    *tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = xsd__boolean__false_;
    *tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = xsd__boolean__false_;
    *tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension = hai_soap_malloc(soap, sizeof(struct tt__NetworkCapabilitiesExtension));
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = hai_soap_malloc(soap, sizeof(enum xsd__boolean));
    *tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Dot11Configuration = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->Extension = NULL;

    tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->__size= 0;
    tds__GetCapabilitiesResponse->Capabilities->Device->Network->Extension->__any = NULL;

    tds__GetCapabilitiesResponse->Capabilities->Device->System = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security = NULL;
#if 0
    tds__GetCapabilitiesResponse->Capabilities->Device->System = hai_soap_malloc(soap, sizeof(struct tt__SystemCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = xsd__boolean__true_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions = (struct tt__OnvifVersion*) hai_soap_malloc(soap, sizeof(struct tt__OnvifVersion));
    tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major = MAJOR;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor = MINOR;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension = (struct tt__SystemCapabilitiesExtension*) hai_soap_malloc(soap, sizeof(struct tt__SystemCapabilitiesExtension));
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = (int *) hai_soap_malloc(soap, sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = (int *) hai_soap_malloc(soap, sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade = xsd__boolean__true_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = (int *) hai_soap_malloc(soap, sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging = xsd__boolean__true_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = (int *) hai_soap_malloc(soap, sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation = xsd__boolean__true_;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->Extension = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->__size = 0;
    tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->__any = NULL;

    tds__GetCapabilitiesResponse->Capabilities->Device->IO = (struct tt__IOCapabilities*) hai_soap_malloc(soap, sizeof(struct tt__IOCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = &_false;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = &_false;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension = (struct tt__IOCapabilitiesExtension *) hai_soap_malloc(soap, sizeof(struct tt__IOCapabilitiesExtension));
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->__size =0;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->__any = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->Auxiliary = &_false;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->__anyAttribute = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->__sizeAuxiliaryCommands = 0;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->AuxiliaryCommands= NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->Extension = NULL;
#endif
#if 0
    tds__GetCapabilitiesResponse->Capabilities->Device->Security = (struct tt__SecurityCapabilities*) hai_soap_malloc(soap, sizeof(struct tt__SecurityCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken = FALSE;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->Extension = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->__size = 0;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->__any = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->__anyAttribute = NULL;
    tds__GetCapabilitiesResponse->Capabilities->Device->Extension = NULL;
#endif

   //getCapabilitiesResponseDevice(soap, tds__GetCapabilitiesResponse->Capabilities, XAddr);
   getCapabilitiesResponseMedia(soap, tds__GetCapabilitiesResponse->Capabilities, XAddr);
   //getCapabilitiesResponsePTZ(soap, tds__GetCapabilitiesResponse->Capabilities, XAddr);
   
    /* C++ version
    for (tt__CapabilityCategory category : categories)
    {
        if (!tds__GetCapabilitiesResponse.Capabilities->Device && ( (category == tt__CapabilityCategory__All) || (category == tt__CapabilityCategory__Device) ) )
        {
            tds__GetCapabilitiesResponse.Capabilities->Device = soap_new_tt__DeviceCapabilities(soap);
            tds__GetCapabilitiesResponse.Capabilities->Device->XAddr = XAddr;
            tds__GetCapabilitiesResponse.Capabilities->Device->System = soap_new_tt__SystemCapabilities(soap);
            tds__GetCapabilitiesResponse.Capabilities->Device->System->SupportedVersions.push_back(soap_new_req_tt__OnvifVersion(soap, 2, 0));
            tds__GetCapabilitiesResponse.Capabilities->Device->Network = soap_new_tt__NetworkCapabilities(soap);
            tds__GetCapabilitiesResponse.Capabilities->Device->Security = soap_new_tt__SecurityCapabilities(soap);
            tds__GetCapabilitiesResponse.Capabilities->Device->IO = soap_new_tt__IOCapabilities(soap);
        }


        if (!tds__GetCapabilitiesResponse.Capabilities->Media && ( (category == tt__CapabilityCategory__All) || (category == tt__CapabilityCategory__Media) ) )
        {
            tds__GetCapabilitiesResponse.Capabilities->Media  = soap_new_tt__MediaCapabilities(soap);
            tds__GetCapabilitiesResponse.Capabilities->Media->XAddr = XAddr;
            tds__GetCapabilitiesResponse.Capabilities->Media->StreamingCapabilities = soap_new_tt__RealTimeStreamingCapabilities(soap);
        }
    }*/


    return SOAP_OK;
}



int __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int  __tds__SetHostnameFromDHCP(struct soap *soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
    size_t i;  
    char ipv4_buf[20], mac_addr[40];
    uint32_t if_ip, if_mask;
        
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    printf("printf: Device: %s\n", __FUNCTION__);

    ServiceContext *sc = (ServiceContext*)soap->user;

    EthDevParam *ethDevParam = NULL;
    int iret = linkedList_FirstItem(&sc->eth_ifs, (void**)&ethDevParam);
    if (iret <= 0 || ethDevParam == NULL)
    {
       return SOAP_ERR;
    }
  
    EthDevParam_get_ip(ethDevParam, &if_ip);
    EthDevParam_get_ip_from_string(ethDevParam, &ipv4_buf[0]);
    EthDevParam_get_hwaddr_from_string(ethDevParam, mac_addr);
     
    EthDevParam_get_mask(ethDevParam, &if_mask);
    char * dev_name = EthDevParam_dev_name(ethDevParam);
    int prefix = EthDevParam_get_mask_prefix(ethDevParam);
     
    printf("giving our IP %s\n", ipv4_buf);
    
    tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;
    tds__GetNetworkInterfacesResponse->NetworkInterfaces = hai_soap_malloc(soap, sizeof(struct tt__NetworkInterface));
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled = xsd__boolean__true_;
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->token = soap_strdup(soap, "eth0"); // FIXME
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info = hai_soap_malloc(soap, sizeof(struct tt__NetworkInterfaceInfo));
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name = soap_strdup(soap, dev_name);
    // C++ tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name->assign(dev_name);
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link = hai_soap_malloc(soap, sizeof(struct tt__NetworkInterfaceLink));
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->InterfaceType = 6;
    printf("Link IT 6\n");
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress = soap_strdup(soap, mac_addr);
  
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4 = hai_soap_malloc(soap, sizeof(struct tt__IPv4NetworkInterface));
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled = xsd__boolean__true_;
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config = hai_soap_malloc(soap, sizeof(struct tt__IPv4Configuration));
    
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal = hai_soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->Address = soap_strdup(soap, ipv4_buf);
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->PrefixLength = 0;
    
    if (1)
    {
        tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
        tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual = hai_soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
        tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address = soap_strdup(soap, ipv4_buf);
        tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->PrefixLength = prefix;
    }
    else
    {
        tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP = hai_soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
    }


    return SOAP_OK;
}



int __tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkMETADATA_SCROLL_MESMERIZER_TMs, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetPkcs10Request(struct soap *soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__LoadCertificates(struct soap *soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int  __tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetRelayOutputs(struct soap* soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__LoadCertificateWithPrivateKey(struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetCertificateInformation(struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__ScanAvailableDot11Networks(struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetStorageConfigurations(struct soap *soap, struct _tds__GetStorageConfigurations *tds__GetStorageConfigurations, struct _tds__GetStorageConfigurationsResponse *tds__GetStorageConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__CreateStorageConfiguration(struct soap *soap, struct _tds__CreateStorageConfiguration *tds__CreateStorageConfiguration, struct _tds__CreateStorageConfigurationResponse *tds__CreateStorageConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__GetStorageConfiguration(struct soap *soap, struct _tds__GetStorageConfiguration *tds__GetStorageConfiguration, struct _tds__GetStorageConfigurationResponse *tds__GetStorageConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__SetStorageConfiguration(struct soap *soap, struct _tds__SetStorageConfiguration *tds__SetStorageConfiguration, struct _tds__SetStorageConfigurationResponse *tds__SetStorageConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__DeleteStorageConfiguration(struct soap *soap, struct _tds__DeleteStorageConfiguration *tds__DeleteStorageConfiguration, struct _tds__DeleteStorageConfigurationResponse *tds__DeleteStorageConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __tds__GetGeoLocation(struct soap *soap, struct _tds__GetGeoLocation *tds__GetGeoLocation, struct _tds__GetGeoLocationResponse *tds__GetGeoLocationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__SetGeoLocation(struct soap *soap, struct _tds__SetGeoLocation *tds__SetGeoLocation, struct _tds__SetGeoLocationResponse *tds__SetGeoLocationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __tds__DeleteGeoLocation(struct soap *soap, struct _tds__DeleteGeoLocation *tds__DeleteGeoLocation, struct _tds__DeleteGeoLocationResponse *tds__DeleteGeoLocationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Device: %s\n", __FUNCTION__);
    return SOAP_OK;
}

