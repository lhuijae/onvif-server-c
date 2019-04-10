/*
 * eth_dev_param.h
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
 *
 */

#ifndef ETH_DEV_PARAM_H
#define ETH_DEV_PARAM_H

#include <stdint.h>
#include <inttypes.h>

#if !defined(_MSC_VER)
#include <net/if.h>
#else
struct ifreq {
	int s;
};
#endif



typedef struct EthDevParam_S
{
  int          sd;
  int         opened; // boolean
  struct ifreq ifr;

} EthDevParam;


int EthDevParam_is_open(EthDevParam *ethDevParam);

int EthDevParam_open(EthDevParam *ethDevParam, const char *dev_name);
void EthDevParam_close(EthDevParam *ethDevParam);

const char *EthDevParam_dev_name(EthDevParam *ethDevParam);


int EthDevParam_set_ip_from_string(EthDevParam *ethDevParam, const char *IP);
int EthDevParam_set_ip(EthDevParam *ethDevParam, uint32_t IP);

int EthDevParam_get_ip_from_string(EthDevParam *ethDevParam, char *IP);
int EthDevParam_get_ip(EthDevParam *ethDevParam, uint32_t *IP);


int EthDevParam_set_mask_from_string(EthDevParam *ethDevParam, const char *mask);
int EthDevParam_set_mask(EthDevParam *ethDevParam, uint32_t mask);

int EthDevParam_get_mask_from_string(EthDevParam *ethDevParam, char *mask);
int EthDevParam_get_mask(EthDevParam *ethDevParam, uint32_t *mask);
int EthDevParam_get_mask_prefix(EthDevParam *ethDevParam);


int EthDevParam_set_gateway_from_string(EthDevParam *ethDevParam, const char *gateway);
int EthDevParam_set_gateway(EthDevParam *ethDevParam, uint32_t gateway);

int EthDevParam_get_gateway_from_string(EthDevParam *ethDevParam, char *gateway);
int EthDevParam_get_gateway(EthDevParam *ethDevParam, uint32_t *gateway);


int EthDevParam_set_hwaddr_from_string(EthDevParam *ethDevParam, const char *hwaddr);
int EthDevParam_set_hwaddr(EthDevParam *ethDevParam, const uint8_t *hwaddr);

int EthDevParam_get_hwaddr_from_string(EthDevParam *ethDevParam, char *hwaddr);
int EthDevParam_get_hwaddr(EthDevParam *ethDevParam, uint8_t *hwaddr);



void EthDevParam_Constructor(EthDevParam *ethDevParam);
void EthDevParam_Destructor(EthDevParam *ethDevParam);




#endif // ETH_DEV_PARAM_H
