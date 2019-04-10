/*
 * eth_dev_param.cpp
 *
 *
 * version 1.0
 *
 *
 * Copyright (c) 2015, Koynov Stas - skojnov@yandex.ru
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#if defined(_MSC_VER)
#include "unistd-windows.h"
#else
#include <unistd.h>
#endif
#include <string.h>

#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "eth_dev_param.h"

#include "dbgtrace.h"

extern HDBG g_hDbg;

static int run_shell_cmd(const char *cmd)
{
    FILE *ptr;

    if(!cmd)
        return -1;


    ptr = popen(cmd, "w");
    if(!ptr)
        return -1;


    if(pclose(ptr)<0)
        return -1;


    return 0;  //good job
}


void EthDevParam_Constructor(EthDevParam *ethDevParam)
{
    memset(ethDevParam, 0, sizeof(*ethDevParam));
    ethDevParam->sd = -1;
    ethDevParam->opened = 0;
}



void EthDevParam_Destructor(EthDevParam *ethDevParam)
{
}


int EthDevParam_is_open(EthDevParam *ethDevParam)
{ 
  return ethDevParam->opened; 
}


const char *EthDevParam_dev_name(EthDevParam *ethDevParam)
{ 
   return ethDevParam->ifr.ifr_name;
}


int EthDevParam_open(EthDevParam *ethDevParam, const char *dev_name)
{
    EthDevParam_close(ethDevParam); //close old socket

    if ( !dev_name || (strlen(dev_name) >= IFNAMSIZ) )
        return -1;


    ethDevParam->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (ethDevParam->sd == -1)
        return -1;


    strncpy(ethDevParam->ifr.ifr_name, dev_name, IFNAMSIZ);
    DBGPRINTEX(g_hDbg, DBG_LEVEL_ERROR, "EthDevParam_open got me %s\n", dev_name);

    // try get flags for interface
    if( ioctl(ethDevParam->sd, SIOCGIFFLAGS, &ethDevParam->ifr) == -1 )
    {
       EthDevParam_close(ethDevParam);
       return -1;     //can't get flags
    }

    ethDevParam->opened = 1;
    return 0; //good job
}



void EthDevParam_close(EthDevParam *ethDevParam)
{
    if (ethDevParam->sd != -1)
        close(ethDevParam->sd);

    memset(&ethDevParam->ifr, 0, sizeof(struct ifreq));
    ethDevParam->opened = 0;
    ethDevParam->sd     = -1;
}



int EthDevParam_set_ip_from_string(EthDevParam *ethDevParam, const char *IP)
{
    if (!EthDevParam_is_open(ethDevParam) || !IP)
        return -1;

    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    memset(addr, 0, sizeof( struct sockaddr_in) );
    addr->sin_family  = AF_INET;

    if( inet_pton(AF_INET, IP, &addr->sin_addr) <= 0 )
        return -1;


    return ioctl(ethDevParam->sd, SIOCSIFADDR, &ethDevParam->ifr);
}



int EthDevParam_set_ip(EthDevParam *ethDevParam, uint32_t IP)
{
    if (!EthDevParam_is_open(ethDevParam))
        return -1;


    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    memset(addr, 0, sizeof( struct sockaddr_in) );
    addr->sin_family      = AF_INET;
    addr->sin_addr.s_addr = IP;


    return ioctl(ethDevParam->sd, SIOCSIFADDR, &ethDevParam->ifr);
}



int EthDevParam_get_ip_from_string(EthDevParam *ethDevParam, char *IP)
{
    if (!EthDevParam_is_open(ethDevParam) || !IP)
        return -1;

    if( ioctl(ethDevParam->sd, SIOCGIFADDR, &ethDevParam->ifr) != 0 )
        return -1;

    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;


    if( inet_ntop(AF_INET, &addr->sin_addr, IP, INET_ADDRSTRLEN) != NULL )
        return 0; //good job


    return -1;
}



int EthDevParam_get_ip(EthDevParam *ethDevParam, uint32_t *IP)
{
    if ( !EthDevParam_is_open(ethDevParam) || !IP )
        return -1;


    if( ioctl(ethDevParam->sd, SIOCGIFDSTADDR, &ethDevParam->ifr) != 0 )
        return -1;

    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    *IP = addr->sin_addr.s_addr;


    return 0; //good job
}



int EthDevParam_set_mask_from_string(EthDevParam *ethDevParam, const char *mask)
{
    if (!EthDevParam_is_open(ethDevParam) || !mask)
        return -1;


    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    memset(addr, 0, sizeof( struct sockaddr_in) );
    addr->sin_family  = AF_INET;

    if( inet_pton(AF_INET, mask, &addr->sin_addr) <= 0 )
        return -1;


    return ioctl(ethDevParam->sd, SIOCSIFNETMASK, &ethDevParam->ifr);
}



int EthDevParam_set_mask(EthDevParam *ethDevParam, uint32_t mask)
{
    if (!EthDevParam_is_open(ethDevParam))
        return -1;


    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    memset(addr, 0, sizeof( struct sockaddr_in) );
    addr->sin_family      = AF_INET;
    addr->sin_addr.s_addr = mask;


    return ioctl(ethDevParam->sd, SIOCSIFNETMASK, &ethDevParam->ifr);
}



int EthDevParam_get_mask_from_string(EthDevParam *ethDevParam, char *mask)
{
    if (!EthDevParam_is_open(ethDevParam) || !mask)
        return -1;


    if( ioctl(ethDevParam->sd, SIOCGIFNETMASK, &ethDevParam->ifr) != 0 )
        return -1;

    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;


    if( inet_ntop(AF_INET, &addr->sin_addr, mask, INET_ADDRSTRLEN) != NULL )
        return 0; //good job


    return -1;
}



int EthDevParam_get_mask(EthDevParam *ethDevParam, uint32_t *mask)
{
    if (!EthDevParam_is_open(ethDevParam) || !mask)
        return -1;


    if (ioctl(ethDevParam->sd, SIOCGIFNETMASK, &ethDevParam->ifr) != 0)
        return -1;

    struct sockaddr_in* addr = (struct sockaddr_in*)&ethDevParam->ifr.ifr_addr;

    *mask = addr->sin_addr.s_addr;


    return 0; //good job
}



int EthDevParam_get_mask_prefix(EthDevParam *ethDevParam)
{
    int prefix = 0;
    uint32_t mask;
    EthDevParam_get_mask(ethDevParam, &mask);

    mask = ntohl(mask);

    while(mask != 0)
    {
        mask <<= 1;
        prefix++;
    }

    return prefix;
}



int EthDevParam_set_gateway_from_string(EthDevParam *ethDevParam, const char *gateway)
{
    if (!EthDevParam_is_open(ethDevParam) || !gateway)
        return -1;

    char cmd[64];

    sprintf(cmd, "route add %s %s", gateway, ethDevParam->ifr.ifr_name);

    return run_shell_cmd(cmd);
}



int EthDevParam_set_gateway(EthDevParam *ethDevParam, uint32_t gateway)
{
    if (!EthDevParam_is_open(ethDevParam))
        return -1;

    char cmd[64];

    struct in_addr in;
    in.s_addr = gateway;

    sprintf(cmd, "route add %s %s", inet_ntoa(in), ethDevParam->ifr.ifr_name);

    return run_shell_cmd(cmd);
}



int EthDevParam_get_gateway_from_string(EthDevParam *ethDevParam, char *gateway)
{
    if (!EthDevParam_is_open(ethDevParam) || !gateway )
        return -1;

    uint32_t tmp_gateway;

    struct sockaddr_in s_addr;

    if (EthDevParam_get_gateway(ethDevParam, &tmp_gateway) != 0)
        return -1;

    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family      = AF_INET;
    s_addr.sin_addr.s_addr = tmp_gateway;


    if( inet_ntop(AF_INET, &s_addr.sin_addr, gateway, INET_ADDRSTRLEN) != NULL )
        return 0; //good job
    else
        return -1;
}



int EthDevParam_get_gateway(EthDevParam *ethDevParam, uint32_t *gateway)
{
    if (!EthDevParam_is_open(ethDevParam) || !gateway)
        return -1;


    char devname[64];
    unsigned long d, g, m;
    int flgs, ref, use, metric, mtu, win, ir;


    FILE *fp = fopen("/proc/net/route", "r");

    if( !fp )
        return -1; //can't open file


    if( fscanf(fp, "%*[^\n]\n") < 0 ) // Skip the first line
    {
        fclose(fp);
        return -1;   // Empty or missing line, or read error
    }


    while( !feof(fp) )
    {
        int r;
        r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
                   devname, &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win, &ir);


        if (r != 11)
        {
            fclose(fp);
            return -1;
        }

        if( strcmp(devname, ethDevParam->ifr.ifr_name) != 0 )
            continue;

        fclose(fp);
        *gateway = g;

        return 0; //good job
    }


    fclose(fp);
    return -1; //can't finde
}



int EthDevParam_set_hwaddr_from_string(EthDevParam *ethDevParam, const char *hwaddr)
{

    if( !EthDevParam_is_open(ethDevParam) || !hwaddr || (strlen(hwaddr) != 17) )
        return -1;

    int i = 0;
    int tmp_mac[6]; // int for sscanf!!!


    sscanf(hwaddr, "%x:%x:%x:%x:%x:%x", &tmp_mac[0], &tmp_mac[1], &tmp_mac[2], &tmp_mac[3], &tmp_mac[4], &tmp_mac[5]);


    ethDevParam->ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    for(i=0; i < 6; i++)
        ethDevParam->ifr.ifr_hwaddr.sa_data[i] = tmp_mac[i];


    return ioctl(ethDevParam->sd, SIOCSIFHWADDR, &ethDevParam->ifr);
}



int EthDevParam_set_hwaddr(EthDevParam *ethDevParam, const uint8_t *hwaddr)
{
    if (!EthDevParam_is_open(ethDevParam) || !hwaddr)
        return -1;


    ethDevParam->ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy(ethDevParam->ifr.ifr_hwaddr.sa_data, hwaddr, 6);


    char cmd[32];

    sprintf(cmd, "ifconfig %s down", EthDevParam_dev_name(ethDevParam));
    run_shell_cmd(cmd);

    int ret = ioctl(ethDevParam->sd, SIOCSIFHWADDR, &ethDevParam->ifr);

    sprintf(cmd, "ifconfig %s up", EthDevParam_dev_name(ethDevParam));
    run_shell_cmd(cmd);


    return ret;
}



int EthDevParam_get_hwaddr_from_string(EthDevParam *ethDevParam, char *hwaddr)
{
    if (!EthDevParam_is_open(ethDevParam) || !hwaddr)
        return -1;


    if( ioctl(ethDevParam->sd, SIOCGIFHWADDR, &ethDevParam->ifr) != 0 )
        return -1;


    if( ethDevParam->ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER )
        return -1;


    uint8_t *tmp_mac = (uint8_t *)ethDevParam->ifr.ifr_hwaddr.sa_data;


    sprintf(hwaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
            tmp_mac[0], tmp_mac[1], tmp_mac[2], tmp_mac[3], tmp_mac[4], tmp_mac[5]);


    return 0; //good job
}



int EthDevParam_get_hwaddr(EthDevParam *ethDevParam, uint8_t *hwaddr)
{
    if (!EthDevParam_is_open(ethDevParam) || !hwaddr)
        return -1;


    if( ioctl(ethDevParam->sd, SIOCGIFHWADDR, &ethDevParam->ifr) != 0 )
        return -1;


    if( ethDevParam->ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER )
        return -1;


    memcpy(hwaddr, ethDevParam->ifr.ifr_hwaddr.sa_data, 6);


    return 0; //good job
}



