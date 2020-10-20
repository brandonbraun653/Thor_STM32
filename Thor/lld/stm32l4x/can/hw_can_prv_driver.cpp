/********************************************************************************
 *  File Name:
 *    hw_can_driver.cpp
 *
 *  Description:
 *    Implements internal private functions for the STM32L4 LLD CAN driver
 *
 *  2020 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* STL Includes */
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>

/* Aurora Includes */
#include <Aurora/math>

/* Chimera Includes */
#include <Chimera/algorithm>
#include <Chimera/common>
#include <Chimera/utility>

/* Driver Includes */
#include <Thor/cfg>
#include <Thor/hld/interrupt/hld_interrupt_definitions.hpp>
#include <Thor/lld/interface/can/can_intf.hpp>
#include <Thor/lld/interface/can/can_prv_data.hpp>
#include <Thor/lld/stm32l4x/can/hw_can_prj.hpp>
#include <Thor/lld/stm32l4x/can/hw_can_types.hpp>
#include <Thor/lld/stm32l4x/can/hw_can_prv_driver.hpp>
#include <Thor/lld/stm32l4x/rcc/hw_rcc_driver.hpp>

#if defined( TARGET_STM32L4 ) && defined( THOR_LLD_CAN )

namespace Thor::LLD::CAN
{
  /*-------------------------------------------------------------------------------
  Static Function Declaration
  -------------------------------------------------------------------------------*/
  static bool assign16BitListFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot );
  static bool assign16BitMaskFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot );
  static bool assign32BitListFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot );
  static bool assign32BitMaskFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot );

  /*-------------------------------------------------------------------------------
  Private Functions
  -------------------------------------------------------------------------------*/
  void prv_reset( RegisterMap *const periph )
  {
    MCR_ALL::set( periph, MCR_Rst );
    MSR_ALL::set( periph, MSR_Rst );
    TSR_ALL::set( periph, TSR_Rst );
    RF0R_ALL::set( periph, RF0R_Rst );
    RF1R_ALL::set( periph, RF1R_Rst );
    IER_ALL::set( periph, IER_Rst );
    ESR_ALL::set( periph, ESR_Rst );
    BTR_ALL::set( periph, BTR_Rst );
  }


  void prv_enter_initialization_mode( RegisterMap *const periph )
  {
    /*-------------------------------------------------
    If we are already in sleep mode, the sleep bit must
    be cleared first. See RM0394 44.4.3.
    -------------------------------------------------*/
    SLEEP::clear( periph, MCR_SLEEP );

    /*-------------------------------------------------
    Don't bother doing anything if device already in
    init mode. Otherwise request the transition.
    -------------------------------------------------*/
    if ( !INRQ::get( periph ) )
    {
      INRQ::set( periph, MCR_INRQ );
      while ( !INAK::get( periph ) )
      {
        continue;
      }
    }
  }


  void prv_enter_normal_mode( RegisterMap *const periph )
  {
    /*-------------------------------------------------
    Don't bother doing anything if device already not
    in init mode. Otherwise request the transition.
    -------------------------------------------------*/
    if ( INRQ::get( periph ) )
    {
      INRQ::clear( periph, MCR_INRQ );
      while ( INAK::get( periph ) )
      {
        continue;
      }
    }
  }


  void prv_enter_sleep_mode( RegisterMap *const periph )
  {
    /*-------------------------------------------------
    Don't bother doing anything if device already in
    sleep mode. Otherwise request the transition.
    -------------------------------------------------*/
    if ( !SLEEP::get( periph ) )
    {
      SLEEP::set( periph, MCR_SLEEP );
      while ( !SLAK::get( periph ) )
      {
        continue;
      }
    }
  }


  size_t prv_set_baud_rate( RegisterMap *const periph, const Chimera::CAN::DriverConfig &cfg )
  {
    /*-------------------------------------------------
    Grab a few preliminary variables. These equations
    are derived from RM0394 Figure 488.
    -------------------------------------------------*/
    size_t pclk = Thor::LLD::RCC::getCoreClock()->getPeriphClock( Chimera::Peripheral::Type::PERIPH_CAN,
                                                                  reinterpret_cast<std::uintptr_t>( periph ) );

    float nominalBitTime = 1.0f / ( float )( cfg.HWInit.baudRate );
    float clkPeriod      = 1.0f / ( float )( pclk );
    float tq             = nominalBitTime / ( float )( cfg.HWInit.timeQuanta );
    float tsync          = tq;
    float est_tbs1       = ( cfg.HWInit.samplePointPercent * nominalBitTime ) - tsync;
    float est_tbs2       = nominalBitTime - tsync - est_tbs1;

    /*-------------------------------------------------
    Calculate configuration settings for BTR register
    -------------------------------------------------*/
    Reg32_t brp = static_cast<Reg32_t>( tq / clkPeriod ) - 1;
    Reg32_t ts1 = static_cast<Reg32_t>( est_tbs1 / tq ) - 1;
    Reg32_t ts2 = static_cast<Reg32_t>( est_tbs2 / tq ) - 1;

    /*-------------------------------------------------
    Apply the configuration values
    -------------------------------------------------*/
    BRP::set( periph, ( brp << BTR_BRP_Pos ) );
    TS1::set( periph, ( ts1 << BTR_TS1_Pos ) );
    TS2::set( periph, ( ts2 << BTR_TS2_Pos ) );

    /*-------------------------------------------------
    Calculate the actual configured baud rate
    -------------------------------------------------*/
    return prv_get_baud_rate( periph );
  }


  size_t prv_get_baud_rate( RegisterMap *const periph )
  {
    using namespace Thor::LLD::RCC;
    using namespace Chimera::Peripheral;

    /*-------------------------------------------------
    Grab a few preliminary variables
    -------------------------------------------------*/
    Reg32_t brp = BRP::get( periph ) >> BTR_BRP_Pos;
    Reg32_t ts1 = TS1::get( periph ) >> BTR_TS1_Pos;
    Reg32_t ts2 = TS2::get( periph ) >> BTR_TS2_Pos;
    size_t pclk = getCoreClock()->getPeriphClock( Type::PERIPH_CAN, reinterpret_cast<std::uintptr_t>( periph ) );

    /*-------------------------------------------------
    Prevent div/0 in calculations below
    -------------------------------------------------*/
    if ( !pclk )
    {
      return std::numeric_limits<size_t>::max();
    }

    /*-------------------------------------------------
    Calculate the expected output baud rate. Taken from
    RM0394 Figure 488.
    -------------------------------------------------*/
    float tq             = ( 1.0f / ( float )pclk ) * ( float )( brp + 1 );
    float tbs1           = tq * ( float )( ts1 + 1 );
    float tbs2           = tq * ( float )( ts2 + 1 );
    float nominalBitTime = tq + tbs1 + tbs2;

    /*-------------------------------------------------
    Assume that a bit time corresponding to 10 MBit is
    invalid (not supported by standard). Also prevents
    a div/0 without having to do extremely precise
    floating point comparisons.
    -------------------------------------------------*/
    if ( nominalBitTime < 0.0000001f )
    {
      return std::numeric_limits<size_t>::max();
    }
    else
    {
      return static_cast<size_t>( 1.0f / nominalBitTime );
    }
  }


  bool prv_validate_frame( const Chimera::CAN::BasicFrame &frame )
  {
    using namespace Chimera::CAN;

    /* clang-format off */
    if ( ( frame.dataLength == 0 ) ||
         ( frame.idMode >= IdentifierMode::NUM_OPTIONS ) ||
         ( frame.frameType >= FrameType::NUM_OPTIONS ) )
    /* clang-format on */
    {
      return false;
    }
    else
    {
      return true;
    }
  }


  bool prv_in_normal_mode( RegisterMap *const periph )
  {
    return INRQ::get( periph ) == 0;
  }


  bool prv_does_filter_fit( const MessageFilter *const filter, volatile FilterReg *const bank, FilterSlot &slot )
  {
    using namespace Chimera::CAN;

    /*-------------------------------------------------
    Initialize the outputs
    -------------------------------------------------*/
    slot = FilterSlot::UNKNOWN;

    /*-------------------------------------------------
    Check each...possible...combination...
    Surely there is a better way than this. This nasty.
    It doesn't seem to lend itself to loops, but who
    knows. I know this is gross. That's why it's hidden
    in this function.
    -------------------------------------------------*/
    if ( filter->mode == FilterMode::MODE_16BIT_LIST )
    {
      constexpr Reg32_t FMSK_EVEN = 0x0000FFFF;
      constexpr Reg32_t FMSK_ODD  = 0xFFFF0000;

      /*-------------------------------------------------
      In this mode, four filters are possible, each 16bit
      -------------------------------------------------*/
      if ( ( bank->FR1 & FMSK_EVEN ) == ( FLTR_RST_1 & FMSK_EVEN ) )
      {
        slot = FilterSlot::SLOT_0;
      }
      else if ( ( bank->FR1 & FMSK_ODD ) == ( FLTR_RST_1 & FMSK_ODD ) )
      {
        slot = FilterSlot::SLOT_1;
      }
      else if ( ( bank->FR2 & FMSK_EVEN ) == ( FLTR_RST_2 & FMSK_EVEN ) )
      {
        slot = FilterSlot::SLOT_2;
      }
      else if ( ( bank->FR2 & FMSK_ODD ) == ( FLTR_RST_2 & FMSK_ODD ) )
      {
        slot = FilterSlot::SLOT_3;
      }
      // else this filter bank is full

    }
    else if( ( filter->mode == FilterMode::MODE_16BIT_MASK ) ||
             ( filter->mode == FilterMode::MODE_32BIT_LIST ) )
    {
      /*-------------------------------------------------
      Only two possible filter types, each 32bit wide.
      One could be configured as 16bit mask, the other
      as 32bit list. The analysis is the same.
      -------------------------------------------------*/
      if ( bank->FR1 == FLTR_RST_1 )
      {
        slot = FilterSlot::SLOT_0;
      }
      else if ( bank->FR2 == FLTR_RST_2 )
      {
        slot = FilterSlot::SLOT_1;
      }
      // else this filter bank is full
    }
    else if ( filter->mode == FilterMode::MODE_32BIT_MASK )
    {
      /*-------------------------------------------------
      Single possible filter type here
      -------------------------------------------------*/
      if ( ( bank->FR1 == FLTR_RST_1 ) && ( bank->FR2 == FLTR_RST_2 ) )
      {
        slot = FilterSlot::SLOT_0;
      }
      // else this filter bank is full
    }

    /*-------------------------------------------------
    Was the free slot successfully detected?
    -------------------------------------------------*/
    return ( slot != FilterSlot::UNKNOWN );
  }


  bool prv_assign_filter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot )
  {
    using namespace Chimera::CAN;

    switch ( filter->mode )
    {
      case FilterMode::MODE_16BIT_LIST:
        return assign16BitListFilter( filter, bank, slot );
        break;

      case FilterMode::MODE_16BIT_MASK:
        return assign16BitMaskFilter( filter, bank, slot );
        break;

      case FilterMode::MODE_32BIT_LIST:
        return assign32BitListFilter( filter, bank, slot );
        break;

      case FilterMode::MODE_32BIT_MASK:
        return assign32BitMaskFilter( filter, bank, slot );
        break;

      default:
        return false;
        break;
    };
  }


  Mailbox prv_get_filter_bank_fifo( RegisterMap *const periph, const size_t bank_idx )
  {
    return ( periph->FFA1R & ( 1u << bank_idx ) ) ? Mailbox::RX_MAILBOX_2 : Mailbox::RX_MAILBOX_1;
  }


  Chimera::CAN::FilterMode prv_get_filter_bank_mode( RegisterMap *const periph, const size_t bank_idx )
  {
    /*-------------------------------------------------
    Input protection
    -------------------------------------------------*/
    if( !periph || !( bank_idx < NUM_CAN_FILTER_BANKS ) )
    {
      return Chimera::CAN::FilterMode::UNKNOWN;
    }

    /*-------------------------------------------------
    Categorize the current filter mode/scale config
    -------------------------------------------------*/
    if( !( periph->FM1R & ( 1u << bank_idx ) ) && !( periph->FS1R & ( 1u << bank_idx ) ) )
    {
      return Chimera::CAN::FilterMode::MODE_16BIT_MASK;
    }
    else if( ( periph->FM1R & ( 1u << bank_idx ) ) && !( periph->FS1R & ( 1u << bank_idx ) ) )
    {
      return Chimera::CAN::FilterMode::MODE_16BIT_LIST;
    }
    else if( !( periph->FM1R & ( 1u << bank_idx ) ) && ( periph->FS1R & ( 1u << bank_idx ) ) )
    {
      return Chimera::CAN::FilterMode::MODE_32BIT_MASK;
    }
    else if( ( periph->FM1R & ( 1u << bank_idx ) ) && ( periph->FS1R & ( 1u << bank_idx ) ) )
    {
      return Chimera::CAN::FilterMode::MODE_32BIT_LIST;
    }
    else
    {
      return Chimera::CAN::FilterMode::UNKNOWN;
    }
  }


  /*-------------------------------------------------------------------------------
  Static Function Definition
  -------------------------------------------------------------------------------*/
  static bool assign16BitListFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot )
  {
    constexpr Reg32_t FMSK_EVEN = 0x0000FFFF;
    constexpr Reg32_t FMSK_ODD  = 0xFFFF0000;

    Reg32_t tmp = 0;

    switch ( slot )
    {
      case FilterSlot::SLOT_0:
        tmp = bank->FR1;
        tmp &= ~( FMSK_EVEN );
        tmp |= ( filter->identifier & FMSK_EVEN ) << 0;
        bank->FR1 = tmp;
        break;

      case FilterSlot::SLOT_1:
        tmp = bank->FR1;
        tmp &= ~( FMSK_ODD );
        tmp |= ( filter->identifier & FMSK_ODD ) << 16;
        bank->FR1 = tmp;
        break;

      case FilterSlot::SLOT_2:
        tmp = bank->FR2;
        tmp &= ~( FMSK_EVEN );
        tmp |= ( filter->identifier & FMSK_EVEN ) << 0;
        bank->FR2 = tmp;
        break;

      case FilterSlot::SLOT_3:
        tmp = bank->FR2;
        tmp &= ~( FMSK_ODD );
        tmp |= ( filter->identifier & FMSK_ODD ) << 16;
        bank->FR2 = tmp;
        break;

      default:
        return false;
        break;
    };

    return true;
  }


  static bool assign16BitMaskFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot )
  {
    constexpr Reg32_t FMSK_LO = 0x0000FFFF;
    constexpr Reg32_t FMSK_HI = 0xFFFF0000;

    Reg32_t tmp = 0;

    switch ( slot )
    {
      case FilterSlot::SLOT_0:
        tmp |= ( filter->identifier & FMSK_LO );
        tmp |= ( filter->mask << 16 ) & FMSK_HI;

        bank->FR1 = tmp;
        break;

      case FilterSlot::SLOT_1:
        tmp |= ( filter->identifier & FMSK_LO );
        tmp |= ( filter->mask << 16 ) & FMSK_HI;

        bank->FR2 = tmp;
        break;

      default:
        return false;
        break;
    };

    return true;
  }


  static bool assign32BitListFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot )
  {
    switch ( slot )
    {
      case FilterSlot::SLOT_0:
        bank->FR1 = filter->identifier;
        break;

      case FilterSlot::SLOT_1:
        bank->FR2 = filter->identifier;
        break;

      default:
        return false;
        break;
    };

    return true;
  }


  static bool assign32BitMaskFilter( const MessageFilter *const filter, volatile FilterReg *const bank, const FilterSlot slot )
  {
    switch ( slot )
    {
      case FilterSlot::SLOT_0:
        bank->FR1 = filter->identifier;
        bank->FR2 = filter->mask;
        break;

      default:
        return false;
        break;
    };

    return true;
  }

}    // namespace Thor::LLD::CAN

#endif /* TARGET_STM32L4 & THOR_LLD_CAN */
