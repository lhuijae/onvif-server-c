#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>


#include "sessmgr_types.h"
#include "dbgtrace.h"

#include "daemon.h"
#include "smacros.h"
#include "ServiceContext.h"
#include "hai_soap.h"
#include "toolbox.h"


HDBG g_hDbg;

// ---- gsoap ----
#include "DeviceBinding.nsmap"
// C++ only #include "soapDeviceBindingService.h"
// C++ only #include "soapMediaBindingService.h"




static const char *help_str =
        " ===============  Help  ===============\n"
        " Daemon name:  " DAEMON_NAME          "\n"
        " Daemon  ver:  " DAEMON_VERSION_STR   "\n"
#ifdef  DEBUG
        " Build  mode:  debug\n"
#else
        " Build  mode:  release\n"
#endif
        " Build  date:  " __DATE__ "\n"
        " Build  time:  " __TIME__ "\n\n"
        "Options:                      description:\n\n"
        "       --no_chdir             Don't change the directory to '/'\n"
        "       --no_fork              Don't do fork\n"
        "       --no_close             Don't close standart IO files\n"
        "       --pid_file     [value] Set pid file name\n"
        "       --log_file     [value] Set log file name\n\n"
        "       --port         [value] Set socket port for Services   (default = 1000)\n"
        "       --user         [value] Set user name for Services     (default = admin)\n"
        "       --password     [value] Set user password for Services (default = admin)\n"
        "       --model        [value] Set model device for Services  (default = Model)\n"
        "       --scope        [value] Set scope for Services         (default don't set)\n"
        "       --ifs          [value] Set Net interfaces for work    (default don't set)\n"
        "       --hardware_id  [value] Set Hardware ID of device      (default = HardwareID)\n"
        "       --serial_num   [value] Set Serial number of device    (default = SerialNumber)\n"
        "       --firmware_ver [value] Set firmware version of device (default = FirmwareVersion)\n"
        "       --manufacturer [value] Set manufacturer for Services  (default = Manufacturer)\n\n"
        "       --name         [value] Set Name for Profile Media Services\n"
        "       --width        [value] Set Width for Profile Media Services\n"
        "       --height       [value] Set Height for Profile Media Services\n"
        "       --url          [value] Set URL (or template URL) for Profile Media Services\n"
        "                              in template mode %s will be changed to IP of interfase (see opt ifs)\n"
        "       --type         [value] Set Type for Profile Media Services (JPEG|MPEG4|H264)\n"
        "                              It is also a sign of the end of the profile parameters\n\n"
        "  -v,  --version              Display daemon version\n"
        "  -h,  --help                 Display this help\n\n";


int g_endrun = 0; // signal program termination
int g_shutdown_now = 0;
ConfigIniManager g_configIniManager; // nightflight.ini manager

#if DO_BENCHMARK == 1
unsigned int g_timestamp;
#endif

// VARIABLES FROM RTSP SERVER .ini file:
char g_local_ip[32] = "";
char g_subnet[32] = "";
char g_public_ip[HAIVISION_MAX_ONVIF_IPV4_LENGTH] = "";
unsigned short g_rtspServerPort = 554;
int g_tcpIpTimeout = 30;
char g_hostname[128] = "";
int g_multicast_enable_0 = 0;
char g_multicast_stream_ip_0[32]= "";
int g_multicast_stream_port_0 = 1400;
int g_multicast_enable_1 = 0;
char g_multicast_stream_ip_1[32]= "";
int g_multicast_stream_port_1 = 1410;
int g_multicast_enable_2 = 0;
char g_multicast_stream_ip_2[32]= "";
int g_multicast_stream_port_2 = 1420;
int g_mtuSize = 0;

