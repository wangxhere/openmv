/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * lwip Python module.
 *
 */
#include <mp.h>
//#include <cc3k.h>
#include <stm32f4xx_hal.h>
#include "../../lwip/src/include/lwip/netif.h"
#include "py_led.h"
#include "lwip.h"
#include "ethernetif.h"

struct netif gnetif;

static mp_obj_t mod_lwip_init()
{
    /* Initialize LwIP module */
	MX_LWIP_Init();

    return mp_const_none;
}

static mp_obj_t mod_lwip_ifconfig()
{
//    tNetappIpconfigRetArgs ipconfig;
//    uint8_t *ip = &ipconfig.aucIP[0];
//    uint8_t *mask= &ipconfig.aucSubnetMask[0];
//    uint8_t *gw= &ipconfig.aucDefaultGateway[0];
//    uint8_t *dhcp= &ipconfig.aucDHCPServer[0];
//    uint8_t *dns= &ipconfig.aucDNSServer[0];
//    uint8_t *mac= &ipconfig.uaMacAddr[0];
//    uint8_t *ssid= &ipconfig.uaSSID[0]; //32

    printf ("IP:%d.%d.%d.%d\n"  \
            "Mask:%d.%d.%d.%d\n"\
            "GW:%d.%d.%d.%d\n"  \
            "MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
			(unsigned int)(gnetif.ip_addr.addr&0xff000000UL)>>24,
			(unsigned int)(gnetif.ip_addr.addr&0x00ff0000UL)>>16,
			(unsigned int)(gnetif.ip_addr.addr&0x0000ff00UL)>>8,
			(unsigned int)(gnetif.ip_addr.addr&0x000000ffUL)>>0,
			(unsigned int)(gnetif.netmask.addr&0xff000000UL)>>24,
			(unsigned int)(gnetif.netmask.addr&0x00ff0000UL)>>16,
			(unsigned int)(gnetif.netmask.addr&0x0000ff00UL)>>8,
			(unsigned int)(gnetif.netmask.addr&0x000000ffUL)>>0,
			(unsigned int)(gnetif.gw.addr&0xff000000UL)>>24,
			(unsigned int)(gnetif.gw.addr&0x00ff0000UL)>>16,
			(unsigned int)(gnetif.gw.addr&0x0000ff00UL)>>8,
			(unsigned int)(gnetif.gw.addr&0x000000ffUL)>>0,
			gnetif.hwaddr[5], gnetif.hwaddr[4],
			gnetif.hwaddr[3], gnetif.hwaddr[2],
			gnetif.hwaddr[1], gnetif.hwaddr[0]);
    return mp_const_none;
}

static mp_obj_t mod_lwip_connected()
{
//    if (lwip_connected && ip_obtained) {
        return mp_const_true;
//    }
//    return mp_const_false;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_lwip_init_obj,         mod_lwip_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_lwip_ifconfig_obj,     mod_lwip_ifconfig);
STATIC MP_DEFINE_CONST_FUN_OBJ_0 (py_lwip_connected_obj,    mod_lwip_connected);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_lwip) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),            (mp_obj_t)&py_lwip_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ifconfig),        (mp_obj_t)&py_lwip_ifconfig_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connected),       (mp_obj_t)&py_lwip_connected_obj },
};

static MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t lwip_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_lwip,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_lwip_init()
{
    return &lwip_module;
}
