#ifndef SERVICECONTEXT_H
#define SERVICECONTEXT_H


#include "soapStub.h"
#include "soapH.h"
#include "eth_dev_param.h"
#include "hai_soap.h"
#include "toolbox-linkedlist.h"

#include <string.h>


typedef struct StreamProfile_S
{
  char name[HAIVISION_MAX_ONVIF_TOKEN];
  int width;
  int height;
  char url[HAIVISION_MAX_ONVIF_URL];
  int type;

  char str_err[HAIVISION_MAX_ONVIF_ERR_STRING];

} StreamProfile;


void StreamProfile_Constructor(StreamProfile * sp);
void StreamProfile_clear(StreamProfile * sp);

int StreamProfile_is_valid(const StreamProfile * sp);
char* StreamProfile_get_name(const StreamProfile * sp);
int StreamProfile_get_width (const StreamProfile * sp);
int StreamProfile_get_height(const StreamProfile * sp);
char* StreamProfile_get_url(const StreamProfile * sp);
int StreamProfile_get_type(const StreamProfile * sp);


struct tt__Profile* StreamProfile_get_profile(StreamProfile * sp, struct soap *soap);
struct tt__VideoSource* StreamProfile_get_video_src(StreamProfile * sp, struct soap *soap) ;

struct tt__VideoSourceConfiguration* get_video_src_cnf(StreamProfile * sp, struct soap *soap);
struct tt__VideoEncoderConfiguration* get_video_enc_cfg(StreamProfile * sp, struct soap *soap);


//methods for parsing opt from cmd
int StreamProfile_set_name(StreamProfile * sp, const char *new_val);
int StreamProfile_set_width(StreamProfile * sp, const char *new_val);
int StreamProfile_set_height(StreamProfile * sp, const char *new_val);
int StreamProfile_set_url(StreamProfile * sp, const char *new_val);
int StreamProfile_set_type(StreamProfile * sp, const char *new_val);


char * StreamProfile_get_str_err(StreamProfile * sp);
const char* StreamProfile_get_cstr_err(StreamProfile * sp);



typedef struct StreamProfileMap_S {
  char* key;
  struct StreamProfileMap_S * next;
  StreamProfile * streamProfile;
} StreamProfileMap;

StreamProfileMap * StreamProfileMapFind(const StreamProfileMap * map, const char * findme);

// will return the new map item with allocated client object.
StreamProfileMap * StreamProfileMapAdd(StreamProfileMap * map, const char * newkey, struct soap *soap);

typedef struct ServiceContext_S
{
        int port;
        char user[HAIVISION_MAX_ONVIF_TOKEN];
        char password[HAIVISION_MAX_ONVIF_TOKEN];

        //Device Information
        char manufacturer[HAIVISION_MAX_ONVIF_TOKEN];
        char model[HAIVISION_MAX_ONVIF_TOKEN];
        char firmware_version[HAIVISION_MAX_ONVIF_TOKEN];
        char serial_number[HAIVISION_MAX_ONVIF_TOKEN];
        char hardware_id[HAIVISION_MAX_ONVIF_TOKEN];
        
        char str_err[HAIVISION_MAX_ONVIF_ERR_STRING];
        
        linkedList scopes; // of types char *
        linkedList eth_ifs; // of types EthDevParam for ethernet interfaces
        StreamProfileMap profiles; // of types char *, StreamProfile

} ServiceContext;

void ServiceContext_Constructor(ServiceContext * sc);

char * ServiceContext_getServerIpFromClientIp(ServiceContext * sc, uint32_t client_ip);
int ServiceContext_getXAddr(ServiceContext * sc, struct soap *soap, char * loadme, size_t loadme_size);

char * ServiceContext_get_str_err(ServiceContext * sc);

int ServiceContext_add_profile(ServiceContext * sc, const StreamProfile * profile, struct soap *soap);

char * ServiceContext_get_stream_uri(ServiceContext * sc, const char * profile_url, uint32_t client_ip);

const StreamProfileMap * ServiceContext_get_profiles(ServiceContext * sc);

// service capabilities
struct tds__DeviceServiceCapabilities* ServiceContext_getDeviceServiceCapabilities(ServiceContext * sc, struct soap* soap);
struct trt__Capabilities*  ServiceContext_getMediaServiceCapabilities(ServiceContext * sc, struct soap* soap);
//        timg__Capabilities* getImagingServiceCapabilities  (struct soap* soap);
//        trc__Capabilities*  getRecordingServiceCapabilities(struct soap* soap);
//        tse__Capabilities*  getSearchServiceCapabilities   (struct soap* soap);
//        trv__Capabilities*  getReceiverServiceCapabilities (struct soap* soap);
//        trp__Capabilities*  getReplayServiceCapabilities   (struct soap* soap);
//        tev__Capabilities*  getEventServiceCapabilities    (struct soap* soap);
//        tls__Capabilities*  getDisplayServiceCapabilities  (struct soap* soap);
//        tmd__Capabilities*  getDeviceIOServiceCapabilities (struct soap* soap);


void* soap_new_ptr(struct soap *soap, void *value, int size);





#endif // SERVICECONTEXT_H
