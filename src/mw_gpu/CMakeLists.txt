add_library(madrona_mw_gpu STATIC
    cuda_utils.hpp cuda_utils.inl cuda_utils.cpp
    cpp_compile.hpp cpp_compile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/madrona/mw_gpu.hpp entry.cpp)

target_link_libraries(madrona_mw_gpu PUBLIC
    madrona_core CUDA::nvrtc CUDA::cudart)

file(GLOB CUDA_DEVICE_RUNTIME "${CUDAToolkit_LIBRARY_DIR}/*cudadevrt*")

set(MADRONA_MW_GPU_COMPILE_FLAGS
    -std=c++20
    -default-device
    -rdc=true
    -use_fast_math
    -lineinfo
    -dopt=on
    -DMADRONA_GPU_MODE=1
)

set(DEVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/device)
set(DEVICE_SOURCES
    ${DEVICE_SRC_DIR}/job.cpp
    ${DEVICE_SRC_DIR}/state.cpp
    ${DEVICE_SRC_DIR}/crash.cpp
)
    
set(INTERNAL_SRC_LIST)
foreach(f ${DEVICE_SOURCES})
    set(INTERNAL_SRC_LIST "${INTERNAL_SRC_LIST}\"${f}\", ")
endforeach()
set(INTERNAL_SRC_LIST "${INTERNAL_SRC_LIST}")

get_target_property(MADRONA_CORE_INCLUDES madrona_core INCLUDE_DIRECTORIES)
get_target_property(MADRONA_MW_GPU_INCLUDES madrona_mw_gpu INCLUDE_DIRECTORIES)

set(NVRTC_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/device/include
    ${MADRONA_MW_GPU_INCLUDES}
    ${MADRONA_CORE_INCLUDES}
    ${CUDAToolkit_INCLUDE_DIRS}
)

set(NVRTC_OPTIONS)
foreach(f ${NVRTC_INCLUDE_DIRS})
    set(NVRTC_OPTIONS "${NVRTC_OPTIONS}\"-I${f}\", ")
endforeach()
foreach(f ${MADRONA_MW_GPU_COMPILE_FLAGS})
    set(NVRTC_OPTIONS "${NVRTC_OPTIONS}\"${f}\", ")
endforeach()
set(NVRTC_OPTIONS "${NVRTC_OPTIONS}")

target_compile_definitions(madrona_mw_gpu PRIVATE
    MADRONA_NVRTC_OPTIONS=${NVRTC_OPTIONS}
    MADRONA_MW_GPU_INTERNAL_CPP=${INTERNAL_SRC_LIST}
    MADRONA_CUDADEVRT_PATH=\"${CUDA_DEVICE_RUNTIME}\"
)

# Helper function for library users to get list of sources / compileflags
function(madrona_build_compile_defns)
    set(one_val_args OUT_TARGET SOURCES_DEFN FLAGS_DEFN)
    set(multi_val_args SRC_TARGETS CUSTOM_FLAGS)

    cmake_parse_arguments(M "" "${one_val_args}" "${multi_val_args}"
                          ${ARGN})

    set(ALL_TARGET_SOURCES "")
    set(ALL_TARGET_INCLUDES "")
    foreach(t ${M_SRC_TARGETS})
        get_target_property(TARGET_DIR ${t} SOURCE_DIR) 

        get_target_property(TARGET_SOURCES ${t} SOURCES) 
        list(FILTER TARGET_SOURCES INCLUDE REGEX ".+\.cpp")
        list(TRANSFORM TARGET_SOURCES REPLACE "${TARGET_DIR}" "")
        list(TRANSFORM TARGET_SOURCES PREPEND "${TARGET_DIR}/")
        list(APPEND ALL_TARGET_SOURCES ${TARGET_SOURCES})

        get_target_property(TARGET_INCLUDES ${t} INCLUDE_DIRECTORIES) 
        list(APPEND ALL_TARGET_INCLUDES ${TARGET_INCLUDES})

        get_target_property(TARGET_INTERFACE_INCLUDES ${t}
            INTERFACE_INCLUDE_DIRECTORIES) 
        list(APPEND ALL_TARGET_INCLUDES ${TARGET_INTERFACE_INCLUDES})
    endforeach()

    set(SRC_LIST)
    foreach(src ${ALL_TARGET_SOURCES})
        set(SRC_LIST "${SRC_LIST}\"${src}\", ")
    endforeach()
    set(SRC_LIST "${SRC_LIST}")

    set(ALL_FLAGS)
    foreach(i ${ALL_TARGET_INCLUDES})
        set(ALL_FLAGS "${ALL_FLAGS}\"-I${i}\", ")
    endforeach()

    foreach(i ${M_CUSTOM_FLAGS})
        set(ALL_FLAGS "${ALL_FLAGS}\"${i}\", ")
    endforeach()

    set(ALL_FLAGS "${ALL_FLAGS}")

    add_library(${M_OUT_TARGET} INTERFACE)
    
    target_compile_definitions(${M_OUT_TARGET} INTERFACE
        ${M_SOURCES_DEFN}=${SRC_LIST}
        ${M_FLAGS_DEFN}=${ALL_FLAGS}
    )
endfunction()
