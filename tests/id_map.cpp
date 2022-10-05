#include <gtest/gtest.h>

#include <madrona/crash.hpp>
#include <madrona/heap_array.hpp>
#include <madrona/dyn_array.hpp>
#include <madrona/impl/id_map_impl.inl>

#include <thread>

using namespace madrona;

struct TestID {
    uint32_t id;
    uint32_t gen;
};

struct TestTracker {
    uint32_t tracker;
    uint32_t other;
};

template <typename T, bool expandable>
struct TestStore {
    TestStore(uint32_t size)
        : data_(size)
    {}

    inline T & operator[](uint32_t idx) { return data_[idx]; }
    inline const T & operator[](uint32_t idx) const { return data_[idx]; }

    uint32_t expand(uint32_t num_new_elems)
    {
        if constexpr (expandable) {
            uint32_t offset = data_.size();
            data_.resize(offset + num_new_elems);

            return offset;
        } else {
            FATAL("Store out of IDs");
        }
    }

    HeapArray<T> data_;
};

template <typename T> using ExpandableTestStore = TestStore<T, true>;
template <typename T> using UnexpandableTestStore = TestStore<T, false>;

TEST(IDs, Threads)
{
    using TestMap = IDMap<TestID, TestTracker, UnexpandableTestStore>;

    TestMap test_map(16384);

    const int num_threads = 6;
    const int num_iters = 10000;
    DynArray<std::thread> threads(num_threads);

    for (int thread_idx = 0; thread_idx < num_threads; thread_idx++) {
        threads.emplace_back([&test_map]() {
            TestMap::Cache cache;
            const int num_ids = 1000;
            DynArray<TestID> thread_ids(num_ids);
            for (int i = 0; i < num_iters; i++) {
                for (int j = 0; j < num_ids; j++) {
                    TestID id = test_map.acquireID(cache);
                    thread_ids.push_back(id);
                }
                for (TestID id : thread_ids) {
                    EXPECT_TRUE(test_map.present(id));
                    test_map.releaseID(cache, id);
                }

                thread_ids.clear();
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }
}
