include_directories(common)

list_subdirectories(DIRS ${CMAKE_CURRENT_SOURCE_DIR} TRUE)

foreach(DIR ${DIRS})
    message("subdirectory: ${DIR}")
    add_subdirectory(${DIR})
endforeach()
