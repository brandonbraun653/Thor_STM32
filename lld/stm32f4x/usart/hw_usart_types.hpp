/********************************************************************************
 *  File Name:
 *    hw_usart_types.hpp
 *
 *  Description:
 *    STM32 Types for the USART Peripheral
 *
 *  2019-2021 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_HW_USART_TYPES_HPP
#define THOR_HW_USART_TYPES_HPP

/* C++ Includes */
#include <cstdint>

/* Chimera Includes */
#include <Chimera/common>
#include <Chimera/serial>

/* Driver Includes */
#include <Thor/lld/common/registers/field_accessor.hpp>
#include <Thor/lld/stm32l4x/interrupt/hw_interrupt_prj.hpp>
#include <Thor/lld/stm32f4x/usart/hw_usart_prj.hpp>

namespace Thor::LLD::USART
{
  /*-------------------------------------------------------------------------------
  Constants
  -------------------------------------------------------------------------------*/
  namespace Configuration
  {
    namespace WordLength
    {
      static constexpr Reg32_t LEN_8BIT = 0u;
      static constexpr Reg32_t LEN_9BIT = CR1_M;
    }    // namespace WordLength

    namespace Stop
    {
      static constexpr Reg32_t BIT_1   = 0u;
      static constexpr Reg32_t BIT_0_5 = CR2_STOP_0;
      static constexpr Reg32_t BIT_2   = CR2_STOP_1;
      static constexpr Reg32_t BIT_1_5 = CR2_STOP_0 | CR2_STOP_1;
    }    // namespace Stop

    namespace Parity
    {
      static constexpr Reg32_t NONE = 0u;
      static constexpr Reg32_t EVEN = CR1_PCE;
      static constexpr Reg32_t ODD  = CR1_PCE | CR1_PS;
    }    // namespace Parity

    namespace Modes
    {
      static constexpr Reg32_t RX    = CR1_RE;
      static constexpr Reg32_t TX    = CR1_TE;
      static constexpr Reg32_t TX_RX = RX | TX;
    }    // namespace Modes

    namespace Clock
    {
      static constexpr Reg32_t CLOCK_DISABLE = 0u;
      static constexpr Reg32_t CLOCK_ENABLE  = CR2_CLKEN;
    }    // namespace Clock

    namespace Polarity
    {
      static constexpr Reg32_t POLARITY_LOW  = 0u;
      static constexpr Reg32_t POLARITY_HIGH = CR2_CPOL;
    }    // namespace Polarity

    namespace Phase
    {
      static constexpr Reg32_t PHASE_1EDGE = 0u;
      static constexpr Reg32_t PHASE_2EDGE = CR2_CPHA;
    }    // namespace Phase

    namespace LastBit
    {
      static constexpr Reg32_t LASTBIT_DISABLE = 0u;
      static constexpr Reg32_t LASTBIT_ENABLE  = CR2_LBCL;
    }    // namespace LastBit

    namespace Nack
    {
      static constexpr Reg32_t NACK_ENABLE  = CR3_NACK;
      static constexpr Reg32_t NACK_DISABLE = 0u;
    }    // namespace Nack

    namespace Flags
    {
      static constexpr Reg32_t FLAG_CTS  = SR_CTS;
      static constexpr Reg32_t FLAG_LBD  = SR_LBD;
      static constexpr Reg32_t FLAG_TXE  = SR_TXE;
      static constexpr Reg32_t FLAG_TC   = SR_TC;
      static constexpr Reg32_t FLAG_RXNE = SR_RXNE;
      static constexpr Reg32_t FLAG_IDLE = SR_IDLE;
      static constexpr Reg32_t FLAG_ORE  = SR_ORE;
      static constexpr Reg32_t FLAG_NF   = SR_NF;
      static constexpr Reg32_t FLAG_FE   = SR_FE;
      static constexpr Reg32_t FLAG_PE   = SR_PE;
    }    // namespace Flags
  }


