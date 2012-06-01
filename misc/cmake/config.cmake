list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/misc/3rd/acmake)
list(APPEND CMAKE_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/misc/3rd/cml-1_0_3)
list(APPEND CMAKE_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/misc/3rd/ans)
list(APPEND CMAKE_FRAMEWORK_PATH ${CMAKE_SOURCE_DIR}/misc/3rd/oventoboost)

include(ans.common)
include(3rd)

function(add_executable_ex target files)
    add_executable_with_3rd(${target} "${files}")
    target_link_libraries(${target} common)
    create_vcproj_userfile_detail(${target} ${CMAKE_SOURCE_DIR}/misc/template/common)
endfunction()

function(add_library_ex target files)
    add_library_with_3rd(${target} "${files}")
endfunction()
