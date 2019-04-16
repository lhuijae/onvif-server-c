/*
 --------------------------------------------------------------------------
 ServiceMedia.cpp
 
 Implementation of functions (methods) for the service:
 ONVIF media.wsdl server side
-----------------------------------------------------------------------------
*/


//#include "soapMediaBindingService.h"
#include "ServiceContext.h"
#include "hai_soap.h"
#include "smacros.h"



int __trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);

    ServiceContext* sc = (ServiceContext*)soap->user;
    trt__GetServiceCapabilitiesResponse->Capabilities = ServiceContext_getMediaServiceCapabilities(sc, soap);
    
    return SOAP_OK;
}



int __trt__GetVideoSources(struct soap* soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    ServiceContext *sc = (ServiceContext*)soap->user;
    StreamProfileMap * it = ServiceContext_get_profiles(sc);
    StreamProfileMap * first = it;
    
    int total_profiles = 0;    
    // count amount
    while (it && it->key)
    {
        total_profiles++;
        it = it->next;
    }

    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s total profiles %d\n", __FUNCTION__, total_profiles);
    printf("Media: %s total profiles %d\n", __FUNCTION__, total_profiles);
    
    if (total_profiles == 0)
    {
       trt__GetVideoSourcesResponse->__sizeVideoSources = 0;
       return SOAP_FAULT;
    }
    
    trt__GetVideoSourcesResponse->VideoSources = hai_soap_malloc(soap, sizeof(struct tt__VideoSource )*total_profiles);
    trt__GetVideoSourcesResponse->__sizeVideoSources = total_profiles;
    
    it = first;
    int i = 0;
    while (it && it->key)
    {
        printf("Media::%s: Will add profile ptr %0xX with name %s\n", __FUNCTION__, (unsigned int)it->streamProfile, it->streamProfile->name);
        struct tt__Profile* newProfile = StreamProfile_get_profile(it->streamProfile, soap);
        
        trt__GetVideoSourcesResponse->VideoSources[i].Framerate = 30;
        trt__GetVideoSourcesResponse->VideoSources[i].Resolution = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution));
        trt__GetVideoSourcesResponse->VideoSources[i].Resolution->Height = 720;
        trt__GetVideoSourcesResponse->VideoSources[i].Resolution->Width = 1280;
        trt__GetVideoSourcesResponse->VideoSources[i].token = hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
        strcpy(trt__GetVideoSourcesResponse->VideoSources[i].token, newProfile->Name);
        strcat(trt__GetVideoSourcesResponse->VideoSources[i].token, "_video_source");

        trt__GetVideoSourcesResponse->VideoSources[i].Imaging = NULL;/*hai_soap_malloc(soap, sizeof(struct tt__ImagingSettings));
        memset(trt__GetVideoSourcesResponse->VideoSources[i].Imaging, 0, sizeof(struct tt__ImagingSettings));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Brightness = (float*) hai_soap_malloc(soap,sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Brightness[0] = 128;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->ColorSaturation = (float*) hai_soap_malloc(soap,sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->ColorSaturation[0] = 128;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Contrast = (float*) hai_soap_malloc(soap,sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Contrast[0] = 128;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->IrCutFilter = (int *) hai_soap_malloc(soap,sizeof(int));
        *trt__GetVideoSourcesResponse->VideoSources[i].Imaging->IrCutFilter = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Sharpness = (float*) hai_soap_malloc(soap,sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Sharpness[0] = 128;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->BacklightCompensation = (struct tt__BacklightCompensation*) hai_soap_malloc(soap, sizeof(struct tt__BacklightCompensation));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->BacklightCompensation->Mode = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->BacklightCompensation->Level = 20;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Exposure = NULL;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Focus = NULL;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WideDynamicRange = (struct tt__WideDynamicRange*) hai_soap_malloc(soap, sizeof(struct tt__WideDynamicRange));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WideDynamicRange->Mode = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WideDynamicRange->Level = 20;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WhiteBalance = (struct tt__WhiteBalance*) hai_soap_malloc(soap, sizeof(struct tt__WhiteBalance));
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WhiteBalance->Mode = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WhiteBalance->CrGain = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->WhiteBalance->CbGain = 0;
        trt__GetVideoSourcesResponse->VideoSources[i].Imaging->Extension = NULL;*/
        
        trt__GetVideoSourcesResponse->VideoSources[i].Extension = NULL;
          
        //soap_dealloc(soap, newProfile);
        it = it->next;
        i++;
    }
    
    return SOAP_OK;
}



