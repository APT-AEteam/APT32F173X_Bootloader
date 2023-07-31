/***********************************************************************//** 
 * \file  bootload.c
 * \brief  bootload description and static inline functions at register level 
 * \copyright Copyright (C) 2015-2020 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2023-7-17 <td>V1.0  <td>YYM    <td>initial
 * </table>
 * *********************************************************************
*/

#include <string.h>
#include <sys_clk.h>
#include <drv/ifc.h>
#include <drv/uart.h>
#include <drv/gpio.h>
#include <drv/bt.h>
#include <drv/pin.h>
#include <drv/iwdt.h>
#include <drv/irq.h>
#include "reliability.h"

#include "tick.h"
#include "iostring.h"
#include "bootloader.h"

uint8_t byProgFailed =0;
uint8_t  byRwBuffer[BOOT_BUF_SIZE];

volatile uint32_t wImageSize =0;
volatile uint32_t wIndx =0;
volatile uint32_t wRecBufLen =0;

volatile uint32_t wHexStartAddr = 0;
volatile uint32_t wHexEndAddr   = 0;
volatile uint32_t wAppJumpAddr  = 0;
volatile uint32_t wCheckNum = 0;

uint16_t hwCheckProTime = 0;
uint16_t hwCheckTimeOutNum = 0;

void delayus_Bycode(unsigned int t)
{
	volatile unsigned int i =  t;
	while(i--){
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}	
}

void boot_mdelay(unsigned int t)
{
	volatile unsigned int i = t;
	while(i--){
		delayus_Bycode(940);
	}
}

/***********************************************************************

函数功能：清空接收缓存
返回值: 无
***********************************************************************/

void bootloader_clear_rec_buffer(void)
{
	wIndx=0;
	wRecBufLen=0;
	wImageSize=0;
}

void bootloader_bt_irqhandler(csp_bt_t *ptBtBase)
{
    // ISR content ...
	volatile uint32_t wMisr = csp_bt_get_isr(ptBtBase);
	
	if(wMisr & BT_PEND_INT)					//PEND interrupt
	{
		hwCheckProTime ++;
		csp_bt_clr_isr(ptBtBase, BT_PEND_INT);
	}
	if(hwCheckProTime > hwCheckTimeOutNum)
	{
		hwCheckProTime = hwCheckTimeOutNum;
		bootloader_clear_rec_buffer();
	}
}

/***********************************************************************

函数功能：串口接收处理函数

***********************************************************************/
void bootloader_prog_irqhandler(csp_uart_t *ptUartBase)
{
	uint8_t byRxData;
	
	if( csp_uart_get_isr(ptUartBase) & UART_RX_INT_S ) 
	{
		byRxData = (uint8_t) csp_uart_get_data(ptUartBase);
		
		wImageSize++;
		byRwBuffer[wIndx] = byRxData;
		wIndx++;
		wRecBufLen++;
		hwCheckProTime = 0;
		if(wIndx == BOOT_BUF_SIZE ) {
			wIndx =0;
		}
		csp_uart_clr_isr(ptUartBase,UART_RX_INT_S);	
	}
}

/***********************************************************************

函数功能：读取flash一个word数据

***********************************************************************/
uint32_t bootloader_flash_read_word(uint32_t wAddr)
{
 	return (*(volatile uint32_t *)(wAddr));
}

/*
uint16_t bootloader_prog_check_us(void)
{
	uint16_t hwChecktime;
	//hwChecktime = 1000000*35/BOOTLOADER_PROG_BORD; //3.5个字节 
	hwChecktime = 1000000*100/BOOTLOADER_PROG_BORD; //10个字节 
	return hwChecktime;
}
*/
/***********************************************************************

函数功能：初始化串口功能

***********************************************************************/

void  bootloader_printf_init(void)
{
	csi_pin_set_mux(PC10, PC10_UART2_TX);		//TX  

	soc_clk_enable(UART2_SYS_CLK);
	csp_uart_set_brdiv(UART2, UART_PRINT_DIV);				//set uart baud rate 
	csp_uart_set_ctrl(UART2, (UART_TX_EN << UART_TX_POS));
//	csp_uart_set_ctrl(UART2, (UART_TX_EN << UART_TX_POS) | (UART_RX_EN << UART_RX_POS)|UART_RX_INT);		//enable uart rx/tx 
//	csi_vic_enable_irq((int32_t)UART2_IRQ_NUM);
}

