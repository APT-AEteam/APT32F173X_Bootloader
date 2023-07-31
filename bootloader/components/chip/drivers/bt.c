/***********************************************************************//** 
 * \file  bt.c
 * \brief  csi bt driver
 * \copyright Copyright (C) 2015-2021 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2021-5-13 <td>V0.0 <td>ZJY     <td>initial
 * </table>
 * *********************************************************************
*/

#include <sys_clk.h>
#include <drv/bt.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include <drv/tick.h>

/* Private macro------------------------------------------------------*/

/* externs function---------------------------------------------------*/
/* externs variablesr-------------------------------------------------*/
/* Private variablesr-------------------------------------------------*/

/** \brief initialize bt data structure
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] wTimeOut: the timeout for bt, unit: us
 *  \return error code \ref csi_error_t
 */ 
csi_error_t csi_bt_timer_init(csp_bt_t *ptBtBase, uint32_t wTimeOut)
{
	uint32_t wTmLoad;
	uint32_t wClkDiv = 1;
	
	//csi_clk_enable((uint32_t *)ptBtBase);									//bt clk enable
	soc_clk_enable((csi_clk_module_e)BT0_SYS_CLK);
	csp_bt_soft_rst(ptBtBase);												//reset bt
	
	wTmLoad = 24 * wTimeOut;	//bt prdr load value
	if(wTmLoad > 0xffff)
	{
		return CSI_ERROR;
	}
		
	csp_bt_set_cr(ptBtBase, (BT_SHDOW << BT_SHDW_POS) | (BT_CONTINUOUS << BT_OPM_POS) |		//bt work mode
			(BT_PCLKDIV << BT_EXTCKM_POS) | (BT_CNTRLD_EN << BT_CNTRLD_POS) | BT_CLK_EN );
	csp_bt_set_pscr(ptBtBase, (uint16_t)wClkDiv - 1);						//bt clk div	
	csp_bt_set_prdr(ptBtBase, (uint16_t)wTmLoad);							//bt prdr load value
	csp_bt_set_cmp(ptBtBase, (uint16_t)(wTmLoad >> 1));						//bt prdr load value
	
//	csp_bt_set_pscr(ptBtBase, 5);						//bt clk div	
//	csp_bt_set_prdr(ptBtBase, 10000);							//bt prdr load value
//	csp_bt_set_cmp(ptBtBase, 5000);						//bt prdr load value
	
	csp_bt_int_set(ptBtBase, BT_PEND_INT, ENABLE);							//enable PEND interrupt
//	csp_bt_int_set(ptBtBase, BT_CMP_INT, ENABLE);							//enable PEND interrupt
	
//	csi_irq_enable((uint32_t *)ptBtBase);									//enable bt irq
	csi_vic_enable_irq((int32_t)BT0_IRQn);
	
    return CSI_OK;
}
/** \brief set bt count mode
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] eCntMode: bt count mode, one pulse/continuous
 *  \return none
 */ 
void csi_bt_count_mode(csp_bt_t *ptBtBase, csi_bt_cntmode_e eCntMode)
{
	csp_bt_count_mode(ptBtBase, (bt_opm_e)eCntMode);
}

/** \brief start bt
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \return none
 */ 
void csi_bt_start(csp_bt_t *ptBtBase)
{
	csp_bt_start(ptBtBase);
}
/** \brief stop bt
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \return none
 */ 
void csi_bt_stop(csp_bt_t *ptBtBase)
{
    csp_bt_stop(ptBtBase);
}

/** \brief stop bt
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] eIntSrc: bt interrupt source
 *  \param[in] bEnable: enable/disable interrupt
 *  \return none
 */ 
void csi_bt_int_enable(csp_bt_t *ptBtBase, csi_bt_intsrc_e eIntSrc, bool bEnable)
{
	csp_bt_int_set(ptBtBase, (bt_int_e)eIntSrc, bEnable);	
	
	if (bEnable) {
		csi_irq_enable((uint32_t *)ptBtBase);
	}
	else {
		if (eIntSrc == csp_bt_get_isr(ptBtBase)) {
			csi_irq_disable((uint32_t *)ptBtBase);
		}
	}
}
/** \brief get bt remaining value
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \return none
 */ 
