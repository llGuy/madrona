set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/madrona")
add_library(madrona_common STATIC
    ${INC_DIR}/memory.hpp ${INC_DIR}/memory.inl memory.cpp
    ${INC_DIR}/heap_array.hpp
    ${INC_DIR}/span.hpp
    ${INC_DIR}/utils.hpp
    ${INC_DIR}/crash.hpp crash.cpp
    ${INC_DIR}/ecs.hpp ${INC_DIR}/ecs.inl
    ${INC_DIR}/type_tracker.hpp ${INC_DIR}/type_tracker.inl type_tracker.cpp
    ${INC_DIR}/hashmap.hpp ${INC_DIR}/hashmap.inl hashmap.cpp
    ${INC_DIR}/table.hpp ${INC_DIR}/table.inl table.cpp
    ${INC_DIR}/virtual.hpp virtual.cpp
    #${INC_DIR}/hash.hpp
    #${INC_DIR}/platform_utils.hpp ${INC_DIR}/platform_utils.inl
    #    platform_utils.cpp
)

# Disable exceptions & RTTI
set(FRONTEND_CLANG FALSE)
set(FRONTEND_GCC FALSE)
set(FRONTEND_MSVC FALSE)

if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(FRONTEND_CLANG TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(FRONTEND_GCC TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(FRONTEND_MSVC TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(FRONTEND_MSVC TRUE)
    else ()
        set(FRONTEND_CLANG TRUE)
    endif ()
endif ()

if (FRONTEND_GCC OR FRONTEND_CLANG)
    target_compile_options(madrona_common PUBLIC
        -fno-exceptions -fno-rtti)
elseif (FRONTEND_MSVC)
    # Disabling exceptions in MSVC seems painful
    target_compile_options(madrona_common PUBLIC
        /GR-)
else ()
    message(FATAL_ERROR "Unsupported compiler frontend")
endif()

target_link_libraries(madrona_common
    PUBLIC 
        madrona_sys_defns
    PRIVATE
)

target_include_directories(madrona_common
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
)
