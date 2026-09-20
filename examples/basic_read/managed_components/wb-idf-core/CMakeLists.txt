if(NOT DEFINED PROJECT_NAME)
    get_filename_component(WB_IDF_CORE_DIR "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
    
    # Register wb-idf-core itself as a component (for root Kconfig)
    list(APPEND EXTRA_COMPONENT_DIRS "${WB_IDF_CORE_DIR}")

    # Automatically register all subdirectories that contain a CMakeLists.txt as components
    file(GLOB children RELATIVE "${WB_IDF_CORE_DIR}" "${WB_IDF_CORE_DIR}/*")
    foreach(child ${children})
        if(IS_DIRECTORY "${WB_IDF_CORE_DIR}/${child}")
            if(EXISTS "${WB_IDF_CORE_DIR}/${child}/CMakeLists.txt")
                # message(STATUS "Registering wb-idf component: ${child}")
                list(APPEND EXTRA_COMPONENT_DIRS "${WB_IDF_CORE_DIR}/${child}")
            endif()
        endif()
    endforeach()

    return()
endif()

idf_component_register(KCONFIG "${CMAKE_CURRENT_LIST_DIR}/Kconfig.projbuild")

