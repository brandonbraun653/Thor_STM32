/********************************************************************************
 * File Name:
 *	  sram.hpp
 *
 * Description:
 *	  Interface to the internal SRAM of the Thor MCU
 *
 * 2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef THOR_INTERNAL_SRAM_HPP
#define THOR_INTERNAL_SRAM_HPP

/* C++ Includes */
#include <cstdint>

/* Chimera Includes */
#include <Chimera/modules/memory/flash.hpp>

/* Thor Includes */
#include <Thor/headers.hpp>

namespace Thor
{
  namespace Memory
  {
    class InternalSRAM : public Chimera::Modules::Memory::GenericInterface
    {
    public:
      InternalSRAM();
      ~InternalSRAM();

      Chimera::Status_t write( const uint32_t address, const uint8_t *const data, const uint32_t length ) final override;

      Chimera::Status_t read( const uint32_t address, uint8_t *const data, const uint32_t length ) final override;

      Chimera::Status_t erase( const uint32_t address, const uint32_t length ) final override;

      Chimera::Status_t writeCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

      Chimera::Status_t readCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

      Chimera::Status_t eraseCompleteCallback( const Chimera::Function::void_func_uint32_t func ) final override;

      bool isInitialized() final override;

    private:
    };
  }    // namespace Memory
}    // namespace Thor

#endif /* THOR_INTERNAL_SRAM_HPP */