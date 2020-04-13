//#include "xil_types.h"
//#include "xil_assert.h"
//#include "xscugic.h"
//#include "xparameters.h"
//
//#ifdef TRUST_IO
///************************** Constant Definitions *****************************/
//
///**************************** Type Definitions *******************************/
//
///***************** Macros (Inline Functions) Definitions *********************/
//
///************************** Function Prototypes ******************************/
//
//static void DistCopy(XScuGic_Config *Config, u32 CpuID);
//static void CPUCopy(XScuGic_Config *Config);
//static XScuGic_Config *LookupConfigByBaseAddress(u32 CpuBaseAddress);
//
///************************** Variable Definitions *****************************/
//
//extern XScuGic_Config XScuGic_ConfigTable[XPAR_XSCUGIC_NUM_INSTANCES];
//extern u32 CpuId;
//
///*****************************************************************************/
///**
//*
//* CfgInitialize a specific interrupt controller instance/driver. The
//* initialization entails:
//*
//* - Initialize fields of the XScuGic structure
//* - Initial vector table with stub function calls
//* - All interrupt sources are disabled
//*
//* @param InstancePtr is a pointer to the XScuGic instance to be worked on.
//* @param ConfigPtr is a pointer to a config table for the particular device
//*        this driver is associated with.
//* @param EffectiveAddr is the device base address in the virtual memory address
//*        space. The caller is responsible for keeping the address mapping
//*        from EffectiveAddr to the device physical base address unchanged
//*        once this function is invoked. Unexpected errors may occur if the
//*        address mapping changes after this function is called. If address
//*        translation is not used, use Config->BaseAddress for this parameters,
//*        passing the physical address instead.
//*
//* @return
//*
//* - XST_SUCCESS if initialization was successful
//*
//* @note
//*
//* None.
//*
//******************************************************************************/
//s32 XScuGic_DeviceCopy(u32 DeviceId)
//{
//	XScuGic_Config *Config;
//	u32 Cpu_Id = XScuGic_GetCpuID() + (u32)1;
//
//	Config = &XScuGic_ConfigTable[(u32 )DeviceId];
//
////	xil_printf("CPU ID: %08X BASE: %08X ADDR: %p", Cpu_Id,
////			Config->DistBaseAddress,
////			&XScuGic_ConfigTable);
//
//	DistCopy(Config, Cpu_Id);
//
//	CPUCopy(Config);
//
//	return XST_SUCCESS;
//}
//
///*****************************************************************************/
///**
//*
//* DistInit initializes the distributor of the GIC. The
//* initialization entails:
//*
//* - Write the trigger mode, priority and target CPU
//* - All interrupt sources are disabled
//* - Enable the distributor
//*
//* @param	InstancePtr is a pointer to the XScuGic instance.
//* @param	CpuID is the Cpu ID to be initialized.
//*
//* @return	None
//*
//* @note		None.
//*
//******************************************************************************/
//static void DistCopy(XScuGic_Config *Config, u32 CpuID)
//{
//	u32 Int_Id;
//	u32 LocalCpuID = CpuID;
//
//#if USE_AMP==1
//	#warning "Building GIC for AMP"
//
//	/*
//	 * The distrubutor should not be initialized by FreeRTOS in the case of
//	 * AMP -- it is assumed that Linux is the master of this device in that
//	 * case.
//	 */
//	return;
//#endif
//
//	/*
//	 * Set the security domains in the int_security registers for non-secure
//	 * interrupts. All are secure, so leave at the default. Set to 1 for
//	 * non-secure interrupts.
//	 */
//
//
//	/*
//	 * For the Shared Peripheral Interrupts INT_ID[MAX..32], set:
//	 */
//
//	/*
//	 * 1. The trigger mode in the int_config register
//	 * Only write to the SPI interrupts, so start at 32
//	 */
//	for (Int_Id = 32U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+16U) {
//	/*
//	 * Each INT_ID uses two bits, or 16 INT_ID per register
//	 * Set them all to be level sensitive, active HIGH.
//	 */
//		XScuGic_WriteReg(Config->DistBaseAddress,
//			XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id), 0U);
//	}
//
//
//#define DEFAULT_PRIORITY	0xa0a0a0a0U
//	for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+4U) {
//		/*
//		 * 2. The priority using int the priority_level register
//		 * The priority_level and spi_target registers use one byte per
//		 * INT_ID.
//		 * Write a default value that can be changed elsewhere.
//		 */
//		XScuGic_WriteReg(Config->DistBaseAddress,
//				XSCUGIC_PRIORITY_OFFSET_CALC(Int_Id),
//				DEFAULT_PRIORITY);
//	}
//
//	for (Int_Id = 32U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+4U) {
//		/*
//		 * 3. The CPU interface in the spi_target register
//		 * Only write to the SPI interrupts, so start at 32
//		 */
//		LocalCpuID |= LocalCpuID << 8U;
//		LocalCpuID |= LocalCpuID << 16U;
//
//		XScuGic_WriteReg(Config->DistBaseAddress,
//				XSCUGIC_SPI_TARGET_OFFSET_CALC(Int_Id), LocalCpuID);
//	}
//
//	for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+32U) {
//	/*
//	 * 4. Enable the SPI using the enable_set register. Leave all disabled
//	 * for now.
//	 */
//		XScuGic_WriteReg(Config->DistBaseAddress,
//		XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,
//		Int_Id),
//		0xFFFFFFFFU);
//
//	}
//
//	XScuGic_WriteReg(Config->DistBaseAddress, XSCUGIC_DIST_EN_OFFSET,
//						XSCUGIC_EN_INT_MASK);
//
//}
//
///*****************************************************************************/
///**
//*
//* CPUInit initializes the CPU Interface of the GIC. The initialization entails:
//*
//* - Set the priority of the CPU.
//* - Enable the CPU interface
//*
//* @param	ConfigPtr is a pointer to a config table for the particular
//*		device this driver is associated with.
//*
//* @return	None
//*
//* @note		None.
//*
//******************************************************************************/
//static void CPUCopy(XScuGic_Config *Config)
//{
//	/*
//	 * Program the priority mask of the CPU using the Priority mask
//	 * register
//	 */
//	XScuGic_WriteReg(Config->CpuBaseAddress, XSCUGIC_CPU_PRIOR_OFFSET,
//									0xF0U);
//
//	/*
//	 * If the CPU operates in both security domains, set parameters in the
//	 * control_s register.
//	 * 1. Set FIQen=1 to use FIQ for secure interrupts,
//	 * 2. Program the AckCtl bit
//	 * 3. Program the SBPR bit to select the binary pointer behavior
//	 * 4. Set EnableS = 1 to enable secure interrupts
//	 * 5. Set EnbleNS = 1 to enable non secure interrupts
//	 */
//
//	/*
//	 * If the CPU operates only in the secure domain, setup the
//	 * control_s register.
//	 * 1. Set FIQen=1,
//	 * 2. Set EnableS=1, to enable the CPU interface to signal secure .
//	 * interrupts Only enable the IRQ output unless secure interrupts
//	 * are needed.
//	 */
//	XScuGic_WriteReg(Config->CpuBaseAddress, XSCUGIC_CONTROL_OFFSET, 0x07U);
//
//}
//#endif
