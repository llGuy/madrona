namespace madrona {
namespace cu {

void *allocGPU(size_t num_bytes)
{
    void *ptr;
    REQ_CUDA(cudaMalloc(&ptr, num_bytes));

    return ptr;
}

void deallocGPU(void *ptr)
{
    REQ_CUDA(cudaFree(ptr));
}

void *allocStaging(size_t num_bytes)
{
    void *ptr;
    REQ_CUDA(cudaHostAlloc(&ptr, num_bytes,
                           cudaHostAllocMapped | cudaHostAllocWriteCombined));

    return ptr;
}

void *allocReadback(size_t num_bytes)
{
    void *ptr;
    REQ_CUDA(cudaHostAlloc(&ptr, num_bytes,
                           cudaHostAllocMapped));

    return ptr;
}

void deallocCPU(void *ptr)
{
    REQ_CUDA(cudaFreeHost(ptr));
}

void cpyCPUToGPU(cudaStream_t strm, void *gpu, void *cpu, size_t num_bytes)
{
    REQ_CUDA(cudaMemcpyAsync(gpu, cpu, num_bytes, cudaMemcpyHostToDevice,
                             strm));
}

void cpyGPUToCPU(cudaStream_t strm, void *cpu, void *gpu, size_t num_bytes)
{
    REQ_CUDA(cudaMemcpyAsync(cpu, gpu, num_bytes, cudaMemcpyDeviceToHost,
                             strm));
}

cudaStream_t makeStream()
{
    cudaStream_t strm;
    REQ_CUDA(cudaStreamCreate(&strm));

    return strm;
}

void checkCuda(cudaError_t res, const char *file,
               int line, const char *funcname) noexcept
{
    if (res != cudaSuccess) {
        cudaError(res, file, line, funcname);
    }
}

void checkCuDrv(CUresult res, const char *file,
                int line, const char *funcname) noexcept
{
    if (res != CUDA_SUCCESS) {
        cuDrvError(res, file, line, funcname);
    }
}


void checkNVRTC(nvrtcResult res, const char *file,
                int line, const char *funcname) noexcept
{
    if (res != NVRTC_SUCCESS) {
        nvrtcError(res, file, line, funcname);
    }
}

}
}
