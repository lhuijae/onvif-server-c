

#include "soapStub.h"
#include "soapH.h"
#include "hai_soap.h"
#include "smacros.h"

#include "wsdd.nsmap"
#include "wsddapi.h"


int haiSoapProbe()
{
   struct soap* serv = soap_new1(SOAP_IO_UDP);
   if (!soap_valid_socket(soap_bind(serv, NULL, 0, 1000)))
   {
     soap_print_fault(serv, stderr);
     return -10;
   }
   int res = soap_wsdd_Probe(serv, 
                 SOAP_WSDD_ADHOC, 
                 SOAP_WSDD_TO_TS,
                 "soap.udp://239.255.255.250:3702",
                 soap_wsa_rand_uuid(serv), 
                 NULL, 
                 NULL, 
                 NULL, 
                 "");
   if (res != SOAP_OK)
   {
      soap_print_fault(serv, stderr);
      return -11;
   }
    soap_wsdd_listen(serv, 1);
    soap_destroy(serv);
     soap_end(serv);
    soap_done(serv);
    return 0;
}

int __wsdd__Hello(struct soap* soap, struct wsdd__HelloType *wsdd__Hello)
{

}

int __wsdd__Bye(struct soap* soap, struct wsdd__ByeType *wsdd__Bye)
{
}

int __wsdd__Probe(struct soap* soap, struct wsdd__ProbeType *wsdd__Probe)
{
   char _IPAddr[HAIVISION_MAX_ONVIF_IPV4_LENGTH];
   DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "Discovery: %s\n", __FUNCTION__);
    
   wsdd__ProbeMatchesType ProbeMatches;  
   ProbeMatches.__sizeProbeMatch = 1;
   ProbeMatches.ProbeMatch = (struct wsdd__ProbeMatchType *)hai_soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
   
   ProbeMatches.ProbeMatch->XAddrs = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_IPV4_LENGTH);  
   ProbeMatches.ProbeMatch->Types = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_TOKEN);
   ProbeMatches.ProbeMatch->Scopes = hai_soap_malloc(soap,sizeof(struct wsdd__ScopesType));  
   ProbeMatches.ProbeMatch->wsa5__EndpointReference.ReferenceParameters = hai_soap_malloc(soap,sizeof(struct wsa5__ReferenceParametersType));  
//   ProbeMatches.ProbeMatch->wsa5__EndpointReference.Metadata = hai_soap_malloc(soap,sizeof(struct wsa5__MetadataType));  
  ProbeMatches.ProbeMatch->wsa5__EndpointReference.__any = hai_soap_malloc(soap, sizeof(char*) * HAIVISION_SMALL_STRING_LENGTH);  
   ProbeMatches.ProbeMatch->wsa5__EndpointReference.__anyAttribute = hai_soap_malloc(soap, sizeof(char) * HAIVISION_SMALL_STRING_LENGTH);  
   ProbeMatches.ProbeMatch->wsa5__EndpointReference.Address = hai_soap_malloc(soap, sizeof(char) * HAIVISION_MAX_ONVIF_IPV4_LENGTH);  
   
   sprintf(_IPAddr, "http://%d.%d.%d.%d/onvif/device_service", 192, 168, 1, 233);  
   ProbeMatches.__sizeProbeMatch = 1;  
   ProbeMatches.ProbeMatch->Scopes->__item = hai_soap_malloc(soap, 1024);  
   memset(ProbeMatches.ProbeMatch->Scopes->__item, 0, sizeof(ProbeMatches.ProbeMatch->Scopes->__item));    
 
   strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter");  
    
   strcpy(ProbeMatches.ProbeMatch->XAddrs, _IPAddr);
   strcpy(ProbeMatches.ProbeMatch->Types, wsdd__Probe->Types);
   ProbeMatches.ProbeMatch->MetadataVersion = 1; 

   ProbeMatches.ProbeMatch->wsa5__EndpointReference.ReferenceParameters->__size = 0;  
   ProbeMatches.ProbeMatch->wsa5__EndpointReference.ReferenceParameters->__any = NULL;  
    //ws-discovery  
//    strcpy(ProbeMatches.ProbeMatch->wsa5__EndpointReference.PortType[0], "ttl");  
    ProbeMatches.ProbeMatch->wsa5__EndpointReference.__any[0] = (char *)soap_malloc(soap, sizeof(char) * HAIVISION_SMALL_STRING_LENGTH);  
    strcpy(ProbeMatches.ProbeMatch->wsa5__EndpointReference.__any[0], "Any");  
    strcpy(ProbeMatches.ProbeMatch->wsa5__EndpointReference.__anyAttribute, "Attribute");  
    ProbeMatches.ProbeMatch->wsa5__EndpointReference.__size = 0;  
    
    strcpy(ProbeMatches.ProbeMatch->wsa5__EndpointReference.Address, _IPAddr);   // frankl FIXME possibly wrong

    //soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";   
    //soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";   
    soap->header->wsa5__RelatesTo = hai_soap_malloc(soap, sizeof(struct wsa5__RelatesToType));  
    soap->header->wsa5__RelatesTo->__item = soap_strdup(soap, soap->header->wsa5__MessageID);
  
    soap->header->wsa5__MessageID = hai_soap_malloc(soap, sizeof(char) * HAIVISION_LONG_STRING_LENGTH);  
    strcpy(soap->header->wsa5__MessageID, "00-00-00-00-00-01"); /// frank FIXME
  
    /* send over current socket as HTTP OK response: */  
    soap_POST_send__wsdd__Probe(soap, "http://", &ProbeMatches);  
    /*
   struct wsdd__ProbeMatchesType *matches;
   soap_wsdd_init_ProbeMatches(soap, matches); 
   soap_wsdd_add_ProbeMatch(soap,matches, l_cameraId ,l_types, l_scopes,NULL, l_serviceAddr, 2); 
   if (soap_wsdd_ProbeMatches(soap,"soap.udp://",build_my_rand_uuid(soap, MessageID),MessageID,NULL,matches)!=SOAP_OK) 
     soap_print_fault(soap, stderr); */
     
   
   return SOAP_WSDD_ADHOC;
}

int __wsdd__ProbeMatches(struct soap* soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
}

int __wsdd__Resolve(struct soap* soap, struct wsdd__ResolveType *wsdd__Resolve)
{
}


int __wsdd__ResolveMatches(struct soap* soap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
}
