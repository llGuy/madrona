#pragma once

#if defined(__x86_64__)
#define MADRONA_X64 (1)
#elif defined (__arm64__)
#define MADRONA_ARM (1)
#else
#error "Unsupported architecture"
#endif

#if defined(_MSC_VER)
#define MADRONA_MSVC (1)
#elif defined(__clang__)
#define MADRONA_CLANG (1)
#elif defined(__GNUC__)
#define MADRONA_GCC (1)
#else
#error "Unsupported compiler"
#endif

#define MADRONA_STRINGIFY_HELPER(m) #m
#define MADRONA_STRINGIFY(m) MADRONA_STRINGIFY_HELPER(m)

#define MADRONA_LOC_APPEND(m) m ": " __FILE__ " @ " MADRONA_STRINGIFY(__LINE__)

#if defined(__clang__) or defined(__GNUC__) or defined(__CUDACC__)
#define MADRONA_COMPILER_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define MADRONA_COMPILER_FUNCTION_NAME __FUNCSIG__
#endif

#ifdef MADRONA_MW_MODE
#define MADRONA_MW_COND(...) __VA_ARGS__
#else
#define MADRONA_MW_COND(...)
#endif

#ifdef MADRONA_X64
#define MADRONA_CACHE_LINE (64)
#elif defined(MADRONA_ARM) && defined(MADRONA_MACOS)
#define MADRONA_CACHE_LINE (128)
#else
#define MADRONA_CACHE_LINE (64)
#endif

#if defined(MADRONA_MSVC)
#define MADRONA_ALWAYS_INLINE [[msvc::forceinline]]
#define MADRONA_NO_INLINE __declspec(noinline)
#elif defined(MADRONA_CLANG) || defined(MADRONA_GCC)
#define MADRONA_ALWAYS_INLINE __attribute__((always_inline))
#define MADRONA_NO_INLINE __attribute__((noinline))
#endif
