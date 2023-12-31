/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     timer.h
 * @brief    header file for timer driver
 * @version  V1.0
 * @date     27. Apr 2020
 * @model    timer
 ******************************************************************************/

#ifndef _DRV_BT_H_
#define _DRV_BT_H_

#include <drv/common.h>

#include "csp.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \struct csi_bt_pwm_config_t
/// \brief  bt pwm parameter configuration, open to users  
typedef struct {
	uint8_t		byIdleLevel;		//TIMER PWM OUTPUT idel level
	uint8_t		byStartLevel;		//TIMER PWM OUTPUT start Level
	uint8_t		byInt;			    //TIMER PWM interrupt source select
	uint8_t		byDutyCycle;		//TIMER PWM OUTPUT duty cycle
	uint32_t	wFreq;				//TIMER PWM OUTPUT frequency
} csi_bt_pwm_config_t;

/**
 * \enum     csi_bt_pwmlev_e
 * \brief    BT pwm output level 
 */
typedef enum
{
	BT_PWM_START_LOW	= 0,		//PWM start level is low 
    BT_PWM_START_HIGH 	= 1,		//PWM start level is high        
	BT_PWM_IDLE_LOW  	= 0,		//PWM idle level is low 
    BT_PWM_IDLE_HIGH	= 1 		//PWM idle level is high   	    
}csi_bt_pwmlev_e;

/**
 * \enum     csi_bt_workmode_e
 * \brief    BT count mode  
 */
typedef enum{
	BT_CNT_ONEPULSE	= 0,			//one pulse 	
	BT_CNT_CONTINUOUS				//continuous		
}csi_bt_cntmode_e;

/**
 * \enum     csi_bt_trgin_e
 * \brief    BT sync trigger input 
 */
typedef enum{
	BT_TRGIN_SYNCEN0	= 0,		//BT sync trigger input0		
	BT_TRGIN_SYNCEN1,				//BT sync trigger input1	
	BT_TRGIN_SYNCEN2				//BT sync trigger input2		
}csi_bt_trgin_e;

/**
 * \enum     csi_bt_trgmode_e
 * \brief    BT sync trigger mode 
 */
typedef enum{
	BT_TRG_CONTINU	= 0,			//BT continuous trigger mode 
	BT_TRG_ONCE					
}csi_bt_trgmode_e;					//BT once trigger mode 

/**
 * \enum     csi_bt_arearm_e
 * \brief    BT sync arearm mode 
 */
typedef enum{
	BT_TRG_AUTOAREARM	= 1,			//x1：计数器周期结束时，自动AREARM
	BT_TRG_SYCAREARM	                //1x：SYNCIN[1]时，自动AREARM			
}csi_bt_arearm_e;					    //BT  sync arearm mode 


/**
 * \enum     csi_bt_trgsrc_e
 * \brief    BT event trigger source
 */
typedef enum
{
	BT_TRGSRC_DIS	= 0,  			//BT none trigger	       
	BT_TRGSRC_PEND,					//BT PEND as trigger event	
	BT_TRGSRC_CMP,					//BT CMP as trigger event		
	BT_TRGSRC_OVF					//BT CMP as trigger event	         
}csi_bt_trgsrc_e;

/**
 * \enum     csi_bt_intsrc_e
 * \brief    BT interrupt source 
 */
typedef enum
{
	BT_INTSRC_NONE   =	(0x00ul << 0), 		//NONE interrupt
	BT_INTSRC_PEND   =	(0x01ul << 0),		//PEND interrupt
	BT_INTSRC_CMP    =	(0x01ul << 1),		//CMP interrupt   
	BT_INTSRC_OVF    =	(0x01ul << 2), 		//OVF interrupt
	BT_INTSRC_EVTRG  =	(0x01ul << 3)		//EVTRG interrupt
}csi_bt_intsrc_e;


/**
  \brief       BT irq 
  \param[in]   ptBtBase		pointer of bt register structure
  \return      none
*/
__attribute__((weak)) void bt_irqhandler(csp_bt_t *ptBtBase);
/**
  \brief       Initialize BT Interface. Initializes the resources needed for the TIMER interface
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   wTimeOut		the timeout for bt, unit: us
  \return      error code \ref csi_error_t
*/
csi_error_t csi_bt_timer_init(csp_bt_t *ptBtBase, uint32_t wTimeOut);

/** 
  \brief 	   set work mode
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   eCntMode		bt count mode, one pulse/continuous
  \return 	   none
 */ 