void  bootloader_prog_init(void)
{
	csi_pin_set_mux(PA2, PA2_UART1_TX);		//TX  48	
	csi_pin_set_mux(PA3, PA3_UART1_RX);		//RX   1
	
	soc_clk_enable(UART1_SYS_CLK);
	csp_uart_set_brdiv(UART1, UART_PROG_DIV);				//set uart baud rate 
	csp_uart_set_ctrl(UART1, (UART_TX_EN << UART_TX_POS) | (UART_RX_EN << UART_RX_POS)|UART_RX_INT);		//enable uart rx/tx 
	csi_vic_enable_irq((int32_t)UART1_IRQn);
}

/***********************************************************************

函数功能：MCU发送响应字符串

***********************************************************************/
void bootloader_send_ack(volatile uint8_t *p,uint8_t len)
{
	uint8_t j;

	csi_uart_putc(UART1, BOOT_CMD_ACK_CHECK);
	for(j=0;j<len;j++){
		csi_uart_putc(UART1, p[j]);
	}
}

void bootloader_send_back_num(uint32_t wNum)
{
#ifdef PRINT_ENABLE	
	char cc;
	uint8_t i;
	for(i=0;i<8;i++){
		cc = (wNum >> ((7-i) << 2)) & 0xF;
		if(cc < 10) cc += '0';
		else cc += ('A' - 10);
		csi_uart_putc(UART2, (uint8_t)cc);
	}
#endif
}

/***********************************************************************

函数功能：MCU发送响应字符串

***********************************************************************/
void bootloader_send_back_str(const char *str,uint8_t byLen)
{	
#ifdef PRINT_ENABLE	
	uint8_t j =0;

	for(j=0;j<byLen;j++){
		csi_uart_putc(UART2, (uint8_t)str[j]);
	}
#endif
}

/***********************************************************************

函数功能：烧录串口接收数据到flash

***********************************************************************/

int bootloader_program_flash(void)
{
	uint8_t byProgAck[4] = {0xfe,0xff,0xef,0x5a};
	uint32_t programcnt =0;
	uint32_t wAddrOffset =0;

	wAddrOffset = APP_START_ADDR;
	wImageSize += APP_START_ADDR;
	while(1){
		if(wRecBufLen/BOOT_INTEVAL_SIZE > 0){
			if(wAddrOffset <= ROM_END_ADDR - BOOT_INTEVAL_SIZE){
				csi_ifc_pflash_page_program(IFC, wAddrOffset, (uint32_t *)&(byRwBuffer[programcnt]), BOOT_INTEVAL_SIZE);
				//csi_ifc_program(IFC, wAddrOffset, (uint32_t *)&(byRwBuffer[programcnt]), BOOT_INTEVAL_SIZE);  		
			}
			else{
				csi_ifc_pflash_page_program(IFC, wAddrOffset, (uint32_t *)&(byRwBuffer[programcnt]), ROM_END_ADDR - wAddrOffset);  
				//csi_ifc_program(IFC, wAddrOffset, (uint32_t *)&(byRwBuffer[programcnt]), ROM_END_ADDR - wAddrOffset);  
				break;
			}
			bootloader_send_ack(byProgAck,4);
			csi_iwdt_feed();
			bootloader_send_back_num(wAddrOffset); 
			programcnt +=BOOT_INTEVAL_SIZE;
			wAddrOffset +=BOOT_INTEVAL_SIZE;
			if(wAddrOffset > ROM_END_ADDR) wAddrOffset = ROM_END_ADDR-4;
			wRecBufLen -=BOOT_INTEVAL_SIZE;
			programcnt %=BOOT_BUF_SIZE;				
		}
		if((wImageSize >= wHexEndAddr)&&((wRecBufLen/BOOT_INTEVAL_SIZE) == 0))
		{
			break;
		}
	}
	return BOOT_SUCCESS;
}

