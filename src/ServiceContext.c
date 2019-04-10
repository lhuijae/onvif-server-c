#include <arpa/inet.h>
#include "ServiceContext.h"
#include "hai_soap.h"



void ServiceContext_Constructor(ServiceContext * sc)
{
    char * build_version_insider = NULL;
    char * build_date_insider = NULL;
    unsigned int special_flags = 0;
    sc->port = 1000;
    strcpy(sc->user, "admin");
    strcpy(sc->password, "manager");

    //Device Information
    strcpy(sc->manufacturer, "Haivision Systems Inc.");
    strcpy(sc->model, hai_get_card_type());
    strcpy(sc->firmware_version, hai_get_system_firmware());
    strcpy(sc->serial_number, hai_get_system_serial());
    sprintf(sc->hardware_id, hai_get_hardware_id());
    
    linkedList_Constructor(&sc->scopes); // of types char *
    linkedList_Constructor(&sc->eth_ifs); // of types EthDevParam for ethernet interfaces
    sc->profiles.key = NULL;
    sc->profiles.next = NULL; // of types char *, StreamProfile
}


char* StreamProfile_get_name(const StreamProfile * sp) { return sp->name;  }
int StreamProfile_get_width (const StreamProfile * sp) { return sp->width; }
int StreamProfile_get_height(const StreamProfile * sp) { return sp->height; }
char* StreamProfile_get_url(const StreamProfile * sp) { return &sp->url[0]; }
int StreamProfile_get_type(const StreamProfile * sp) { return sp->type; }
char * StreamProfile_get_str_err(StreamProfile * sp)  { return sp->str_err; }
const char* StreamProfile_get_cstr_err(StreamProfile * sp) { return sp->str_err; }

StreamProfileMap * StreamProfileMapFind(const StreamProfileMap * map, const char * findme)
{
   if (map->key == NULL) return NULL;
   
   while (map != NULL)
   {
      if (strcmp(map->key, findme) == 0)
      {
         return map;
      }
      map = map->next;
   }
   
   return NULL;
}


// will return the new map item with allocated client object.
StreamProfileMap * StreamProfileMapAdd(StreamProfileMap * map, const char * newkey, struct soap *soap)
{
   StreamProfileMap * newItem = NULL;
   if (map == NULL) return NULL;
   
   if (map->key == NULL)
   {
      // first item
      printf("using first item\n");
      newItem = map;
   }
      
   while (map->next != NULL)
   {
      map = map->next;
   }
   
   if (newItem == NULL)
   {
     newItem = malloc(sizeof(StreamProfileMap));
   }
   
   if (newItem == NULL)
   { 
      return NULL;
   }
   
   newItem->next = NULL;
   int abc = sizeof(StreamProfile);
   void* def = hai_soap_malloc(soap, abc);
   printf("allocaing size %d, ptr 0x%X\n", abc, (unsigned int)def);
   newItem->streamProfile = def;
   newItem->key = strdup(newkey);
   
   return newItem;
}




char * ServiceContext_getServerIpFromClientIp(ServiceContext * sc, uint32_t client_ip)
{
    char server_ip[INET_ADDRSTRLEN] = "";
    size_t i;
    EthDevParam *ethDevParam = NULL;
    char ipv4[INET_ADDRSTRLEN] = "";

    int iret = linkedList_FirstItem(&sc->eth_ifs, (void**)&ethDevParam);
    while (iret >= 1 && ethDevParam)
    {
		uint32_t if_ip, if_mask;
		EthDevParam_get_ip(ethDevParam, &if_ip);
		EthDevParam_get_ip_from_string(ethDevParam, ipv4); // FIXME That is not original code intent
      EthDevParam_get_mask(ethDevParam, &if_mask);
      if ( (if_ip & if_mask) == (client_ip & if_mask) )
      {
          if (EthDevParam_get_ip_from_string(ethDevParam, server_ip) == 0)
          { 
              // ok

              return strdup(server_ip);
          }
          else
          { 
             // TODO: log error
             return strdup("127.0.0.1");  //localhost
          }
      }
		else
		{
			printf("WARNING: Returning our own IPv4 %s\n", ipv4); // FIXME 
         if (g_public_ip[0] != '\0')
         {
           printf("Using global ip %s\n", g_public_ip);
           DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "ServiceContext_getServerIpFromClientIp Using global ip %s\n", g_public_ip);
           return strdup(g_public_ip);
         }
			return strdup(ipv4);
		}
        
      iret = linkedList_NextItem(&sc->eth_ifs, (void**)&ethDevParam);
    }

    return strdup("127.0.0.1");  //localhost
}