void csi_bt_count_mode(csp_bt_t *ptBtBase, csi_bt_cntmode_e eCntMode);

/** 
  \brief 	   start bt
  \param[in]   ptBtBase		pointer of bt register structure
  \return      none
 */ 
void csi_bt_start(csp_bt_t *ptBtBase);

/** 
  \brief       stop bt
  \param[in]   ptBtBase		pointer of bt register structure
  \return      none
 */ 
void csi_bt_stop(csp_bt_t *ptBtBase);

/** 
  \brief 	   enable/disable bt interrupt
  \param[in]   ptBtBase	 	pointer of bt register structure
  \param[in]   eIntSrc		bt interrupt source
  \param[in]   bEnable		enable/disable interrupt
  \return 	   none
 */ 
void csi_bt_int_enable(csp_bt_t *ptBtBase, csi_bt_intsrc_e eIntSrc, bool bEnable);

/**
  \brief       Get bt remaining value
  \param[in]   ptBtBase		pointer of bt register structure
  \return      timer		remaining value
*/
uint32_t csi_bt_get_remaining_value(csp_bt_t *ptBtBase);

/**
  \brief       Get bt load value
  \param[in]   ptBtBase		pointer of bt register structure
  \return      timer		load value
*/
uint32_t csi_bt_get_load_value(csp_bt_t *ptBtBase);

/**
  \brief       Check bt is running
  \param[in]   ptBtBase		pointer of bt register structure
  \return      true->running, false->stopped
*/
bool csi_bt_is_running(csp_bt_t *ptBtBase);

/**
  \brief       bt pwm init
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   ptTmPara		pointer of bt parameter structure
  \return      error code \ref csi_error_t
*/
csi_error_t csi_bt_pwm_init(csp_bt_t *ptBtBase, csi_bt_pwm_config_t *ptBtPwmCfg);

/** 
  \brief  	   updata bt para and cmp reg value
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   hwPrdr		bt prdr reg value
  \param[in]   hwCmp  		bt cmp reg value
  \return      none
 */
void csi_bt_prdr_cmp_updata(csp_bt_t *ptBtBase, uint16_t hwPrdr, uint16_t hwCmp);

/** 
  \brief  	   updata bt pwm duty cycle
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   byDutyCycle	duty cycle(0 -> 100)
  \return 	   none
 */
void csi_bt_pwm_duty_cycle_updata(csp_bt_t *ptBtBase, uint8_t byDutyCycle); 

/** 
  \brief  	   updata bt pwm freq and duty cycle
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   wfreq  	  	pwm frequency  
  \param[in]   byDutyCycle	pwm duty cycle(0 -> 100)
  \return 	   none
 */
void csi_bt_pwm_updata(csp_bt_t *ptBtBase, uint32_t wfreq, uint8_t byDutyCycle); 

/** 
  \brief 	   bt sync evtrg config  
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   eTrgin		bt evtrg input channel(0~1)
  \param[in]   eTrgMode 	bt evtrg mode, continuous/once
  \param[in]   bAutoRearm 	auto rearm, ENABLE/DISABLE(true/false)
  \return      none
*/
csi_error_t csi_bt_set_sync(csp_bt_t *ptBtBase,csi_bt_trgin_e eTrgin, csi_bt_trgmode_e eTrgMode, csi_bt_arearm_e bAutoRearm);

/** 
  \brief 	   restart bt sync evtrg 
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   eTrgin		bt evtrg input channel(0~1)
  \return      none
 */
void csi_bt_rearm_sync(csp_bt_t *ptBtBase,csi_bt_trgin_e eTrgin);

/** 
  \brief 	   bt evtrg output config
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   eTrgSrc 		bt evtrg source(0~3) 
  \param[in]   bTrgoe		evtrg out enable
  \return 	   error code \ref csi_error_t
 */
csi_error_t csi_bt_set_evtrg(csp_bt_t *ptBtBase,csi_bt_trgsrc_e eTrgSrc, bool bTrgoe);

/** \brief bt software generates a trigger event
 * 
 *  \param[in] ptBtBase:pointer of bt register structure
 *  \return error code \ref csi_error_t
 */
void csi_bt_soft_evtrg(csp_bt_t *ptBtBase);

/** \brief     start bt by sync event
  \param[in]   ptBtBase		pointer of bt register structure
  \param[in]   wTimeOut 	the timeout for bt, unit: us
  \return      none
 */ 
void csi_bt_start_sync(csp_bt_t *ptBtBase, uint32_t wTimeOut);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_TIMER_H_ */
