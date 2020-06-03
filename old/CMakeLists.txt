

project(old_conversion)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES nasoq_driver.cpp)
add_executable(old_conversion ${SOURCE_FILES} nasoq_driver.cpp)
