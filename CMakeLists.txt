cmake_minimum_required(VERSION 3.20)
project(Poisson2Hip LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(fmt)

add_executable(my_app src/main.cpp src/poisson.cpp)

target_link_libraries(my_app PRIVATE fmt::fmt-header-only)

target_compile_options(my_app PRIVATE -xhip)
target_compile_definitions(my_app PRIVATE __HIP_PLATFORM_AMD__)

target_include_directories(my_app
	PRIVATE
		${CMAKE_SOURCE_DIR}    # project root
		${CMAKE_SOURCE_DIR}/inc
		${CMAKE_SOURCE_DIR}/hip-templates
)

get_target_property(FMT_INCLUDES fmt::fmt-header-only INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(my_app SYSTEM PRIVATE
	${FMT_INCLUDES}               # fmt headers
	/opt/rocm/hip/include         # HIP headers
)

target_compile_options(my_app PRIVATE
	-Wall
	-Wextra
	-Wno-unused-parameter
	-Wfloat-equal
	-Wnon-virtual-dtor
	-Wcast-align
	-Wzero-as-null-pointer-constant
	-Wunused
	-Woverloaded-virtual
	-Wmisleading-indentation
	-Wnull-dereference
)

target_compile_options(my_app PRIVATE
	-Werror=format-security
	-Werror=missing-field-initializers
	-Werror=return-type
	-Werror=conversion
	-Werror=sign-conversion
	-Werror=float-conversion
)
