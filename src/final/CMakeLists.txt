file(GLOB_RECURSE FILES *.hpp *.cpp)
set(TARGET final)
add_executable_ex(${TARGET} "${FILES}")