/***********************************************************************

函数功能：检查复位同步pin的状态，等待同步pin释放
参数：byLogic -- 释放对应的电平逻辑 1 高电平代表释放，0 低电平代表释放
参数：wTimeOut -- 是否需要wTimeOut 0 不需要， 其他 wTimeOut计数值

***********************************************************************/
uint8_t bootloader_wait_syncpin_release(void)
{
	uint32_t wHighTime=0,wLowTime=0;
	
	csi_pin_set_mux(PA3, PA3_INPUT);	
	while(1)
	{				
		boot_mdelay(2);
		if(csi_pin_read(PA3) == PIN_CHECK_VALUE) {
			wHighTime++;
			wLowTime = 0;
		}
		else {
			wLowTime ++ ;
			wHighTime = 0;
		}
		if(wHighTime >= 20) 
			return 1;
		else if(wLowTime >= 20) 
			return 0;
	}
}

/***********************************************************************

函数功能：计算代码镜像校验和

***********************************************************************/
uint32_t bootloader_cal_image_checknum(void)
{
	uint32_t wSum =0;
	uint32_t wAddr = wHexStartAddr;	
	while(wAddr<wHexEndAddr){		
		wSum +=   bootloader_flash_read_word(wAddr);
		wAddr +=4;
	}
	return wSum;
}

/***********************************************************************

函数功能：对烧录的代码做校验
返回值: 错误码
***********************************************************************/
int bootloader_program_verify(void)
{
	uint32_t wCheck_Sum = wCheckNum+1;
	wCheck_Sum = bootloader_cal_image_checknum();
	if(wCheckNum != wCheck_Sum) {
		return BOOT_ERROR;
	}
	//my_printf("\n# VrfCkSum:0x%x\n",wCheck_Sum);
	bootloader_send_back_str("CkSum",5);
	bootloader_send_back_num(wCheck_Sum);  
	bootloader_send_back_str("\n",1);
	return BOOT_SUCCESS;
}

/***********************************************************************

函数功能：boot起始函数

***********************************************************************/
uint8_t bootloader_program_option(void)
{
	uint8_t bootopt = 0;
	if((bootloader_wait_syncpin_release() == 1)&&(csp_get_ureg(SYSCON, USER_REG0) == 0))
	{
		bootopt = JUMPAPP_MODE;
	}
	else
	{
		bootloader_prog_init();
		bootopt = BOOTLOADER_MODE;	
	}
	return bootopt;	
}

uint32_t bootloader_get_cmd_value(void)
{
	uint32_t wValue;
	
	wValue = (byRwBuffer[2]<<24)|(byRwBuffer[3]<<16)|(byRwBuffer[4]<<8)|byRwBuffer[5];
	return wValue;	
}

/*
void bootloader_buf_reset(void)
{
	uint16_t i;
	for(i = 0;i < BOOT_BUF_SIZE;i ++)
	{
		byRwBuffer[i] = 0x00;
	}
}
*/
/***********************************************************************

函数功能：boot APP升级初始化

***********************************************************************/
void bootloader_program_init(void)
{
	hwCheckTimeOutNum = 80;           //bootloader_prog_check_us()/BT_INT_TIMER_US;
	memset(byRwBuffer,0,BOOT_BUF_SIZE);
	//csi_iwdt_init(IWDT_TO_4096);			//初始化看门狗，溢出时间为2000ms(系统复位时间)
	csp_iwdt_set_ovt(SYSCON, IWDT_TO_4096);
	csp_iwdt_clr(SYSCON);
	csi_iwdt_open();						//打开看门狗
	csi_iwdt_feed();
	bootloader_clear_rec_buffer();		
}
/***********************************************************************

函数功能：boot APP升级函数

***********************************************************************/