int __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s get profile:%s\n", __FUNCTION__, trt__GetProfile->ProfileToken);

    int ret = SOAP_FAULT;

    ServiceContext *sc = (ServiceContext*)soap->user;
    const StreamProfileMap *streamProfileMap = ServiceContext_get_profiles(sc);
    StreamProfileMap * it = StreamProfileMapFind(streamProfileMap, trt__GetProfile->ProfileToken);
    printf("Stream profile map(0x%X)\n", (unsigned int)streamProfileMap);
    
    if (it == NULL)
    {
      printf("FAILED TO FIND PROFILE!!!!!!!!\n");
      printf("FAILED TO FIND PROFILE!!!!!!!!\n");
    }

    if (it && it->key)
    {
        printf("Media::%s: Will add profile ptr %0xX with key %s name %s\n", __FUNCTION__, (unsigned int)it->streamProfile, it->key, it->streamProfile->name);
        trt__GetProfileResponse->Profile = StreamProfile_get_profile(it->streamProfile, soap);
        ret = SOAP_OK;
    }

    return ret;
}



int __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);

    ServiceContext *sc = (ServiceContext*)soap->user;
    const StreamProfileMap *streamProfileMap = ServiceContext_get_profiles(sc);
    StreamProfileMap * it = streamProfileMap;
    StreamProfileMap * first = it;
    
    int total_profiles = 0;    
    // count amount
    while (it && it->key)
    {
        total_profiles++;
        it = it->next;
    }
    
    if (total_profiles == 0)
    {
       trt__GetProfilesResponse->__sizeProfiles = 0;
       return SOAP_FAULT;
    }
    
    printf("total profiles %d\n", total_profiles);
    
    trt__GetProfilesResponse->Profiles = hai_soap_malloc(soap, sizeof(struct tt__Profile)*total_profiles);
    trt__GetProfilesResponse->__sizeProfiles = total_profiles;
    
    it = first;
    while (it && it->key)
    {
        printf("Will add profile ptr %0xX with name %s\n", (unsigned int)it->streamProfile, it->streamProfile->name);
        struct tt__Profile* newProfile = StreamProfile_get_profile(it->streamProfile, soap);
        memcpy(&trt__GetProfilesResponse->Profiles[0], newProfile, sizeof(struct tt__Profile));
        printf("new profile after memcpy name %s\n", trt__GetProfilesResponse->Profiles[0].Name);
        soap_dealloc(soap, newProfile);
        it = it->next;
    }

printf("end profile func\n");
    return SOAP_OK;
}



int __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    
    trt__GetVideoEncoderConfigurationsResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationsResponse));
    
    trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
    trt__GetVideoEncoderConfigurationsResponse->Configurations = hai_soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration)*trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations);
    struct tt__VideoEncoderConfiguration *config = &trt__GetVideoEncoderConfigurationsResponse->Configurations[0];
    config->Name = strdup("h264_high");
    config->token = strdup("h264_high");
    config->Encoding = tt__VideoEncoding__H264;
    config->Resolution = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution));
    config->Resolution[0].Width = 1280;
    config->Resolution[0].Height = 720; // FRANK FIXME, use from system
    config->Quality = 100.0;
    config->RateControl = hai_soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    config->RateControl->FrameRateLimit = 60;
    config->RateControl->EncodingInterval = 1; // 1 means all frames are encoded
    config->RateControl->BitrateLimit = 100000; // FRANK FIXME: get from network "Total TX Bandwidth Limit"
    config->H264 = hai_soap_malloc(soap, sizeof(struct tt__H264Configuration));
    config->H264->GovLength = 100; // gop
    config->H264->H264Profile = tt__H264Profile__Main; // FIXME GET FORM ENC
    config->Multicast = hai_soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    config->Multicast->Address = hai_soap_malloc(soap, sizeof(struct tt__IPAddress));
    config->Multicast->Address->Type = tt__IPType__IPv4;
    config->Multicast->Address->IPv4Address = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_IPV4_LENGTH);
    strcpy(config->Multicast->Address->IPv4Address, "0.0.0.0");//"224.65.11.199"); // FIXME use actual multicast if configured as such
    config->Multicast->Address->IPv6Address = NULL;
    config->Multicast->Port = 0; // FIXME correct port.
    config->Multicast->TTL = 60;
    config->Multicast->AutoStart = 0;
    config->Multicast->__size = 0;
    config->Multicast->__any = NULL;
    return SOAP_OK;
}