int ServiceContext_getXAddr(ServiceContext * sc, struct soap *soap, char * loadme, size_t loadme_size)
{
printf("step1\n");
    const char * ip = ServiceContext_getServerIpFromClientIp(sc, htonl(soap->ip));
    
    if (g_public_ip[0] != '\0')
    {
        ip = g_public_ip;
        printf("Using global ip XAddr %s\n", g_public_ip);
        DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Using global ip XAddr %s\n", g_public_ip);
    }
    
printf("step2\n");
    int result = snprintf(loadme, loadme_size-1, "http://%s:%d", ip, sc->port);
    if (result >= loadme_size)
    {
       // FIXME log error
       return -1; // was truncated !!
    }
    return 1; // ok
}


int ServiceContext_add_profile(ServiceContext * sc, const StreamProfile * profile, struct soap *soap)
{
    if (!StreamProfile_is_valid(profile))
    {
        strcpy(sc->str_err, "profile has unset parameters");
        printf("ERROR: Profile has unset parameters\n");
        return -10;
    }

    char * name = StreamProfile_get_name(profile);    
    
printf("will add profile from name %s\n", name);
    StreamProfileMap * streamProfileMap = StreamProfileMapFind(&sc->profiles, name);
    if (streamProfileMap != NULL)
    {
        snprintf(sc->str_err, sizeof(sc->str_err)-1, "profile: %s already exists", name);
        return -20;
    }

    StreamProfileMap * newItem = StreamProfileMapAdd(&sc->profiles, name, soap);    
    memcpy(newItem->streamProfile, profile, sizeof(StreamProfile));
    printf("added profile key %s, profile ptr %0xX, name %s\n", newItem->key, (unsigned int)newItem->streamProfile, newItem->streamProfile->name);
    return 1;
}



char * ServiceContext_get_stream_uri(ServiceContext * sc, const char * profile_url, uint32_t client_ip)
{
   const char * strRet = strstr(profile_url, "%s");
   char * result = NULL;
   size_t result_size;
   if (strRet)
   {   
      // replace %s by client_ip
      const char * ip = ServiceContext_getServerIpFromClientIp(sc, client_ip);
      result_size = strlen(profile_url) + strlen(ip) + 1;
      result = malloc(result_size);
      memset(result, 0, result_size);
      snprintf(result, result_size-1, profile_url, ip);
   }
   else
   {
      result = strdup(profile_url);
   }

   return result;
}


const StreamProfileMap * ServiceContext_get_profiles(ServiceContext * sc)
{ 
  return &(sc->profiles);
}


struct tds__DeviceServiceCapabilities* ServiceContext_getDeviceServiceCapabilities(ServiceContext * sc, struct soap *soap)
{
    enum xsd__boolean bvalue = xsd__boolean__false_;
    int ivalue = 0;
    
    struct tds__DeviceServiceCapabilities *capabilities = soap_new_tds__DeviceServiceCapabilities(soap, -1);

    capabilities->Network = soap_new_tds__NetworkCapabilities(soap, -1);