// indexes for long_opt function

    enum LongOpts
    {
        LongOpts_version = 'v',
        LongOpts_help    = 'h',

        //daemon options
        LongOpts_no_chdir = 1,
        LongOpts_no_fork,
        LongOpts_no_close,
        LongOpts_pid_file,
        LongOpts_log_file,

        //ONVIF Service options (context)
        LongOpts_port,
        LongOpts_user,
        LongOpts_password,
        LongOpts_manufacturer,
        LongOpts_model,
        LongOpts_firmware_ver,
        LongOpts_serial_num,
        LongOpts_hardware_id,
        LongOpts_scope,
        LongOpts_ifs,

        //Media Profile for ONVIF Media Service
        LongOpts_name,
        LongOpts_width,
        LongOpts_height,
        LongOpts_url,
        LongOpts_type
    };



static const char *short_opts = "hv";


static const struct option long_opts[] =
{
    { "version",      no_argument,       NULL, LongOpts_version       },
    { "help",         no_argument,       NULL, LongOpts_help          },

    //daemon options
    { "no_chdir",     no_argument,       NULL, LongOpts_no_chdir      },
    { "no_fork",      no_argument,       NULL, LongOpts_no_fork       },
    { "no_close",     no_argument,       NULL, LongOpts_no_close      },
    { "pid_file",     required_argument, NULL, LongOpts_pid_file      },
    { "log_file",     required_argument, NULL, LongOpts_log_file      },

    //ONVIF Service options (context)
    { "port",         required_argument, NULL, LongOpts_port          },
    { "user",         required_argument, NULL, LongOpts_user          },
    { "password",     required_argument, NULL, LongOpts_password      },
    { "manufacturer", required_argument, NULL, LongOpts_manufacturer  },
    { "model",        required_argument, NULL, LongOpts_model         },
    { "firmware_ver", required_argument, NULL, LongOpts_firmware_ver  },
    { "serial_num",   required_argument, NULL, LongOpts_serial_num    },
    { "hardware_id",  required_argument, NULL, LongOpts_hardware_id   },
    { "scope",        required_argument, NULL, LongOpts_scope         },
    { "ifs",          required_argument, NULL, LongOpts_ifs           },

    //Media Profile for ONVIF Media Service
    { "name",          required_argument, NULL, LongOpts_name         },
    { "width",         required_argument, NULL, LongOpts_width        },
    { "height",        required_argument, NULL, LongOpts_height       },
    { "url",           required_argument, NULL, LongOpts_url          },
    { "type",          required_argument, NULL, LongOpts_type         },

    { NULL,           no_argument,       NULL,  0                      }
};





static struct soap *soap;

ServiceContext service_ctx;

void daemon_exit_handler(int sig)
{
    //Here we release resources

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);

    unlink(daemon_info.pid_file);

    exit(EXIT_SUCCESS); // good job (we interrupted (finished) main loop)
}



void init_signals(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = daemon_exit_handler;
    if( sigaction(SIGTERM, &sa, NULL) != 0 )
        daemon_error_exit("Can't set daemon_exit_handler: %m\n");


    signal(SIGCHLD, SIG_IGN); // ignore child
    signal(SIGTSTP, SIG_IGN); // ignore tty signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
}



