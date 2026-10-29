cmake_minimum_required(VERSION 3.20)

project(upastation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(upastation
    ps1/cop0/cop0.hpp
    ps1/cop0/cop0.cpp
    ps1/cop0/cop0_exception.cpp
    ps1/cop0/cop0_func.cpp
    ps1/cop0/cop0_rw.cpp

    ps1/cop2/cop2.hpp
    ps1/cop2/cop2.cpp
    ps1/cop2/cop2_func.cpp
    ps1/cop2/cop2_rw.cpp

    ps1/gpu/gpu.cpp
    ps1/gpu/gpu_display.cpp
    ps1/gpu/gpu.hpp
    ps1/gpu/gpu_state.hpp
    ps1/gpu/gpu_rasterise.cpp

    ps1/main/main.cpp

    ps1/memory/dma.hpp
    ps1/memory/dma.cpp
    ps1/memory/memory.hpp
    ps1/memory/memory.cpp
    ps1/memory/memory_io.cpp

    ps1/mips/delay_slot.cpp
    ps1/mips/exception.cpp
    ps1/mips/load_delay.cpp
    ps1/mips/instruction.hpp
    ps1/mips/instruction.cpp
    ps1/mips/mips.hpp
    ps1/mips/mips.cpp
    ps1/mips/mips_testing.cpp
    ps1/mips/mips_rw.cpp
    ps1/mips/mips_exe.cpp

    ps1/misc/utility.hpp
    ps1/misc/utility.cpp

    ps1/ps1/ps1.hpp
    ps1/ps1/ps1_scheduling.cpp

    ps1/test_handler/test_handler.hpp
    ps1/test_handler/test_memory.hpp

    ps1/timers/timer_manager.hpp
    ps1/timers/timer_manager.cpp
    ps1/timers/timer_manager_rw.cpp
    ps1/timers/timer_ext.cpp

    nlohmann/json.hpp
)

target_include_directories(upastation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/main
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/cop0
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/cop2
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/mips
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/ps1
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/gpu
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/test_handler
    ${CMAKE_CURRENT_SOURCE_DIR}/ps1/timers
    ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann
)

find_package(SDL3 REQUIRED CONFIG)
target_link_libraries(
upastation PRIVATE SDL3::SDL3)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(upastation PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-fomit-frame-pointer>
    )

    target_link_options(upastation PRIVATE
        $<$<CONFIG:Release>:-flto>
    )
endif()
