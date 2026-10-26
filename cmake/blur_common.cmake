# Shared build description for every phase (Blur1PC, Blur1Multi, BlurX).
# [Blur2PC] Split out of CMakeLists.txt on 2026-09-24 so every phase shares it.
# Since 2026-09-25 (D16) each phase is a git branch checked out in its own
# worktree, so BLUR_TOP and BLUR_PROJECT are both the worktree root. The
# including CMakeLists.txt sets:
#   BLUR_TOP        root of the source tree (contains code/)
#   BLUR_PROJECT    the phase project that owns _work/ and cmake/targets.cmake
#   BLUR_WORKSPACE  the container of the phase worktrees; Blur1PC/_work/vendor
#                   there holds the third-party SDKs every phase uses
# Everything below is the build as it was in Blur1PC/CMakeLists.txt.

if(NOT MSVC)
    message(FATAL_ERROR "MSVC only (the source is Win32/D3D9 MSVC code)")
endif()
# [Blur2PC][x64] The x64 child branch configures with -A x64; -A Win32 still
# builds the original x86 game. BLUR_X64 selects the matching vendor libraries.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BLUR_X64 ON)
    set(_blur_arch_x86_x64 x64)
    set(_blur_arch_openal Win64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BLUR_X64 OFF)
    set(_blur_arch_x86_x64 x86)
    set(_blur_arch_openal Win32)
else()
    message(FATAL_ERROR "Configure with -A x64 or -A Win32")
endif()

# The third-party SDKs live in Blur1PC/_work/vendor (untracked, so the other
# phases cannot inherit them by merge); a phase's own _work/vendor wins if present.
if(EXISTS ${BLUR_PROJECT}/_work/vendor)
    set(_blur_vendor_default ${BLUR_PROJECT}/_work/vendor)
else()
    set(_blur_vendor_default ${BLUR_WORKSPACE}/Blur1PC/_work/vendor)
endif()
set(BLUR_VENDOR ${_blur_vendor_default} CACHE PATH "Downloaded third-party SDKs (shared by every phase)")

set(BLUR_DXSDK_INCLUDE ${BLUR_VENDOR}/d3dx/build/native/include CACHE PATH "d3dx9.h location")
set(BLUR_D3DX9_LIB ${BLUR_VENDOR}/d3dx/build/native/release/lib/${_blur_arch_x86_x64}/d3dx9.lib CACHE FILEPATH "d3dx9.lib")
set(BLUR_OPENAL_ROOT ${BLUR_VENDOR}/openal-soft-1.24.3-bin CACHE PATH "OpenAL Soft binary distribution")
set(BLUR_OPENAL_INCLUDE ${BLUR_OPENAL_ROOT}/include ${BLUR_OPENAL_ROOT}/include/AL ${BLUR_TOP}/code/vendor/efx-util)
set(BLUR_OPENAL_LIB ${BLUR_OPENAL_ROOT}/libs/${_blur_arch_openal}/OpenAL32.lib CACHE FILEPATH "OpenAL32.lib")
set(BLUR_JPEG_DIR ${BLUR_VENDOR}/jpeg-6b CACHE PATH "IJG libjpeg 6b sources")

# ---- global compiler settings -------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# /Zi + /DEBUG in every configuration: this is a port in progress and crash
# addresses have to resolve to source lines.
add_link_options(/DEBUG)
add_compile_options(/MP /W3 /permissive /Zc:wchar_t /EHsc /GR /bigobj /Zi
    /wd4996 /wd4244 /wd4267 /wd4305 /wd4800 /wd4018 /wd4310 /wd4838)
# [Blur2PC][x64] A pointer cast to a 32-bit integer loses the upper half on x64
# (C4311 and C4302 each catch casts the other misses).
if(BLUR_X64)
    add_compile_options(/we4311 /we4302)
endif()
# No global WIN32_LEAN_AND_MEAN / _WINSOCKAPI_: the original build relied on
# <windows.h> bringing winsock.h (IN_ADDR etc. in WsSocket.h), and _WINSOCKAPI_
# makes curl.h skip winsock2.h. Winsock ordering conflicts are fixed per file.
# PLATFORM_PC must come from the command line (the vcproj did that): the PCH
# headers test it before they include core/Core.h, which is the only place it
# is otherwise derived from _WIN32.
add_compile_definitions(
    PLATFORM_PC WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE _WINSOCK_DEPRECATED_NO_WARNINGS
    _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    $<$<CONFIG:Debug>:BUILD_CONFIG_DEBUG>
    $<$<CONFIG:Release>:BUILD_CONFIG_SPEED>
    $<$<CONFIG:RelWithDebInfo>:BUILD_CONFIG_RELEASE>
    $<$<CONFIG:MinSizeRel>:BUILD_CONFIG_MASTER>
)
# shims that replace SDK pieces we do not have (dxerr, Bink) or that were proprietary (EFX-Util)
include_directories(${BLUR_TOP}/code/vendor/dxerr ${BLUR_TOP}/code/vendor/bink-stub ${BLUR_TOP}/code/mercury/library/common/core/include)