void processing_cmd(int argc, char *argv[])
{
    int opt, ival;

    StreamProfile profile;
    StreamProfile_Constructor(&profile);

    while( (opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1 )
    {
        switch( opt )
        {

            case LongOpts_help:
                        puts(help_str);
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;

            case LongOpts_version:
                        puts(DAEMON_NAME "  version  " DAEMON_VERSION_STR "\n");
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;


                 //daemon options
            case LongOpts_no_chdir:
                        daemon_info.no_chdir = 1;
                        break;

            case LongOpts_no_fork:
                        daemon_info.no_fork = 1;
                        break;

            case LongOpts_no_close:
                        daemon_info.no_close_stdio = 1;
                        break;

            case LongOpts_pid_file:
                        daemon_info.pid_file = optarg;
                        break;

            case LongOpts_log_file:
                        daemon_info.log_file = optarg;
                        break;


            //ONVIF Service options (context)
            case LongOpts_port:
                        service_ctx.port = atoi(optarg);
                        break;

            case LongOpts_user:
                        strncpy(service_ctx.user, optarg, sizeof(service_ctx.user)-1);
                        break;

            case LongOpts_password:
                        strncpy(service_ctx.password, optarg, sizeof(service_ctx.password)-1);
                        break;

            case LongOpts_manufacturer:
                        strncpy(service_ctx.manufacturer, optarg, sizeof(service_ctx.manufacturer)-1);
                        break;

            case LongOpts_model:
                        strncpy(service_ctx.model, optarg, sizeof(service_ctx.model)-1);
                        break;

            case LongOpts_firmware_ver:
                        strncpy(service_ctx.firmware_version, optarg, sizeof(service_ctx.firmware_version)-1);
                        break;

            case LongOpts_serial_num:
                        strncpy(service_ctx.serial_number, optarg, sizeof(service_ctx.serial_number)-1);
                        break;

            case LongOpts_hardware_id:
                        strncpy(service_ctx.hardware_id, optarg, sizeof(service_ctx.hardware_id)-1);
                        break;

            case LongOpts_scope:
            printf("SET SCOPE\n");
                        linkedList_Push(&service_ctx.scopes, strdup(optarg));
                        break;

            case LongOpts_ifs:
            {
                        EthDevParam *ethDevParam = malloc(sizeof(EthDevParam));
                        EthDevParam_Constructor(ethDevParam);
                        ival = linkedList_Push(&service_ctx.eth_ifs, ethDevParam);
                        printf("now ifs has %d itemAmount with ival %d\n", service_ctx.eth_ifs.itemAmount, ival);

                        if (EthDevParam_open(ethDevParam, optarg) != 0 )
                            daemon_error_exit("Can't open ethernet interface: %s - %m\n", optarg);

                        // global ip
                        ethDevParam = malloc(sizeof(EthDevParam));
                        EthDevParam_Constructor(ethDevParam);
                        ival = linkedList_Push(&service_ctx.eth_ifs, ethDevParam);
                        printf("now ifs has %d itemAmount with ival %d\n", service_ctx.eth_ifs.itemAmount, ival);

                        if (EthDevParam_open(ethDevParam, optarg) != 0 )
                            daemon_error_exit("Can't open ethernet interface: %s - %m\n", optarg);

                        break;
            }

            //Media Profile for ONVIF Media Service
            case LongOpts_name:
            printf("SET NAME\n");
                        if( !StreamProfile_set_name(&profile, optarg) )
                            daemon_error_exit("Can't set name for Profile: %s\n", StreamProfile_get_cstr_err(&profile));

                        break;


            case LongOpts_width:
                        if( !StreamProfile_set_width(&profile, optarg) )
                            daemon_error_exit("Can't set width for Profile: %s\n", StreamProfile_get_cstr_err(&profile));

                        break;


            case LongOpts_height:
                        if( !StreamProfile_set_height(&profile, optarg) )
                            daemon_error_exit("Can't set height for Profile: %s\n", StreamProfile_get_cstr_err(&profile));

                        break;


            case LongOpts_url:
            printf("SET URL\n");
                        if( !StreamProfile_set_url(&profile, optarg) )
                            daemon_error_exit("Can't set URL for Profile: %s\n", StreamProfile_get_cstr_err(&profile));

                        break;


            case LongOpts_type:
            printf("SET TYPE\n");
                        if( !StreamProfile_set_type(&profile, optarg) )
                            daemon_error_exit("Can't set type for Profile: %s\n", StreamProfile_get_cstr_err(&profile));

                        if( !ServiceContext_add_profile(&service_ctx, &profile, soap) )
                            daemon_error_exit("Can't add Profile: %s\n", ServiceContext_get_str_err(&service_ctx));
                            
                        printf("Init: Added new type and profile, first key %s, name %s\n", service_ctx.profiles.key, service_ctx.profiles.streamProfile->name);
                        printf("Init: Stream profile map addr(0x%X)\n", (unsigned int)&service_ctx.profiles);
                        StreamProfile_clear(&profile); //now we can add new profile (just uses one variable)

                        break;


            default:
                        puts("for more detail see help\n\n");
                        exit_if_not_daemonized(EXIT_FAILURE);
                        break;
        }
    }
}



void check_service_ctx(void)
{
    if (service_ctx.eth_ifs.itemAmount == 0)
        daemon_error_exit("Error: not set no one ehternet interface more details see opt --ifs\n");


    if (service_ctx.scopes.itemAmount == 0)
        daemon_error_exit("Error: not set scopes more details see opt --scope\n");
        
    printf("Scope amounts %d\n", service_ctx.scopes.itemAmount);

    const StreamProfileMap * streamProfileMap = ServiceContext_get_profiles(&service_ctx);
    if (streamProfileMap->key == NULL)
        daemon_error_exit("Error: not set no one profile more details see --help\n");
}



void init_gsoap(void)
{
    soap = soap_new1(SOAP_XML_INDENT);

    if(!soap)
        daemon_error_exit("Can't get mem for SOAP\n");

    soap->bind_flags = SO_REUSEADDR;

    if( !soap_valid_socket(soap_bind(soap, NULL, service_ctx.port, 10)) )
    {
        soap_print_fault(soap, stderr);
        printf("soap fault");
        exit(EXIT_FAILURE);
    }

    soap->send_timeout = 5; // timeout in sec
    soap->recv_timeout = 5; // timeout in sec


    //save pointer of service_ctx in soap
    soap->user = (void*)&service_ctx;
}



void init_makito()
{
   int ival, a; 
   char temp[256];
   
   temp[sizeof(temp)-1] = '\0';
   
   DBGINIT("ONVIF", &g_hDbg);
    
#if defined(_MSC_VER)
   ival = ConfigIniManager_Constructor(&g_configIniManager, "./rtsp.ini");
   #else
ival = ConfigIniManager_Constructor(&g_configIniManager, configDirectory "/rtsp.ini");
   #endif
   
   ival = ConfigIniManager_GetChar(&g_configIniManager, g_public_ip, "public_ip");
   
  printf("Reading global ip g_public_ip %s\n", g_public_ip);
   ival = ConfigIniManager_GetChar(&g_configIniManager, temp, "multicast_enable_0");
   if ( ival > 0 && temp[0] != '\0' )
   {
   	g_multicast_enable_0 = atoi(temp);
   }
   
   ival = ConfigIniManager_GetChar(&g_configIniManager, temp, "multicast_stream_ip_0");
   if ( ival > 0 && temp[0] != '\0' )
   {
   	strncpy(g_multicast_stream_ip_0, temp, 31);
   }
   
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "multicast_stream_port_0");
   if ( ival > 0 && a > 0 )
   {
   	g_multicast_stream_port_0 = a;
   }
   
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "multicast_enable_1");
   if ( ival > 0 && a > 0 )
   {
   	g_multicast_enable_1 = a;
   }
   
   ival = ConfigIniManager_GetChar(&g_configIniManager, temp, "multicast_stream_ip_1");
   if ( ival > 0 && temp[0] != '\0' )
   {
   	strncpy(g_multicast_stream_ip_1, temp, 31);
   }
   
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "multicast_stream_port_1");
   if ( ival > 0 && a > 0 )
   {
   	g_multicast_stream_port_1 = a;
   }
   
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "multicast_enable_2");
   if ( ival > 0 && a > 0 )
   {
   	g_multicast_enable_2 = a;
   }
   
   ival = ConfigIniManager_GetChar(&g_configIniManager, temp, "multicast_stream_ip_2");
   if ( ival > 0 && temp[0] != '\0' )
   {
   	strncpy(g_multicast_stream_ip_2, temp, 31);
   }
   
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "multicast_stream_port_2");
   if ( ival > 0 && a > 0 )
   {
   	g_multicast_stream_port_2 = a;
   }
   
   a = 0;
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "mtu_size");
   if ( ival > 0 )
   {
   	g_mtuSize = a;
   }
   
   a = 0;
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "server_port");
   if ( ival > 0 )
   {
   	g_rtspServerPort = a;
   	if ( a <= 0 )
   	{
   		g_rtspServerPort = 554;
   	}
   }
   
   a = 0;
   ival = ConfigIniManager_GetInteger(&g_configIniManager, &a, "tcpip_timeout_sec");
   if ( ival > 0 )
   {
   	g_tcpIpTimeout = a;
   	if ( a <= 0 )
   	{
   		g_tcpIpTimeout = 30;
   	}
   }
   
   hai_session_mngr_init();
}