int bootloader_program_loading(void)
{
	uint8_t byCmdId = 0,byProgStart = 0;
	uint8_t byCmdAck[4] = {0};
	int g_error = BOOT_SUCCESS;
	uint32_t wProgCheckStart = 0;

	bootloader_program_init();
	bootloader_send_back_str("prog\n",5);
	while(1)
	{
		if(wIndx >= 6)
		{
			if(byRwBuffer[0] == BOOT_CMD_CHECK)
			{
				byCmdId = byRwBuffer[1];
				memcpy(byCmdAck,&byRwBuffer[2],4);
			}
		}
		switch(byCmdId)
		{
			case ID_START_CHK:
				wProgCheckStart = bootloader_get_cmd_value();
				if(wProgCheckStart == BOOTLOADER_PROG_CHECK)
				{
					byProgStart = 1;
				}
				else 
				{
					bootloader_send_back_str("err0\n",5);	
				}
				bootloader_send_ack(byCmdAck,4);
				boot_mdelay(1);
				byCmdId = 0;
			break;
			case ID_START_ADD:
				if(byProgStart)
				{
					wHexStartAddr = bootloader_get_cmd_value();
					bootloader_send_ack(byCmdAck,4);
				}
				if(wHexStartAddr != APP_START_ADDR)
				{
					bootloader_send_back_str("err1\n",5);	
				}
				boot_mdelay(1);
				byCmdId = 0;
			break;
			case ID_END_ADD:
				if(byProgStart)
				{
					wHexEndAddr = bootloader_get_cmd_value();
					bootloader_send_ack(byCmdAck,4);
				}
				if(wHexEndAddr <= APP_START_ADDR)
				{
					bootloader_send_back_str("err2\n",5);	
				}
				boot_mdelay(1);
				byCmdId = 0;
			
			break;
			case ID_JUMP_ADD:
				if(byProgStart)
				{
					wAppJumpAddr = bootloader_get_cmd_value();
					bootloader_send_ack(byCmdAck,4);
				}
				boot_mdelay(1);
				byCmdId = 0;
			break;
			case ID_CHECK_NUM:
				if(byProgStart)
				{
					wCheckNum = bootloader_get_cmd_value();
					bootloader_send_ack(byCmdAck,4);
				}
				boot_mdelay(1);
				byCmdId = 0;
			break;
			case ID_PROG_DATA:
				if(byProgStart != 1)  return BOOT_ERROR;
				wCheckNum = bootloader_get_cmd_value();
				bootloader_send_ack(byCmdAck,4);
				bootloader_clear_rec_buffer();
				byCmdId = 0;
				hwCheckTimeOutNum = 0x8000;
				g_error = bootloader_program_flash();
				if(g_error) {
					byProgFailed = 1;
					return BOOT_ERROR;
				}
				csi_iwdt_close();
				g_error = bootloader_program_verify();	
				
				if(g_error == BOOT_ERROR)
				{
					bootloader_send_back_str("Vy Fail\n",8);
					return BOOT_ERROR;	
				}
				else
				{
					bootloader_send_back_str("Vy OK\n",6);
					csp_set_ureg(SYSCON, USER_REG0, 0);
					return BOOT_SUCCESS;	
				}
			break;
			default:
			break;
			
		}
		csi_iwdt_feed();
	}
}

/***********************************************************************

函数功能：IAP 跳转到 APP 函数

***********************************************************************/

void bootloader_jump_to_app(void)
{
	asm("mv t2,%0\n"::"r"(APP_START_ADDR):);
	asm(" jalr t2 ");
}

uint8_t bootloader_check_jump_addr()
{
	uint32_t wTemp;
	uint8_t byResult = 0;
	wTemp = bootloader_flash_read_word(APP_START_ADDR);
	if(wTemp == APP_EIP_ADDR )byResult = 1;
	return byResult;
}

/***********************************************************************

函数功能：boot跳转到 APP 函数

***********************************************************************/
int bootloader_jump_function(void)
{
	boot_mdelay(200);
	if(byProgFailed ==1 || bootloader_check_jump_addr() == 0){
		bootloader_send_back_str("BtFail\n",7);
		return BOOT_ERROR;
	}
	bootloader_send_back_str("JumpAPP\n",8);
	boot_mdelay(100);
	bootloader_jump_to_app();
	return JUMP_SUCCESS;	
}

/***********************************************************************

函数功能：boot复位函数

***********************************************************************/
void bootloader_reset(void)
{
//	SYSCON -> RSR = 0xFFFFFFFF;
	SYSCON -> IDCCR = 0xE11E0081; //系统软复位	
}

/***********************************************************************

函数功能：boot初始化函数

***********************************************************************/
void bootloader_system_init(void)
{
	SYSCON->PCER0=0xFFFFFFF;  
//	csi_bootloader_sysclk_config();
	
	CK_CPU_ENALLNORMALIRQ;
	csi_iwdt_close();				//close iwdt
}