uint32_t csi_bt_get_remaining_value(csp_bt_t *ptBtBase)
{
    uint32_t wTimeVal = csp_bt_get_prdr(ptBtBase) - csp_bt_get_cnt(ptBtBase);

	return wTimeVal;
}
/** \brief get bt load value
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \return none
 */ 
uint32_t csi_bt_get_load_value(csp_bt_t *ptBtBase)
{
	return csp_bt_get_prdr(ptBtBase);
}
/** \brief check bt is running
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \return rue: running; false: stopped
 */ 
bool csi_bt_is_running(csp_bt_t *ptBtBase)
{
	CSI_PARAM_CHK(ptBtBase, false);
	 
	if(ptBtBase->RSSR & BT_START)
		return true;
	else 
		return false;
}

/** \brief bt pwm init 
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] ptBtPwmCfg: pointer of bt pwm parameter config structure
 *  \return error code \ref csi_error_t
 */ 
csi_error_t csi_bt_pwm_init(csp_bt_t *ptBtBase, csi_bt_pwm_config_t *ptBtPwmCfg)
{
	uint32_t wClkDiv;
	uint32_t wCrVal;
	uint32_t wCmpLoad; 
	uint32_t wPrdrLoad; 
	
	if(ptBtPwmCfg->wFreq == 0)
		return CSI_ERROR;
	
	csi_clk_enable((uint32_t *)ptBtBase);								//bt clk enable
	csp_bt_soft_rst(ptBtBase);											//reset bt
		
	wClkDiv = (csi_get_pclk_freq() / ptBtPwmCfg->wFreq / 30000);		//bt clk div value
	if(wClkDiv == 0)
		wClkDiv = 1;
	
	wPrdrLoad  = csi_get_pclk_freq() / (wClkDiv * ptBtPwmCfg->wFreq);	//prdr load value
	if(ptBtPwmCfg->byDutyCycle >= 100)
	{
		wCmpLoad = wPrdrLoad + 1;
	}
	else
	{
		wCmpLoad = wPrdrLoad * ptBtPwmCfg->byDutyCycle / 100;			//cmp load value	
	}
	
	wCrVal = BT_CLK_EN | (BT_SHDOW << BT_SHDW_POS) | (BT_CONTINUOUS << BT_OPM_POS) | (BT_PCLKDIV << BT_EXTCKM_POS) |
				(BT_CNTRLD_EN << BT_CNTRLD_POS) | (ptBtPwmCfg->byIdleLevel << BT_IDLEST_POS) | (ptBtPwmCfg->byStartLevel << BT_STARTST_POS);
	csp_bt_set_cr(ptBtBase, wCrVal);									//set bt work mode
	csp_bt_set_pscr(ptBtBase, (uint16_t)wClkDiv - 1);					//bt clk div
	csp_bt_set_prdr(ptBtBase, (uint16_t)wPrdrLoad);						//bt prdr load value
	csp_bt_set_cmp(ptBtBase, (uint16_t)wCmpLoad);						//bt cmp load value
	
//	csp_bt_set_pscr(ptBtBase, 3);					//bt clk div
//	csp_bt_set_prdr(ptBtBase, 6000);						//bt prdr load value
//	csp_bt_set_cmp(ptBtBase, 500);	
	
	if(ptBtPwmCfg->byInt)
	{
		csp_bt_int_set(ptBtBase, ptBtPwmCfg->byInt, true);			//enable interrupt
		csi_irq_enable((uint32_t *)ptBtBase);							//enable bt irq
	}	
	return CSI_OK;
}
/** \brief  updata bt para and cmp reg value
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] hwPrdr: bt padr reg  value
 *  \param[in] hwCmp: bt cmp reg value
 *  \return none
 */