void init_soap(void *data)
{
   int ival, a; 
   char temp[256];
   
   init_signals();
   check_service_ctx();
   init_gsoap();
   
}
/*
static int plugin_send(struct soap *soap, const char *buf, size_t len)  
{ 
  struct plugin_data *data = (struct plugin_data*)soap_lookup_plugin(soap, plugin_id);

  fwrite(buf, len, 1, stderr); //<--  pick up the xml from here!

  return data->fsend(soap, buf, len); // pass data on to old send callback 
}  

static size_t plugin_recv(struct soap *soap, char *buf, size_t len)  
{ 
  struct plugin_data *data = (struct plugin_data*)soap_lookup_plugin(soap, plugin_id);  
  size_t res = data->frecv(soap, buf, len); // get data from old recv callback 

  fwrite(buf, res, 1, stderr); //<--  pick up the xml from here!

  return res;  
}  */

int main(int argc, char *argv[])
{
    init_makito();
    ServiceContext_Constructor(&service_ctx);
    processing_cmd(argc, argv);
#if 0
    daemonize2(init_soap, NULL);
#else
    init_soap(NULL);
#endif
    
   // TODO init service_ctx

    soap_set_namespaces(soap, namespaces);
//    FOREACH_SERVICE(DECLARE_SERVICE, soap)

    while (1)
    {
        // wait new client
        if( !soap_valid_socket(soap_accept(soap)) )
        {
            soap_print_fault(soap, stderr);
            return EXIT_FAILURE;
        }


        // process service
        if (soap_serve(soap))
        {
        printf("FAULT:\n");
           soap_print_fault(soap, stderr);
        }
        //FOREACH_SERVICE(DISPATCH_SERVICE, soap)
        //else
        //{
           DBGPRINTEX(g_hDbg, DBG_LEVEL_INFO, "after serve\n");
        //}
    }


    return EXIT_SUCCESS; // good job (we interrupted (finished) main loop)
}


/******************************************************************************\
 *
 *	Relayed SOAP-ENV:Fault Handler for FaultTo Server
 *
\******************************************************************************/

int SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
  printf("Received Fault:\n");
  /* populate the fault struct from the operation arguments to print it */
  soap_fault(soap);
  /* SOAP 1.1 */
  soap->fault->faultcode = faultcode;
  soap->fault->faultstring = faultstring;
  soap->fault->faultactor = faultactor;
  soap->fault->detail = detail;
  /* SOAP 1.2 */
  soap->fault->SOAP_ENV__Code = SOAP_ENV__Code;
  soap->fault->SOAP_ENV__Reason = SOAP_ENV__Reason;
  soap->fault->SOAP_ENV__Node = SOAP_ENV__Node;
  soap->fault->SOAP_ENV__Role = SOAP_ENV__Role;
  soap->fault->SOAP_ENV__Detail = SOAP_ENV__Detail;
  /* set error */
  soap->error = SOAP_FAULT;
  soap_print_fault(soap, stdout);
  return soap_send_empty_response(soap, SOAP_OK); /* HTTP 202 Accepted */
}
