cmake_minimum_required(VERSION 2.8)

project(fcontext)

# FCTX_ARCH: arm arm64 i386 mips32 ppc32 ppc64 x86_64
# FCTX_PLATFORM: aapcs ms sysv o32
# FCTX_COFF: elf pe macho xcoff 
# FCTX_ASM_COMPILER: gas masm armasm
# FCTX_ASM_SOURCE_EXTENSION: .S .asm

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64" 
    OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64"
    OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386"
    OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(FCTX_ARCH x86_64)
    else()
        set(FCTX_ARCH i386)
    endif()
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    if (CMAKE_CL_64)
        set(FCTX_ARCH arm64)
    else()
        set(FCTX_ARCH arm)
    endif()
else()
    message(FATAL_ERROR "arch ${CMAKE_SYSTEM_PROCESSOR} not supported")
endif()

if (WIN32)
    set(FCTX_PLATFORM ms)
    set(FCTX_COFF pe)
    set(FCTX_ASM_SOURCE_EXTENSION .asm)

    if (MINGW)
        set(CMAKE_ASM_COMPILER as)
        set(FCTX_ASM_COMPILER gas)
        enable_language(ASM)
    else()
        set(FCTX_ASM_COMPILER masm)
        enable_language(ASM_MASM)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    endif()
else()
    set(FCTX_PLATFORM sysv)
    set(FCTX_COFF elf)
    set(FCTX_ASM_SOURCE_EXTENSION .S)
    set(FCTX_ASM_COMPILER gas)
    enable_language(ASM)
endif()

set(FCTX_SOURCE_SUFFIX ${FCTX_ARCH}_${FCTX_PLATFORM}_${FCTX_COFF}_${FCTX_ASM_COMPILER}${FCTX_ASM_SOURCE_EXTENSION})
set(SRC asm/make_${FCTX_SOURCE_SUFFIX} asm/jump_${FCTX_SOURCE_SUFFIX} asm/ontop_${FCTX_SOURCE_SUFFIX})

add_definitions(-DBOOST_CONTEXT_EXPORT)
add_library(fcontext fcontext.h ${SRC})

add_executable(fcontext_test test.c)
target_link_libraries(fcontext_test PRIVATE fcontext)