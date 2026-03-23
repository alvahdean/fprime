function(esp32_prepare_idf_wrapper)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/idf-wrapper/CMakeLists.txt")
        return()
    endif()

    set(_wrapper_build_dir "${CMAKE_SOURCE_DIR}/idf-wrapper/build")
    set(_sdkconfig_defaults "${CMAKE_SOURCE_DIR}/idf-wrapper/sdkconfig.defaults")
    set(_sdkconfig_header "${_wrapper_build_dir}/config/sdkconfig.h")
    set(_include_metadata "${_wrapper_build_dir}/fprime_idf_include_dirs.cmake")
    set(_link_metadata "${_wrapper_build_dir}/fprime_idf_link_info.cmake")

    if (NOT DEFINED ESP32_IDF_FORCE_RECONFIGURE)
        set(ESP32_IDF_FORCE_RECONFIGURE OFF)
    endif()

    set(_needs_prepare OFF)
    if (EXISTS "${_sdkconfig_header}" AND EXISTS "${_include_metadata}" AND EXISTS "${_link_metadata}" AND
        NOT ESP32_IDF_FORCE_RECONFIGURE)
        if (EXISTS "${_sdkconfig_defaults}" AND "${_sdkconfig_defaults}" IS_NEWER_THAN "${_sdkconfig_header}")
            set(_needs_prepare ON)
        endif()
    else()
        set(_needs_prepare ON)
    endif()

    if (NOT _needs_prepare)
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

function(esp32_register_flash_image_target deployment_target)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/idf-wrapper/CMakeLists.txt")
        return()
    endif()

    find_program(ESP32_BASH_PROGRAM bash)
    if (NOT ESP32_BASH_PROGRAM)
        message(FATAL_ERROR "ESP32 flash image packaging requires bash")
    endif()

    set(_build_flash_image_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts/build_flash_image.sh")
    if (NOT EXISTS "${_build_flash_image_script}")
        message(FATAL_ERROR "Missing ESP32 flash image packaging script: ${_build_flash_image_script}")
    endif()

    set(_flash_manifest "${CMAKE_SOURCE_DIR}/idf-wrapper/build/fprime_flash_artifacts.json")

    add_custom_command(TARGET "${deployment_target}" POST_BUILD
        COMMAND "${ESP32_BASH_PROGRAM}" "${_build_flash_image_script}" "${CMAKE_SOURCE_DIR}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Packaging ESP32 flash image for ${deployment_target}"
        VERBATIM
    )

    add_custom_target(esp32_flash_image
        COMMAND "${ESP32_BASH_PROGRAM}" "${_build_flash_image_script}" "${CMAKE_SOURCE_DIR}"
        BYPRODUCTS "${_flash_manifest}"
        DEPENDS "${deployment_target}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Packaging ESP32 flash image for ${deployment_target}"
        VERBATIM
    )
endfunction()
