# [Blur2PC] T272: keep the tested upstream implementation replaceable at runtime.
option(BLUR_WITH_INPUTFUSION "Bundle InputFusion controller support" ${BLUR_X64})
if(BLUR_WITH_INPUTFUSION)
    if(NOT BLUR_X64)
        message(FATAL_ERROR "The pinned InputFusion backend is x64; configure -A x64 or -DBLUR_WITH_INPUTFUSION=OFF")
    endif()
    set(BLUR_INPUTFUSION_DLL "${BLUR_VENDOR}/inputfusion-0.4.1/BlurInputFusion.dll"
        CACHE FILEPATH "Verified InputFusion 0.4.1 x64 release DLL")
    if(NOT EXISTS "${BLUR_INPUTFUSION_DLL}")
        message(FATAL_ERROR "InputFusion is missing. Run powershell -File tools/inputfusion/setup.ps1, or configure -DBLUR_WITH_INPUTFUSION=OFF")
    endif()
    file(SHA256 "${BLUR_INPUTFUSION_DLL}" _blur_inputfusion_hash)
    if(NOT _blur_inputfusion_hash STREQUAL "242e16a3a5904eb9068bd442c6a466fd983dbbe7851022704985dce3fb6ec408")
        message(FATAL_ERROR "InputFusion DLL differs from the pinned 0.4.1 x64 release")
    endif()
    target_compile_definitions(input PRIVATE BLUR_WITH_INPUTFUSION=1)
    add_custom_command(TARGET Amax POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${BLUR_INPUTFUSION_DLL}" "$<TARGET_FILE_DIR:Amax>/BlurInputFusion.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BLUR_TOP}/code/vendor/inputfusion/notices" "$<TARGET_FILE_DIR:Amax>/InputFusion-Licenses"
        VERBATIM)
endif()
