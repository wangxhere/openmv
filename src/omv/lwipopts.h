
/**
  ******************************************************************************
  * File Name          : lwipopts.h
  * Date               : 31/12/2014 11:35:12
  * Description        : This file overrides LwIP stack default configuration
  *                      done in opt.h file.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LWIPOPTS__H__
#define __LWIPOPTS__H__

#include "stm32f4xx_hal.h"

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
 extern "C" {
#endif

/*----- Default Value for LWIP_DHCP: 0 -----*/  
#define LWIP_DHCP   1
/*----- Default Value for WITH_RTOS: 1 -----*/ 
#define WITH_RTOS   0
/*----- Default Value for NO_SYS: 0 -----*/ 
#define NO_SYS   1			
/*----- Default Value for MEM_ALIGNMENT: 1 -----*/ 
#define MEM_ALIGNMENT   4
/*----- Default Value for MEM_SIZE: 1600 -----*/
#define MEM_SIZE   65536		
/*----- Default Value for MEMP_NUM_SYS_TIMEOUT: 5 -----*/
#define MEMP_NUM_SYS_TIMEOUT   6
/*----- Default Value for LWIP_DNS: 0 -----*/
#define LWIP_DNS   1
/*----- Default Value for LWIP_NETCONN:  -----*/
#define LWIP_NETCONN  0
/*----- Default Value for LWIP_SOCKET:  -----*/
#define LWIP_SOCKET   0
/*----- No Default Value for LWIP_PROVIDE_ERRNO -----*/
#define LWIP_PROVIDE_ERRNO  1
/* USER CODE BEGIN 1 */
/*----- Heap for LwIP -----*/
extern	uint32_t				_lwip_heap_base;
#define	LWIP_RAM_HEAP_POINTER	(&_lwip_heap_base)
/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /*__ LWIPOPTS__H_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
