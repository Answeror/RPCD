file(GLOB FILES *.cpp *.hpp)
add_executable_ex(homework.1.1 "${FILES}")
target_link_libraries(homework.1.1 common)