void csi_bt_prdr_cmp_updata(csp_bt_t *ptBtBase, uint16_t hwPrdr, uint16_t hwCmp) 
{
	csp_bt_set_prdr(ptBtBase, hwPrdr);						//bt prdr load value
	csp_bt_set_cmp(ptBtBase, hwCmp);						//bt cmp load value
}
/** \brief  updata bt pwm duty cycle
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] byDutyCycle: duty cycle(0 -> 100)
 *  \return none
 */
void csi_bt_pwm_duty_cycle_updata(csp_bt_t *ptBtBase, uint8_t byDutyCycle) 
{
	uint32_t wCmpLoad;			
	
	if(byDutyCycle >= 100)
		wCmpLoad = csp_bt_get_prdr(ptBtBase) + 1;
	else
		wCmpLoad = csp_bt_get_prdr(ptBtBase) * byDutyCycle / 100;	
	
	csp_bt_set_cmp(ptBtBase, (uint16_t)wCmpLoad);
	//csp_bt_updata_en(ptBtBase);
}

/** \brief  updata bt pwm freq and duty cycle
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] wFreq: pwm frequency  
 *  \param[in] byDutyCycle: pwm duty cycle(0 -> 100)
 *  \return none
 */
void csi_bt_pwm_updata(csp_bt_t *ptBtBase, uint32_t wFreq, uint8_t byDutyCycle) 
{
	uint32_t wCmpLoad;
	uint32_t wPrdrLoad; 
	uint16_t hwClkDiv = (csi_get_pclk_freq() / wFreq / 30000);		//bt clk div value
	
	if(0 == hwClkDiv)
		hwClkDiv = 1;

	wPrdrLoad  = (csi_get_pclk_freq() / (hwClkDiv * wFreq));		//prdr value
	
	if(byDutyCycle >= 100)
		wCmpLoad = wPrdrLoad + 1;
	else
		wCmpLoad = wPrdrLoad * byDutyCycle / 100;			//cmp load value	

	csp_bt_set_pscr(ptBtBase, hwClkDiv - 1);				//bt clk div
	csp_bt_set_prdr(ptBtBase, (uint16_t)wPrdrLoad);			//bt prdr load value
	csp_bt_set_cmp(ptBtBase, (uint16_t)wCmpLoad);			//bt cmp load value
}

/** \brief bt sync input evtrg config  
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] eTrgin: bt sync evtrg input channel(0~1)
 *  \param[in] eTrgMode: bt evtrg mode, continuous/once
 *  \param[in] eAutoRearm: auto rearm, BT_TRG_AUTOAREARM/BT_TRG_SYCAREARM
 *  \return none
 */
csi_error_t csi_bt_set_sync(csp_bt_t *ptBtBase,csi_bt_trgin_e eTrgin, csi_bt_trgmode_e eTrgMode, csi_bt_arearm_e eAutoRearm)
{
	if(eTrgin > BT_TRGIN_SYNCEN2)
		return CSI_ERROR;
	
//	ptBtBase->CR = ptBtBase->CR & ~(BT_SYNCCMD_MSK | BT_OSTMD_MSK(eTrgin) | BT_AREARM_MSK(eTrgin));
//	
//	ptBtBase->CR |=  (BT_SYNC_EN << BT_SYNC_POS(eTrgin)) | (BT_SYNCMD_DIS << BT_SYNCCMD_POS) | 
//				(eTrgMode << BT_OSTMD_POS(eTrgin)) | (eAutoRearm << BT_AREARM_POS(eTrgin));
	
	ptBtBase->CR = ptBtBase->CR & ~(BT_SYNCCMD_MSK | BT_OSTMD_MSK(eTrgin));
	ptBtBase->CR |=  ((BT_SYNC_EN << BT_SYNC_POS(eTrgin)) | (BT_SYNCMD_EN << BT_SYNCCMD_POS) | (eTrgMode << BT_OSTMD_POS(eTrgin)));
	if((eTrgin == BT_TRGIN_SYNCEN0) || (eTrgin == BT_TRGIN_SYNCEN1))
	{
		ptBtBase->CR = ptBtBase->CR & ~(BT_AREARM_MSK(eTrgin));
		ptBtBase->CR |= (eAutoRearm << BT_AREARM_POS(eTrgin));
	}
	if(eTrgin == BT_TRGIN_SYNCEN2)
	{
		ptBtBase->CR |= BT_EXTCKM_MSK;		//selecet count clk source
		//ptBtBase->RSSR |= BT_START;			//start bt				
	}
	else
		ptBtBase->CR &= ~BT_EXTCKM_MSK;		//selecet count clk source
		
	return CSI_OK; 
}
/** \brief rearm bt sync evtrg  
 * 
 *  \param[in] ptBtBase: pointer of bt register structure
 *  \param[in] eTrgin: bt sync evtrg input channel(0~1)
 *  \return none
 */
