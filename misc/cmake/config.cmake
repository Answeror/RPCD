#file(TO_CMAKE_PATH $ENV{CODE} CODE_HOME)
#if(NOT CODE_HOME)
#    message(FATAL_ERROR "Please define CODE enviounment variable first.")
#endif()
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CODE_HOME}/cmake)

#include(qt_support)
#include(set_output_dir)
#include(disable_msvc_warnings)
#include(create_vcproj_userfile)
#include(list_subdirectories)
#include(3rd)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/misc/3rd/cmake);

include(ans.common)
include(3rd)

function(add_executable_ex target files)
    add_executable_with_3rd(${target} "${files}")
    create_vcproj_userfile_detail(${target} ${CMAKE_SOURCE_DIR}/misc/template/common)
endfunction()

function(add_library_ex target files)
    add_library_with_3rd(${target} "${files}")
endfunction()