int __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}


int __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    ServiceContext *sc = (ServiceContext*)soap->user;
    char * request_token = trt__GetVideoSourceConfiguration->ConfigurationToken;
    StreamProfileMap * streamProfileMap = StreamProfileMapFind(&sc->profiles, request_token);
    if (streamProfileMap == NULL)
    {       
       printf("ERROR: Could not find profile from token(%s)\n", request_token);
       printf("ERROR: Could not find profile from token(%s)\n", request_token);
       printf("ERROR: Could not find profile from token(%s)\n", request_token);
       printf("ERROR: Could not find profile from token(%s)\n", request_token);
       printf("ERROR: Could not find profile from token(%s)\n", request_token);
       return SOAP_ERR;
    }
    
    StreamProfile *sp = streamProfileMap->streamProfile;
    
    trt__GetVideoSourceConfigurationResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetVideoSourceConfigurationResponse));
    struct tt__VideoSourceConfiguration * vSourceCfg = hai_soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    trt__GetVideoSourceConfigurationResponse->Configuration = vSourceCfg;
    trt__GetVideoSourceConfigurationResponse->Configuration->Name = hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    trt__GetVideoSourceConfigurationResponse->Configuration->token = hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = hai_soap_malloc(soap, sizeof(char)*HAIVISION_MAX_ONVIF_TOKEN);
    trt__GetVideoSourceConfigurationResponse->Configuration->__size = 0;
    trt__GetVideoSourceConfigurationResponse->Configuration->__any = NULL;
    
    if (sp->name[0] == '\0')
    {
       printf("ERROR: Cannot send empty token response\n");
       return SOAP_ERR;
    }    
            
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s profile token %s\n", __FUNCTION__, sp->name);
    strcpy(vSourceCfg->Name, sp->name);
    strcpy(vSourceCfg->token, sp->name);
    strcpy(vSourceCfg->SourceToken, sp->name); 
    vSourceCfg->UseCount = 44;
    vSourceCfg->Bounds->x = 0;
    vSourceCfg->Bounds->y = 0;
    vSourceCfg->Bounds->height = 720;
    vSourceCfg->Bounds->width = 1280;
    return SOAP_OK;
}



int __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s X\n", __FUNCTION__);
        
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Will return data\n");
    trt__GetVideoEncoderConfigurationResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationResponse));
    
    trt__GetVideoEncoderConfigurationResponse->Configuration = hai_soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    struct tt__VideoEncoderConfiguration *config = trt__GetVideoEncoderConfigurationResponse->Configuration;
    config->Name = strdup("h264_high");
    config->token = strdup("h264_high");
    config->Encoding = tt__VideoEncoding__H264;
    config->Resolution = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution));
    config->Resolution[0].Width = 1280;
    config->Resolution[0].Height = 720; // FRANK FIXME, use from system
    config->Quality = 100.0;
    config->RateControl = hai_soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    config->RateControl->FrameRateLimit = 60;
    config->RateControl->EncodingInterval = 1; // 1 means all frames are encoded
    config->RateControl->BitrateLimit = 100000; // FRANK FIXME: get from network "Total TX Bandwidth Limit"
    config->H264 = hai_soap_malloc(soap, sizeof(struct tt__H264Configuration));
    config->H264->GovLength = 100; // gop
    config->H264->H264Profile = tt__H264Profile__Main; // FIXME GET FORM ENC
    config->Multicast = hai_soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    config->Multicast->Address = hai_soap_malloc(soap, sizeof(struct tt__IPAddress));
    config->Multicast->Address->Type = tt__IPType__IPv4;
    config->Multicast->Address->IPv4Address = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_IPV4_LENGTH);
    strcpy(config->Multicast->Address->IPv4Address, "0.0.0.0");//"224.65.11.199"); // FIXME use actual multicast if configured as such
    config->Multicast->Address->IPv6Address = NULL;
    config->Multicast->Port = 0; // FIXME correct port.
    config->Multicast->TTL = 60;
    config->Multicast->AutoStart = 0;
    config->Multicast->__size = 0;
    config->Multicast->__any = NULL;
    
    return SOAP_OK;
}



