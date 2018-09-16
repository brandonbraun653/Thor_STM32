set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
    if(EXISTS "$ENV{CMAKE_GNU_ARM_EMBEDDED_TOOLCHAIN}")
        file(TO_CMAKE_PATH "$ENV{CMAKE_GNU_ARM_EMBEDDED_TOOLCHAIN}" WIN32_TOOLCHAIN_DIR)
    else()
        message(FATAL_ERROR "Could not find environment variable [CMAKE_GNU_ARM_EMBEDDED_TOOLCHAIN]. Please set this to the GNU ARM bin directory.")
    endif()

    if(${CMAKE_VERSION} VERSION_LESS 3.6.0)
        include(CMakeForceCompiler)
        CMAKE_FORCE_C_COMPILER("${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe" GNU)
        CMAKE_FORCE_CXX_COMPILER("${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-g++.exe" GNU)
    else()
        set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
        set(CMAKE_C_COMPILER "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe")
        set(CMAKE_CXX_COMPILER "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-g++.exe")
    endif()

    set(CMAKE_OBJCOPY "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-objcopy.exe")
    set(CMAKE_OBJDUMP "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-objdump.exe")
    set(CMAKE_SIZE    "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-size.exe")
    set(CMAKE_DEBUGER "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-gdb.exe")
    set(CMAKE_CPPFILT "${WIN32_TOOLCHAIN_DIR}/arm-none-eabi-c++filt.exe")

elseif(UNIX)
    set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
    set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
    set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
    set(CMAKE_OBJDUMP "arm-none-eabi-objdump")
    set(CMAKE_SIZE    "arm-none-eabi-size")
    set(CMAKE_DEBUGER "arm-none-eabi-gdb")
    set(CMAKE_CPPFILT "arm-none-eabi-c++filt")
else()
    message(FATAL_ERROR "Unsupported OS type!")
endif()

set(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(NOT TARGET debug)
    ADD_CUSTOM_TARGET(debug
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
        COMMENT "Creating the executable in the debug mode."
    )
endif()

if(NOT TARGET release)
    ADD_CUSTOM_TARGET(release
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
        COMMENT "Creating the executable in the release mode."
    )
endif()
