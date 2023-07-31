/***********************************************************************//** 
 * \file  bootload.h
 * \brief  bootload description and static inline functions at register level 
 * \copyright Copyright (C) 2015-2020 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2022-6-10 <td>V1.0  <td>YYM    <td>initial
 * </table>
 * *********************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"

#define PRINT_ENABLE           

#define APP_START_ADDR         0x2000ul
#define APP_EIP_ADDR           0x517ul
#define ROM_END_ADDR           0xffff //如果使用了dataflash功能，注意修改这个参数

#define BOOTLOADER_PRINT_BORD  115200
#define BOOTLOADER_PROG_BORD   115200

#define BOOTLOADER_PROG_CHECK  0xF2F3F4F5

#define BOOT_CMD_CHECK         0xEF
#define BOOT_CMD_ACK_CHECK     0xA5

#define ID_START_CHK           0xF5
#define ID_JUMP_BOOT           0x9F
#define ID_START_ADD           0x10
#define ID_END_ADD             0x20 
#define ID_JUMP_ADD            0x30 
#define ID_CHECK_NUM           0x40 
#define ID_PROG_DATA           0x80   


#define BOOT_SUCCESS           0
#define BOOT_ERROR             1
#define JUMP_SUCCESS           2

#define STARTCHECK_MODE           0
#define BOOTLOADER_MODE           1
#define JUMPAPP_MODE              2
#define WAITTING_MODE             3

#define BT_INT_TIMER_US        10    

#define PIN_CHECK_VALUE        0x08 

#define UART_PRINT_DIV  (24000000/BOOTLOADER_PRINT_BORD)  //串口波特率分频数 = pclk/baudrate
#define UART_PROG_DIV   (24000000/BOOTLOADER_PROG_BORD)   //串口波特率分频数 = pclk/baudrate

#define BOOT_BUF_SIZE          256
#define BOOT_INTEVAL_SIZE      256     //此值尽量与BufSize是整数倍的关系
#define BOOT_CODELEN           5

#define BOOT_PAGE_SIZE         0x100   // 256B


typedef enum{
	WDT_128MS  = 0,          
	WDT_256MS ,         
	WDT_512MS ,      
	WDT_1024MS,    
	WDT_2048MS,
	WDT_3072MS,
	WDT_4096MS,
	WDT_8192MS
} bt_wdt_times_e;        

typedef enum{
	BOOT_REST_ACK=0,
	BOOT_PARAM_ERR,
	BOOT_VERIFY,
	BOOT_BOOT,
	BOOT_JUMP_APP
} bt_message_e;

//uint16_t bootloader_prog_check_us(void);
void bootloader_send_back_str(const char *str,uint8_t byLen);
void bootloader_bt_irqhandler(csp_bt_t *ptBtBase);
void bootloader_prog_irqhandler(csp_uart_t *ptUartBase);
void  bootloader_printf_init(void);
#endif