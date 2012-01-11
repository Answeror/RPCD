## find 3rd libs, including OpenCV and Boost
#function(find_3rd)
#    find_package(OpenCV REQUIRED)
#    # only use head version
#    find_package(Boost REQUIRED)
#endfunction()

# qt, opencv, boost support
macro(add_executable_with_3rd target files)
    # my lib
    find_package(Ans REQUIRED)
    include_directories(${ANS_INCLUDE_DIRS})
    # opencv
    find_package(OpenCV REQUIRED)
    # qt
    qt_support(qt_files qt_libs)
    add_executable(${target} ${files} ${qt_files})
    target_link_libraries(${target} ${qt_libs} ${OpenCV_LIBS})
    # boost, oven to boost, boost dynamic any
    boost_support(${target})

    #qwt added on 2012-01-06
    qwt_support(${target})

    #2d/3d math
    cml_support(${target})
endmacro()

# qt, opencv, boost support
macro(add_library_with_3rd target files)
    # my lib
    find_package(Ans REQUIRED)
    include_directories(${ANS_INCLUDE_DIRS})
    # opencv
    find_package(OpenCV REQUIRED)
    # qt
    qt_support(qt_files qt_libs)
    add_library(${target} STATIC ${files} ${qt_files})
    # boost, oven to boost, boost dynamic any
    boost_support(${target})

    #qwt added on 2012-01-06
    qwt_support(${target})

    #2d/3d math
    cml_support(${target})
endmacro()
