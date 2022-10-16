/*
 * Copyright 2021-2022 Brennan Shacklett and contributors
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */
#pragma once

#include <madrona/job.hpp>
#include <madrona/state.hpp>

#include "mw_gpu/worker_init.hpp"

namespace madrona {

class Context {
public:
    inline Context(WorldBase *world_data, WorkerInit &&init);

    inline StateManager & state();

    template <typename Fn, typename... DepTs>
    inline JobID submit(Fn &&fn, bool is_child = true,
                        DepTs && ...dependencies);

    template <typename Fn, typename... DepTs>
    inline JobID submitN(Fn &&fn, uint32_t num_invocations,
        bool is_child = true, DepTs && ...dependencies);

    template <typename... ColTypes, typename Fn, typename... DepTs>
    inline JobID parallelFor(const Query<ColTypes...> &query, Fn &&fn,
        bool is_child = true, DepTs && ... dependencies);

    void markJobFinished(uint32_t num_jobs);

    inline JobID currentJobID() const { return job_id_; }

    inline uint32_t worldID() const { return world_id_; }

    inline WorldBase & data() const { return *data_; }

protected:
    template <typename ContextT, typename Fn, typename... DepTs>
    inline JobID submitImpl(Fn &&fn, bool is_child,
                            DepTs && ... dependencies);

    template <typename ContextT, typename Fn, typename... DepTs>
    inline JobID submitNImpl(Fn &&fn, uint32_t num_invocations, bool is_child,
                             DepTs && ... dependencies);

    template <typename ContextT, typename... ComponentTs,
              typename Fn, typename... DepTs>
    inline JobID parallelForImpl(const Query<ComponentTs...> &query, Fn &&fn,
                                 bool is_child, DepTs && ... dependencies);

    WorldBase *data_;

private:
    struct WaveInfo {
        uint32_t activeMask;
        uint32_t numActive;
        uint32_t leaderLane;
        uint32_t coalescedIDX;
    };

    WaveInfo computeWaveInfo();

    JobID getNewJobID(bool link_parent, uint32_t num_invocations);

    JobContainerBase * allocJob(uint32_t bytes_per_job, WaveInfo wave_info);

    void addToWaitList(uint32_t func_id, JobContainerBase *data,
                       uint32_t num_invocations,
                       uint32_t num_bytes_per_job,
                       uint32_t lane_id, WaveInfo wave_info);

    JobID job_id_;
    uint32_t grid_id_;
    uint32_t world_id_;
    uint32_t lane_id_;
};

}

#include "context.inl"
