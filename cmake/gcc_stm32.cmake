GET_FILENAME_COMPONENT(STM32_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
SET(CMAKE_MODULE_PATH ${STM32_CMAKE_DIR} ${CMAKE_MODULE_PATH})

SET(STM32_SUPPORTED_FAMILIES L0 L4 F0 F1 F2 F3 F4 F7 CACHE INTERNAL "stm32 supported families")
IF(STM32_CHIP)
	 SET(STM32_CHIP "${STM32_CHIP}" CACHE STRING "STM32 chip to build for")
ENDIF()

IF(NOT STM32_FAMILY)
    MESSAGE(STATUS "No STM32_FAMILY specified, trying to get it from STM32_CHIP")
    IF(NOT STM32_CHIP)
        SET(STM32_FAMILY "F1" CACHE INTERNAL "stm32 family")
        MESSAGE(STATUS "Neither STM32_FAMILY nor STM32_CHIP specified, using default STM32_FAMILY: ${STM32_FAMILY}")
    ELSE()
        STRING(REGEX REPLACE "^[sS][tT][mM]32(([fF][0-47])|([lL][0-14])|([tT])|([wW])).+$" "\\1" STM32_FAMILY ${STM32_CHIP})
        STRING(TOUPPER ${STM32_FAMILY} STM32_FAMILY)
        MESSAGE(STATUS "Selected STM32 family: ${STM32_FAMILY}")
    ENDIF()
ENDIF()

STRING(TOUPPER "${STM32_FAMILY}" STM32_FAMILY)
LIST(FIND STM32_SUPPORTED_FAMILIES "${STM32_FAMILY}" FAMILY_INDEX)

IF(FAMILY_INDEX EQUAL -1)
    MESSAGE(FATAL_ERROR "Invalid/unsupported STM32 family: ${STM32_FAMILY}")
ENDIF()

## Generate a target that creates binaries
FUNCTION(STM32_ADD_HEX_BIN_TARGETS TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    ADD_CUSTOM_TARGET(${TARGET}.hex DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex)
    ADD_CUSTOM_TARGET(${TARGET}.bin DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Obinary ${FILENAME} ${FILENAME}.bin)
ENDFUNCTION()

## Generate a target that dumps target info
FUNCTION(STM32_ADD_DUMP_TARGET TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    ADD_CUSTOM_TARGET(${TARGET}.dump DEPENDS ${TARGET} COMMAND ${CMAKE_OBJDUMP} -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
ENDFUNCTION()

## Generate a target that calculates sizing info
FUNCTION(STM32_PRINT_SIZE_OF_TARGETS TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})
ENDFUNCTION()

<<<<<<< HEAD
# Import the specific device family file
STRING(TOLOWER ${STM32_FAMILY} STM32_FAMILY_LOWER)
<<<<<<< HEAD
INCLUDE(${STM32_CMAKE_DIR}/gcc_stm32${STM32_FAMILY_LOWER}.cmake)

STM32_GET_CHIP_TYPE(${STM32_CHIP} STM32_CHIP_TYPExx)
=======
=======
## Validate the STM32_CHIP and import definitions
FUNCTION(STM32_VALIDATE_DEVICE)
    # Find the appropriate device cmake file
    STRING(TOLOWER ${STM32_FAMILY} STM32_FAMILY_LOWER)
    include(${STM32_CMAKE_DIR}/gcc_stm32${STM32_FAMILY_LOWER}.cmake)
    STM32_GET_CHIP_TYPE(${STM32_CHIP} STM32_CHIP_TYPExx)
>>>>>>> 4c5ee3a... Removed build directory tracking. Updated CMakeLists.txt and several of the stm32 cmake tools.

    # Generate the correct device definition for stm32 hal compiler
    string(TOUPPER ${STM32_CHIP} STM32_HAL_DEVICE_DEFINITION)
    string(REPLACE "XX" "xx" STM32_HAL_DEVICE_DEFINITION ${STM32_HAL_DEVICE_DEFINITION})

    # Define a few variables needed to compile HAL in a project
    set_property(GLOBAL PROPERTY STM32_DEVICE_FAMILY "${STM32_FAMILY}")
    set_property(GLOBAL PROPERTY STM32_HAL_DEVICE_DEFINITION "${STM32_HAL_DEVICE_DEFINITION}")
ENDFUNCTION()




>>>>>>> 4a5a2ef... Update