# ---- libjpeg 6b ------------------------------------------------------------------
if(EXISTS ${BLUR_JPEG_DIR}/jpeglib.h)
    if(NOT EXISTS ${BLUR_JPEG_DIR}/jconfig.h)
        configure_file(${BLUR_JPEG_DIR}/jconfig.vc ${BLUR_JPEG_DIR}/jconfig.h COPYONLY)
    endif()
    set(JPEG_SRC jcapimin jcapistd jccoefct jccolor jcdctmgr jchuff jcinit jcmainct jcmarker jcmaster jcomapi
        jcparam jcphuff jcprepct jcsample jctrans jdapimin jdapistd jdatadst jdatasrc jdcoefct jdcolor jddctmgr
        jdhuff jdinput jdmainct jdmarker jdmaster jdmerge jdphuff jdpostct jdsample jdtrans jerror jfdctflt
        jfdctfst jfdctint jidctflt jidctfst jidctint jidctred jquant1 jquant2 jutils jmemmgr jmemnobs)
    list(TRANSFORM JPEG_SRC PREPEND ${BLUR_JPEG_DIR}/)
    list(TRANSFORM JPEG_SRC APPEND .c)
    add_library(blur_jpeg STATIC ${JPEG_SRC})
    # JpegHelper includes <jpeg/jpeglib.h> (the vendor tree had include/jpeg/); mirror that layout
    foreach(h jpeglib.h jconfig.h jmorecfg.h jerror.h)
        configure_file(${BLUR_JPEG_DIR}/${h} ${CMAKE_BINARY_DIR}/vendor_include/jpeg/${h} COPYONLY)
    endforeach()
    target_include_directories(blur_jpeg PUBLIC ${BLUR_JPEG_DIR} ${CMAKE_BINARY_DIR}/vendor_include)
    set(BLUR_JPEG_DIR ${CMAKE_BINARY_DIR}/vendor_include)
    set_target_properties(blur_jpeg PROPERTIES FOLDER "vendor")
else()
    message(WARNING "libjpeg 6b not found at ${BLUR_JPEG_DIR}; JpegHelper will not link")
    add_library(blur_jpeg INTERFACE)
endif()

# ---- EFX-Util shim (Creative's reverb preset conversion) ---------------------------
add_library(blur_shims STATIC
    ${BLUR_TOP}/code/vendor/efx-util/EFX-Util.cpp
    ${BLUR_TOP}/code/vendor/bink-stub/bink_stub.cpp
    ${BLUR_TOP}/code/vendor/crt-compat/crt_compat.cpp)
target_include_directories(blur_shims PUBLIC ${BLUR_OPENAL_INCLUDE})
set_target_properties(blur_shims PROPERTIES FOLDER "vendor")

# ---- system / vendor link inputs for the executable ------------------------------
set(BLUR_SYSTEM_LIBS
    secur32 crypt32 ws2_32 dbghelp psapi winmm
    dxguid dinput8 xinput    # [Blur2PC][x64] no d3d9/d3dx9: D3D11 is the only renderer
    dsound    # [Blur2PC] T144: retail PC's audio output engine (engine_pc.cpp) is DirectSound
    ${BLUR_OPENAL_LIB}
    blur_shims
    legacy_stdio_definitions
)
if(BLUR_X64)
    # [Blur2PC][x64] DemonWare ships x86 libraries only; build the same libraries
    # from the bundled sources (cmake/demonware.cmake, generated from the
    # Release-mt|Win32 projects) against an x64 OpenSSL 0.9.8i built by
    # tools/demonware_cmake/build_openssl_x64.cmd.
    set(BLUR_OPENSSL_X64 ${BLUR_PROJECT}/_work/vendor_x64/openssl-x64 CACHE PATH "x64 OpenSSL 0.9.8i install (build_openssl_x64.cmd)")
    if(NOT EXISTS ${BLUR_OPENSSL_X64}/lib/libeay32.lib)
        message(FATAL_ERROR "x64 OpenSSL 0.9.8i missing at ${BLUR_OPENSSL_X64}; run tools/demonware_cmake/build_openssl_x64.cmd")
    endif()
    include(${BLUR_TOP}/cmake/demonware.cmake)
    foreach(lib bdConnection bdCore bdLobby bdSocket bdPlatform bdNet bdExampleCommonLobby)
        list(APPEND BLUR_SYSTEM_LIBS dw_${lib})
    endforeach()
    list(APPEND BLUR_SYSTEM_LIBS ${BLUR_OPENSSL_X64}/lib/ssleay32.lib ${BLUR_OPENSSL_X64}/lib/libeay32.lib dw_libcurl)
else()
set(BLUR_DEMONWARE_LIBDIR ${BLUR_TOP}/code/vendor/demonware/2.0/lib/win32-vs8)
foreach(lib bdConnection bdCore bdLobby bdSocket bdPlatform bdNet bdExampleCommonLobby ssleay32 libeay32 libcurl)
    list(APPEND BLUR_SYSTEM_LIBS
        $<IF:$<CONFIG:Debug>,${BLUR_DEMONWARE_LIBDIR}/debug-mt/${lib}.lib,${BLUR_DEMONWARE_LIBDIR}/release-mt/${lib}.lib>)
endforeach()
endif()