void csi_bt_rearm_sync(csp_bt_t *ptBtBase,csi_bt_trgin_e eTrgin)
{
	csp_bt_rearm_sync(ptBtBase, (bt_evtrg_in_e)eTrgin);
}
/** \brief bt evtrg output config
 * 
 *  \param[in] ptBtBase:pointer of bt register structure
 *  \param[in] byTrgOut: evtrg out port (byTrgOut = 0)
 *  \param[in] eTrgSrc: evtrg source(1~3) 
 *  \return error code \ref csi_error_t
 */
csi_error_t csi_bt_set_evtrg(csp_bt_t *ptBtBase,csi_bt_trgsrc_e eTrgSrc, bool bTrgoe)
{
	if(eTrgSrc == BT_TRGSRC_DIS)
		csp_bt_set_evtrg(ptBtBase,0x00);
	else
		csp_bt_set_evtrg(ptBtBase,(eTrgSrc | (bTrgoe << BT_TRGOE_POS)));
		
	return CSI_OK;
}

/** \brief bt software generates a trigger event
 * 
 *  \param[in] ptBtBase:pointer of bt register structure
 *  \return error code \ref csi_error_t
 */
void csi_bt_soft_evtrg(csp_bt_t *ptBtBase)
{
	csp_bt_evswf_en(ptBtBase);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/** \brief start bt by sync event
 * 
 *  \param[in] ptBtBase: handle bt handle to operate
 *  \param[in] wTimeOut: tthe timeout for bt, unit: us
 *  \return none
 */ 
void csi_bt_start_sync(csp_bt_t *ptBtBase, uint32_t wTimeOut)
{
	uint32_t wTmLoad;
	uint32_t wClkDiv;
	
	csi_clk_enable((uint32_t *)ptBtBase);									//bt clk enable
	csp_bt_soft_rst(ptBtBase);												//reset bt
	
	wClkDiv = (csi_get_pclk_freq()/100000 * wTimeOut/600000);				//bt clk div
	if(wClkDiv == 0)
		wClkDiv  = 1;
	wTmLoad = (csi_get_pclk_freq() / (wClkDiv * 20000)) * wTimeOut / 50;	//bt prdr load value
	
	csp_bt_set_cr(ptBtBase, (BT_IMMEDIATE << BT_SHDW_POS) | (BT_CONTINUOUS << BT_OPM_POS) |		//bt work mode
			(BT_PCLKDIV << BT_EXTCKM_POS) | (BT_CNTRLD_EN << BT_CNTRLD_POS) | BT_CLK_EN );
	csp_bt_set_pscr(ptBtBase, (uint16_t)wClkDiv - 1);						//bt clk div	
	csp_bt_set_prdr(ptBtBase, (uint16_t)wTmLoad);							//bt prdr load value
	csp_bt_int_set(ptBtBase, BT_PEND_INT, true);							//enable PEND interrupt
	csi_irq_enable((uint32_t *)ptBtBase);									//enable bt irq
}