  namespace Runtime
  {
    using Flag_t = Reg32_t;
    namespace Flag
    {
      /* Let the first 16 bits match the Status Register for consistency */
      static constexpr Flag_t RX_PARITY_ERROR    = Configuration::Flags::FLAG_PE;
      static constexpr Flag_t RX_FRAMING_ERROR   = Configuration::Flags::FLAG_FE;
      static constexpr Flag_t RX_NOISE_ERROR     = Configuration::Flags::FLAG_NF;
      static constexpr Flag_t RX_OVERRUN         = Configuration::Flags::FLAG_ORE;
      static constexpr Flag_t RX_IDLE_DETECTED   = Configuration::Flags::FLAG_IDLE;
      static constexpr Flag_t RX_BYTE_READY      = Configuration::Flags::FLAG_RXNE;
      static constexpr Flag_t TX_COMPLETE        = Configuration::Flags::FLAG_TC;
      static constexpr Flag_t TX_DR_EMPTY        = Configuration::Flags::FLAG_TXE;
      static constexpr Flag_t RX_LINE_IN_BREAK   = Configuration::Flags::FLAG_LBD;
      static constexpr Flag_t CTL_CLEAR_TO_SEND  = Configuration::Flags::FLAG_CTS;

      /* Use the remaining 16 bits for other signals */
      static constexpr Flag_t RX_LINE_IDLE_ABORT = ( 1u << 16 );
      static constexpr Flag_t RX_COMPLETE        = ( 1u << 17 );
    }
  }


  /*-------------------------------------------------------------------------------
  Structures
  -------------------------------------------------------------------------------*/
  struct RegisterMap
  {
    volatile Reg32_t SR;   /**< USART Status register,                   Address offset: 0x00 */
    volatile Reg32_t DR;   /**< USART Data register,                     Address offset: 0x04 */
    volatile Reg32_t BRR;  /**< USART Baud rate register,                Address offset: 0x08 */
    volatile Reg32_t CR1;  /**< USART Control register 1,                Address offset: 0x0C */
    volatile Reg32_t CR2;  /**< USART Control register 2,                Address offset: 0x10 */
    volatile Reg32_t CR3;  /**< USART Control register 3,                Address offset: 0x14 */
    volatile Reg32_t GTPR; /**< USART Guard time and prescaler register, Address offset: 0x18 */
  };

  /*-------------------------------------------------------------------------------
  Classes
  -------------------------------------------------------------------------------*/
  /*-------------------------------------------------
  Control Register 1
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, CR1, CR1_Msk, CR1, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_UE_Msk, UE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_TCIE_Msk, TCIE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_TXEIE_Msk, TXEIE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_RXNEIE_Msk, RXNEIE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_IDLEIE_Msk, IDLEIE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_M0_Msk, M0, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_M1_Msk, M1, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_TE_Msk, TE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_RE_Msk, RE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_OVER8_Msk, OVER8, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_PCE_Msk, PCE, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR1, CR1_PS_Msk, PS, BIT_ACCESS_RW );

  /*-------------------------------------------------
  Control Register 2
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, CR2, CR2_Msk, CR2, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR2, CR2_STOP_Msk, STOP, BIT_ACCESS_RW );


  /*-------------------------------------------------
  Control Register 3
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, CR3, CR3_Msk, CR3, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, CR3, CR3_DMAT_Msk, DMAT, BIT_ACCESS_RW );

  /*-------------------------------------------------
  Baurd Rate Register
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, BRR, BRR_Msk, BRR, BIT_ACCESS_RW );

  /*-------------------------------------------------
  Interrupt and Status Register
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, ISR, ISR_Msk, ISR, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, ISR, ISR_TC_Msk, TC, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, ISR, ISR_RXNE_Msk, RXNE, BIT_ACCESS_RW );

  /*-------------------------------------------------
  Interrupt Flag Clear Register
  -------------------------------------------------*/
  REG_ACCESSOR( RegisterMap, ICR, ICR_Msk, ICR, BIT_ACCESS_RW );
  REG_ACCESSOR( RegisterMap, ICR, ICR_PECF_Msk, PECF, BIT_ACCESS_RCW1 );
  REG_ACCESSOR( RegisterMap, ICR, ICR_FECF_Msk, FECF, BIT_ACCESS_RCW1 );
  REG_ACCESSOR( RegisterMap, ICR, ICR_NECF_Msk, NECF, BIT_ACCESS_RCW1 );
  REG_ACCESSOR( RegisterMap, ICR, ICR_ORECF_Msk, ORECF, BIT_ACCESS_RCW1 );
  REG_ACCESSOR( RegisterMap, ICR, ICR_IDLECF_Msk, IDLECF, BIT_ACCESS_RCW1 );
  REG_ACCESSOR( RegisterMap, ICR, ICR_TCCF_Msk, TCCF, BIT_ACCESS_RCW1 );

}    // namespace Thor::LLD::USART

#endif /* !THOR_HW_USART_TYPES_HPP */