    capabilities->Network->IPFilter            = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->ZeroConfiguration   = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->IPVersion6          = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->DynDNS              = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->Dot11Configuration  = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->Dot1XConfigurations = soap_new_ptr(soap, &ivalue, sizeof(int));
    capabilities->Network->HostnameFromDHCP    = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Network->NTP                 = soap_new_ptr(soap, &ivalue, sizeof(int));
    capabilities->Network->DHCPv6              = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));

    capabilities->Security = soap_new_tds__SecurityCapabilities(soap, -1);

    capabilities->Security->TLS1_x002e0          = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->TLS1_x002e1          = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->TLS1_x002e2          = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->OnboardKeyGeneration = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->AccessPolicyConfig   = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->DefaultAccessPolicy  = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->Dot1X                = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->RemoteUserHandling   = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->X_x002e509Token      = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->SAMLToken            = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->KerberosToken        = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->UsernameToken        = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->HttpDigest           = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->RELToken             = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->Security->MaxUsers             = soap_new_ptr(soap, &ivalue, sizeof(int));
    capabilities->Security->MaxUserNameLength    = soap_new_ptr(soap, &ivalue, sizeof(int));
    capabilities->Security->MaxPasswordLength    = soap_new_ptr(soap, &ivalue, sizeof(int));


    capabilities->System = soap_new_tds__SystemCapabilities(soap, 1);

    bvalue = xsd__boolean__true_;
    capabilities->System->DiscoveryResolve       = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->DiscoveryBye           = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->RemoteDiscovery        = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    bvalue = xsd__boolean__false_;
    capabilities->System->SystemBackup           = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->SystemLogging          = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->FirmwareUpgrade        = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->HttpFirmwareUpgrade    = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->HttpSystemBackup       = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->HttpSystemLogging      = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->HttpSupportInformation = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    capabilities->System->StorageConfiguration   = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));


    return capabilities;
}



struct trt__Capabilities *ServiceContext_getMediaServiceCapabilities(ServiceContext * sc, struct soap *soap)
{
    enum xsd__boolean bvalue = xsd__boolean__false_;
    struct trt__Capabilities *capabilities = soap_new_trt__Capabilities(soap, -1);

    capabilities->ProfileCapabilities = soap_new_trt__ProfileCapabilities(soap, -1);
    int ival = 24;
    capabilities->ProfileCapabilities->MaximumNumberOfProfiles = soap_new_ptr(soap, &ival, sizeof(int));

    capabilities->StreamingCapabilities = soap_new_trt__StreamingCapabilities(soap, -1);
    
    bvalue = xsd__boolean__true_;
    capabilities->StreamingCapabilities->RTPMulticast = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    
    bvalue = xsd__boolean__true_;
    capabilities->StreamingCapabilities->RTP_USCORETCP = soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));
    
    bvalue = xsd__boolean__true_;
    capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP =  soap_new_ptr(soap, &bvalue, sizeof(enum xsd__boolean));

    return capabilities;
} 




// ------------------------------- StreamProfile -------------------------------

void StreamProfile_Constructor(StreamProfile * sp)
{
        //StreamProfile_clear(sp);
        memset(sp, 0, sizeof(StreamProfile));
}

struct tt__VideoSourceConfiguration* StreamProfile_get_video_src_cnf(StreamProfile * sp, struct soap *soap)
{
    struct tt__VideoSourceConfiguration* src_cfg = soap_new_tt__VideoSourceConfiguration(soap, -1); // or ?(struct tt__VideoSourceConfiguration *) hai_soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration ));
    
