file(GLOB_RECURSE FILES *.cpp *.hpp)
set(TARGET homework.3)
add_executable_ex(${TARGET} "${FILES}")
