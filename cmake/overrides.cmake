# Hand-maintained per-target / per-file tweaks applied after the generated
# cmake/targets.cmake. Keep each entry commented with the reason.
#
# blur_source_options(<target> <file-regex> <option>...) adds compile options
# to the sources of <target> whose path matches <file-regex>. It looks the
# file up in the target's source list so the generated path spelling
# (jam-era capitalisation) does not matter.
function(blur_source_options tgt rx)
    get_target_property(_srcs ${tgt} SOURCES)
    foreach(_s IN LISTS _srcs)
        if(_s MATCHES "${rx}")
            set_property(SOURCE ${_s} APPEND PROPERTY COMPILE_OPTIONS ${ARGN})
        endif()
    endforeach()
endfunction()

# libtomcrypt 1.10: tomcrypt.h defines SHA1/SHA256 descriptor macros, and the
# Windows 11 SDK winnt.h (IMAGE_HOT_PATCH_HASHES) has fields named SHA1/SHA256.
# rng_get_bytes.c includes <windows.h> after tomcrypt.h; force windows.h first.
blur_source_options(libtomcryptv "rng_get_bytes\\.c$" /FIwindows.h)

# libjpeg 6b's jmorecfg.h typedefs INT32 unless XMD_H is defined; the Windows
# SDK (basetsd.h) already has INT32, and JpegHelper includes both.
target_compile_definitions(JpegHelper PRIVATE XMD_H)

# crash reporter for the bring-up phase (writes crash.txt / crash.dmp next to
# the exe). Lived in CMakeLists.txt until the 2026-09-24 split; it is a
# per-target addition, so it belongs here.
target_sources(Amax PRIVATE ${BLUR_TOP}/code/vendor/crash-handler/crash_handler.cpp)

# [Blur2PC] T256: embed the existing source Xbox atlas for dynamic PC prompts.
# A resource keeps the normal single-exe delivery and does not alter retail data.
file(WRITE ${CMAKE_BINARY_DIR}/amax_glyphs.rc
    "BLUR2PC_CONTROLLER_GLYPHS RCDATA \"${BLUR_TOP}/code/mercury/applications/UITestBed/Bundler/Textures/Common/xbox360_buttons.tga\"\n")
target_sources(Amax PRIVATE ${CMAKE_BINARY_DIR}/amax_glyphs.rc)

# [Blur2PC] Retail Blur.exe's icon. Retail carries one icon group, resource 101
# (32x32, 24 bpp), which C_Application loads for the window (ApplicationPC.cpp,
# LoadIcon 101) and Explorer shows for the exe. It was extracted from the user's
# retail install (_work/vendor/retail_icons/extract_icons.py in Blur1PC).
# [Blur2PC][x64] This branch commits the icon beside ApplicationPC.cpp so GitHub
# release builds (.github/workflows/release.yml) carry it too.
set(BLUR_RETAIL_ICON ${BLUR_TOP}/code/mercury/library/gamesys/framework/source/application/blur_101.ico CACHE FILEPATH "Retail Blur.exe icon group 101")
if(EXISTS ${BLUR_RETAIL_ICON})
    file(WRITE ${CMAKE_BINARY_DIR}/amax_icon.rc "101 ICON \"${BLUR_RETAIL_ICON}\"\n")
    target_sources(Amax PRIVATE ${CMAKE_BINARY_DIR}/amax_icon.rc)
else()
    message(WARNING "Retail icon not found at ${BLUR_RETAIL_ICON}; Amax.exe builds without it")
endif()
# [Blur2PC][x64] MinSizeRel is the engine's Master configuration (BUILD_CONFIG_MASTER,
# CFG_GOLD: retail's build, no developer menus/HUD or debug_values.xml). It is the
# player's exe on this branch, so optimise it like Release; CMake's default is /O1.
set(CMAKE_C_FLAGS_MINSIZEREL "/O2 /Ob2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "/O2 /Ob2 /DNDEBUG")
