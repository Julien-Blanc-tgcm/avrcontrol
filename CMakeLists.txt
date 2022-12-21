cmake_minimum_required(VERSION 3.10)

project(avrcontrol CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Network REQUIRED)

set(ENABLE_TESTS ON CACHE BOOL "Enable compilation of tests")

set(CMAKE_AUTOMOC ON)

if (${ENABLE_TESTS})
	enable_testing()
	find_package(Qt5Test)
endif()
set(sources
	"${CMAKE_CURRENT_SOURCE_DIR}/src/AVRDevice.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteProperty.cpp"
)

set(headers
	"${CMAKE_CURRENT_SOURCE_DIR}/src/marantzuart.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/AVRDevice.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/RemoteProperty.hpp"
)

add_library(avrcontrol ${sources} ${headers})

target_link_libraries(avrcontrol PRIVATE Qt5::Core Qt5::Network)

install(TARGETS avrcontrol DESTINATION lib)
install(FILES ${headers} DESTINATION include/jbc/avrcontrol)

if(${Qt5Test_FOUND})
	add_executable(test_parser tests/test_parser.cpp)
	target_include_directories(test_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
	add_test(test_parser test_parser)
	target_link_libraries(test_parser Qt5::Test )
	add_executable(test_commands tests/test_commands.cpp)
	target_include_directories(test_commands PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
	add_test(test_commands test_commands)
	target_link_libraries(test_commands Qt5::Test )
endif()