int __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int  __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    
    return SOAP_OK;
}



int __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse);__trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s WITH FULL OPTIONS 2\n", __FUNCTION__);
    trt__GetVideoEncoderConfigurationOptionsResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetVideoEncoderConfigurationOptionsResponse));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options = hai_soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));

    trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange = hai_soap_malloc(soap, sizeof(struct tt__IntRange));
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 0;    
    trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 100;
    
     struct tt__H264Options * h264 = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 = hai_soap_malloc(soap, sizeof(struct tt__H264Options));
     h264->__sizeResolutionsAvailable = 1;
     h264->ResolutionsAvailable = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution)*h264->__sizeResolutionsAvailable);
     h264->ResolutionsAvailable[0].Width = 1280;
     h264->ResolutionsAvailable[0].Height = 720; // FRANK FIXME, use from system

     h264->GovLengthRange = hai_soap_malloc(soap, sizeof(struct tt__IntRange));
     h264->GovLengthRange->Min = 0;
     h264->GovLengthRange->Max = 1000;
     
     h264->FrameRateRange = hai_soap_malloc(soap, sizeof(struct tt__IntRange));
     h264->FrameRateRange->Min = 1;
     h264->FrameRateRange->Max = 60; // Could be 30 based on INPUT FRANK FIXME
     
     h264->EncodingIntervalRange = hai_soap_malloc(soap, sizeof(struct tt__IntRange));
     h264->EncodingIntervalRange->Min = 1;
     h264->EncodingIntervalRange->Max = 1; // An encoding interval value of "1" means that all frames are encoded. 
     
     h264->__sizeH264ProfilesSupported = 2;
     h264->H264ProfilesSupported = hai_soap_malloc(soap, sizeof(struct tt__VideoResolution)*h264->__sizeH264ProfilesSupported);
     h264->H264ProfilesSupported[0] = tt__H264Profile__Baseline;
     h264->H264ProfilesSupported[1] = tt__H264Profile__Main;

    return 501;
}



int __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    trt__GetAudioSourceConfigurationOptionsResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetAudioSourceConfigurationOptionsResponse));
    return SOAP_OK;
}



int __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    trt__GetAudioEncoderConfigurationOptionsResponse = hai_soap_malloc(soap, sizeof(struct _trt__GetAudioEncoderConfigurationOptionsResponse));
    return SOAP_OK;
}



int __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s   for profile:%s\n", __FUNCTION__, trt__GetStreamUri->ProfileToken);

    int ret = SOAP_FAULT;

    ServiceContext *sc = (ServiceContext*)soap->user;
    const StreamProfileMap *streamProfileMap = ServiceContext_get_profiles(sc);
    StreamProfileMap * it = &sc->profiles;

    if (it && it->key)
    {
        printf("Media::%s: Will add profile ptr %0xX with name %s\n", __FUNCTION__, (unsigned int)it->streamProfile, it->streamProfile->name);
        struct tt__Profile* profile = StreamProfile_get_profile(it->streamProfile, soap);
        trt__GetStreamUriResponse->MediaUri = hai_soap_malloc(soap, sizeof(struct tt__MediaUri));
        const char * url = StreamProfile_get_url(it->streamProfile);
        trt__GetStreamUriResponse->MediaUri->Uri = ServiceContext_get_stream_uri(sc, url, htonl(soap->ip));
        
        printf("Media::%s: From soap->ip %s %s\n", __FUNCTION__, url, trt__GetStreamUriResponse->MediaUri->Uri);
        
        printf("Media::%s: Uri %s\n", __FUNCTION__, trt__GetStreamUriResponse->MediaUri->Uri);
        
        ret = SOAP_OK;
    }


    return ret;
}



int __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetVideoSourceModes(struct soap *soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetVideoSourceMode(struct soap *soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}



int __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse)
{
    DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Media: %s\n", __FUNCTION__);
    return SOAP_OK;
}