    src_cfg->Name = (char *) hai_soap_malloc(soap,sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    src_cfg->token = (char *) hai_soap_malloc(soap,sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    src_cfg->SourceToken = (char *) hai_soap_malloc(soap,sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    src_cfg->Bounds = (struct tt__IntRectangle *) hai_soap_malloc(soap,sizeof(struct tt__IntRectangle));
    src_cfg->Extension = NULL;
    src_cfg->__any = NULL;
    src_cfg->__size = 0;
   
    if (sp->name[0] == '\0')
    {
       printf("ERROR: Cannot send empty token response\n");
       return NULL;
    }
    
    strcpy(src_cfg->Name, sp->name);
    strcpy(src_cfg->token, sp->name);
    strcat(src_cfg->token, "_video_source");
    strcpy(src_cfg->SourceToken, sp->name); 
    strcat(src_cfg->SourceToken, "_video_source");
    src_cfg->UseCount = 1;
    src_cfg->Bounds->x = 0;
    src_cfg->Bounds->y = 0;
    src_cfg->Bounds->height = 720;
    src_cfg->Bounds->width = 1280;

    return src_cfg;
}


static 

struct tt__VideoEncoderConfiguration* StreamProfile_get_video_enc_cfg(StreamProfile * sp, struct soap *soap)
{
    struct tt__VideoEncoderConfiguration* enc_cfg = soap_new_tt__VideoEncoderConfiguration(soap, -1); //(struct tt__VideoEncoderConfiguration *) hai_soap_malloc(soap,sizeof(struct tt__VideoEncoderConfiguration));
    
    enc_cfg->Name = (char *) hai_soap_malloc(soap,sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    enc_cfg->token = (char *) hai_soap_malloc(soap,sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    strcpy(enc_cfg->Name, sp->name);
    strcpy(enc_cfg->token, sp->name);
    strcat(enc_cfg->token, "_video_encoder");
    enc_cfg->UseCount = 1;
    enc_cfg->Quality = 80;
    enc_cfg->Encoding = (enum tt__VideoEncoding)(sp->type);//JPEG = 0, MPEG4 = 1, H264 = 2;
    enc_cfg->Resolution = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution));
    enc_cfg->Resolution->Height = 720;
    enc_cfg->Resolution->Width = 1280;
    enc_cfg->RateControl = hai_soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    enc_cfg->RateControl->FrameRateLimit = 60;
    enc_cfg->RateControl->EncodingInterval = 1;
    enc_cfg->RateControl->BitrateLimit = 40000;

    enc_cfg->MPEG4 = NULL;
    enc_cfg->H264 = NULL;

    enc_cfg->H264 = hai_soap_malloc(soap, sizeof(struct tt__H264Configuration));
    enc_cfg->H264->GovLength = 30;
    enc_cfg->H264->H264Profile = 1;

    enc_cfg->Multicast = (struct tt__MulticastConfiguration *) hai_soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    enc_cfg->Multicast->Address = hai_soap_malloc(soap, sizeof(struct tt__IPAddress));
    enc_cfg->Multicast->Address->Type = tt__IPType__IPv4;
    enc_cfg->Multicast->Address->IPv4Address = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_IPV4_LENGTH);
    strcpy(enc_cfg->Multicast->Address->IPv4Address, "0.0.0.0");//"224.65.11.199"); // FIXME use actual multicast if configured as such
    enc_cfg->Multicast->Address->IPv6Address = NULL;
    enc_cfg->Multicast->Port = 0; // FIXME correct port.
    enc_cfg->Multicast->TTL = 60;
    enc_cfg->Multicast->AutoStart = 0;
    enc_cfg->Multicast->__size = 0;
    enc_cfg->Multicast->__any = NULL;
    
    enc_cfg->SessionTimeout = 720000;
    enc_cfg->__size = 0;
    enc_cfg->__any = NULL;
printf("generated enc cfg\n");
    return enc_cfg;
}



struct tt__Profile* StreamProfile_get_profile(StreamProfile *sp, struct soap *soap)
{
    struct tt__Profile* profile = hai_soap_malloc(soap, sizeof(struct tt__Profile));
    
    if (sp == NULL)
    {//FIXME LOG ERROR
        printf("ERROR: StreamProfile_get_profile called with sp NULL ptr\n");
        return profile;
    }

    if (sp->name[0] == '\0')
    {//FIXME LOG ERROR
        printf("ERROR: StreamProfile_get_profile called with sp->name empty\n");
        return profile;
    }

    profile->Name  = sp->name;
    profile->token = sp->name;

    profile->VideoSourceConfiguration = StreamProfile_get_video_src_cnf(sp, soap);
    profile->VideoEncoderConfiguration = StreamProfile_get_video_enc_cfg(sp, soap);

    return profile;
}



struct tt__VideoSource* StreamProfile_get_video_src(StreamProfile *sp, struct soap *soap)
{    
    struct tt__VideoSource* video_src = hai_soap_malloc(soap, sizeof(struct tt__VideoSource));
    video_src->Framerate = 30;
    video_src->Resolution = (struct tt__VideoResolution *) hai_soap_malloc(soap, sizeof(struct tt__VideoResolution));
    video_src->Resolution->Height = sp->width;
    video_src->Resolution->Width = sp->height;
    video_src->token = (char *) hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    video_src->Imaging = hai_soap_malloc(soap, sizeof(struct tt__ImagingSettings));
    memset(video_src->Imaging, 0, sizeof(struct tt__ImagingSettings));
    //FIXME memset 0?
    strcpy(video_src->token, sp->name);

    return video_src;
}



int StreamProfile_set_name(StreamProfile *sp, const char *new_val)
{
    if (!new_val)
    {
        strcpy(sp->str_err, "Name is empty");
        return 0;
    }


    strncpy(sp->name, new_val, sizeof(sp->name)-1);
    return 1;
}



int StreamProfile_set_width(StreamProfile *sp, const char *new_val)
{
    char *pcEndPtr;
    int tmp_val = strtol(new_val, &pcEndPtr, 10);


    if( (tmp_val < 100) || (tmp_val >= 10000) )
    {
        strcpy(sp->str_err, "width is bad, correct range: 100-10000");
        return 0;
    }


    sp->width = tmp_val;
    return 1;
}



int StreamProfile_set_height(StreamProfile *sp, const char *new_val)
{
    char *pcEndPtr;
    int tmp_val = strtol(new_val, &pcEndPtr, 10);

    if ( (tmp_val < 100) || (tmp_val >= 10000) )
    {
        strcpy(sp->str_err, "height is bad, correct range: 100-10000");
        return 0;
    }

    sp->height = tmp_val;
    return 1;
}



int StreamProfile_set_url(StreamProfile *sp, const char *new_val)
{
    if (!new_val)
    {
        strcpy(sp->str_err, "URL is empty");
        return 0;
    }

    strncpy(sp->url, new_val, sizeof(sp->url)-1);
    return 1;
}



int StreamProfile_set_type(StreamProfile *sp, const char *new_type)
{
    if (strcasecmp(new_type, "JPEG") == 0)
        sp->type = tt__VideoEncoding__JPEG;
    else if (strcasecmp(new_type, "MPEG4") == 0)
        sp->type = tt__VideoEncoding__MPEG4;
    else if (strcasecmp(new_type, "H264") == 0)
        sp->type = tt__VideoEncoding__H264;
    else
    {
        strcpy(sp->str_err, "type dont support");
        return 0;
    }

    return 1;
}



void StreamProfile_clear(StreamProfile * sp)
{
    sp->name[0] = '\0';
    sp->url[0] = '\0';

    sp->width  = -1;
    sp->height = -1;
    sp->type   = -1;
}


char * ServiceContext_get_str_err(ServiceContext * sc) 
{
   return sc->str_err;
}


int StreamProfile_is_valid(const StreamProfile * sp)
{
    return ( sp->name[0] != '\0'  &&
             sp->url[0] != '\0'   &&
             (sp->width  != -1) &&
             (sp->height != -1) &&
             (sp->type   != -1)
           );
}


void* soap_new_ptr(struct soap *soap, void *value, int size)
{
    void* ptr = soap_malloc(soap, size);
    memcpy(ptr, value, size);

    return ptr;
}
