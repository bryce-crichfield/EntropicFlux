cmake_minimum_required(VERSION 3.16)

# Project Name
set (PROJECT_NAME EntropicFlux)
project(${PROJECT_NAME})

# Library Locations
if (NOT DEFINED LIBDAISY_DIR)
    # Default to relative paths if not defined
    message(STATUS "Using default libDaisy directory: ${LIBDAISY_DIR}")
    set(LIBDAISY_DIR "../libDaisy")
endif ()


#add_subdirectory(../libDaisy ${CMAKE_BINARY_DIR}/libDaisy)

# Glob the source files from the /src directory
file(GLOB_RECURSE CPP_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# C Sources
#set(C_SOURCES
#        ${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_f32.c
#        ${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_f32.c
#        ${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c
#        ${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Source/ControllerFunctions/arm_sin_cos_f32.c
#)

# Include directories
#include_directories(${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Include)


# Add compile definitions
#add_definitions(-DARM_MATH_CM7)

# Create executable
add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${C_SOURCES_SRC}
        src/main.cpp
        src/app/IApplication.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# You'll need to add target-specific compiler flags and linking for your embedded target
# This basic structure converts the Makefile variables but requires additional
# ARM/embedded-specific CMake configuration