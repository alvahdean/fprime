function(esp32_prepare_idf_wrapper)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/idf-wrapper/CMakeLists.txt")
        return()
    endif()

    set(_wrapper_build_dir "${CMAKE_SOURCE_DIR}/idf-wrapper/build")
    set(_sdkconfig_header "${_wrapper_build_dir}/config/sdkconfig.h")
    set(_include_metadata "${_wrapper_build_dir}/fprime_idf_include_dirs.cmake")
    set(_link_metadata "${_wrapper_build_dir}/fprime_idf_link_info.cmake")

    if (NOT DEFINED ESP32_IDF_FORCE_RECONFIGURE)
        set(ESP32_IDF_FORCE_RECONFIGURE OFF)
    endif()

    if (EXISTS "${_sdkconfig_header}" AND EXISTS "${_include_metadata}" AND EXISTS "${_link_metadata}" AND
        NOT ESP32_IDF_FORCE_RECONFIGURE)
        return()
    endif()

    find_program(ESP32_BASH_PROGRAM bash)
    if (NOT ESP32_BASH_PROGRAM)
        message(FATAL_ERROR "ESP32 wrapper preparation requires bash")
    endif()

    set(_jobs "$ENV{CMAKE_BUILD_PARALLEL_LEVEL}")
    if (NOT _jobs)
        set(_jobs 8)
    endif()

    set(_prepare_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts/prepare_idf_wrapper.sh")
    if (NOT EXISTS "${_prepare_script}")
        message(FATAL_ERROR "Missing ESP32 wrapper preparation script: ${_prepare_script}")
    endif()

    message(STATUS "Preparing ESP-IDF wrapper metadata for ${CMAKE_SOURCE_DIR}")
    execute_process(
        COMMAND "${ESP32_BASH_PROGRAM}" "${_prepare_script}" "${CMAKE_SOURCE_DIR}" "${_jobs}"
        RESULT_VARIABLE _esp32_wrapper_status
        OUTPUT_VARIABLE _esp32_wrapper_stdout
        ERROR_VARIABLE _esp32_wrapper_stderr
    )
    if (NOT _esp32_wrapper_status EQUAL 0)
        message(FATAL_ERROR
            "Failed to prepare ESP-IDF wrapper metadata for ${CMAKE_SOURCE_DIR}\n"
            "stdout:\n${_esp32_wrapper_stdout}\n"
            "stderr:\n${_esp32_wrapper_stderr}"
        )
    endif()
endfunction()
