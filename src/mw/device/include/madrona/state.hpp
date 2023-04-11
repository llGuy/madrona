/*
 * Copyright 2021-2022 Brennan Shacklett and contributors
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */
#pragma once

#include <atomic>
#include <array>

#include <madrona/ecs.hpp>
#include <madrona/hashmap.hpp>
#include <madrona/inline_array.hpp>
#include <madrona/sync.hpp>
#include <madrona/query.hpp>
#include <madrona/optional.hpp>
#include <madrona/type_tracker.hpp>
#include <madrona/memory.hpp>

#include "mw_gpu/const.hpp"

namespace madrona {

class StateManager;

namespace mwGPU {
static inline StateManager *getStateManager()
{
    return (StateManager *)GPUImplConsts::get().stateManagerAddr;
}
}

struct ComponentID {
    uint32_t id;

private:
    ComponentID(uint32_t i) : id(i) {};
friend class StateManager;
};

struct ArchetypeID {
    uint32_t id;

private:
    ArchetypeID(uint32_t i) : id(i) {};
friend class StateManager;
};

class ECSRegistry {
public:
    ECSRegistry(StateManager &state_mgr, void **export_ptr);

    template <typename ComponentT>
    void registerComponent();

    template <typename ArchetypeT>
    void registerArchetype();

    template <typename ArchetypeT>
    void registerFixedSizeArchetype(CountT max_num_entities);

    template <typename SingletonT>
    void registerSingleton();

    template <typename ArchetypeT, typename ComponentT>
    void exportColumn(int32_t slot);

    template <typename SingletonT>
    void exportSingleton(int32_t slot);

private:
    StateManager *state_mgr_;
    void **export_ptr_;
};

struct EntityStore {
    struct EntitySlot {
        Loc loc;
        uint32_t gen;
    };

    AtomicI32 availableOffset = 0;
    AtomicI32 deletedOffset = 0;

    EntitySlot *entities;
    int32_t *availableEntities;
    int32_t *deletedEntities;

    int32_t numMappedEntities;
    uint32_t numGrowEntities;
    uint32_t numSlotGrowBytes;
    uint32_t numIdxGrowBytes;

    SpinLock growLock {};
};

class StateManager {
public:
    StateManager(uint32_t max_components);

    template <typename ComponentT>
    ComponentID registerComponent();

    template <typename ArchetypeT>
    ArchetypeID registerArchetype();

    template <typename SingletonT>
    void registerSingleton();

    template <typename SingletonT>
    SingletonT & getSingleton(WorldID world_id);

    template <typename... ComponentTs>
    Query<ComponentTs...> query();

    template <int32_t num_components, typename Fn>
    void iterateArchetypesRaw(QueryRef *query_ref, Fn &&fn);

    inline uint32_t numMatchingEntities(QueryRef *query_ref);

    template <typename ArchetypeT>
    Entity makeEntityNow(WorldID world_id);

    void destroyEntityNow(Entity e);

    template <typename ArchetypeT>
    Loc makeTemporary(WorldID world_id);

    template <typename ArchetypeT>
    void clearTemporaries();

    void clearTemporaries(uint32_t archetype_id);

    inline Loc getLoc(Entity e) const;

    template <typename ComponentT>
    ComponentT & getUnsafe(Entity e);

    template <typename ComponentT>
    ComponentT & getUnsafe(Loc loc);

    template <typename ComponentT>
    inline ResultRef<ComponentT> get(Entity e);

    template <typename ComponentT>
    inline ResultRef<ComponentT> get(Loc loc);

    template <typename ComponentT>
    inline ComponentT & getDirect(int32_t column_idx, Loc loc);

    template <typename ArchetypeT, typename ComponentT>
    ComponentT * getArchetypeComponent();

    inline void * getArchetypeComponent(uint32_t archetype_id,
                                        uint32_t component_id);

    inline int32_t getArchetypeColumnIndex(uint32_t archetype_id,
                                           uint32_t component_id);

    inline void * getArchetypeColumn(uint32_t archetype_id,
                                     int32_t column_idx);

    inline uint32_t getArchetypeColumnBytesPerRow(uint32_t archetype_id,
                                                  int32_t column_idx);

    inline int32_t getArchetypeNumColumns(uint32_t archetype_id);
    inline uint32_t getArchetypeMaxColumnSize(uint32_t archetype_id);

    inline void remapEntity(Entity e, int32_t row_idx);

    template <typename SingletonT>
    SingletonT * getSingletonColumn();

    void resizeArchetype(uint32_t archetype_id, int32_t num_rows);
    int32_t numArchetypeRows(uint32_t archetype_id) const;

    std::pair<int32_t, int32_t> fetchRecyclableEntities();

    void recycleEntities(int32_t thread_offset,
                         int32_t recycle_base);

    inline CountT archetypeIncrementSortCounter(uint32_t archetype_id);
    inline void archetypeResetSortCounter(uint32_t archetype_id);

    inline bool archetypeNeedsSort(uint32_t archetype_id) const;
    inline void archetypeClearNeedsSort(uint32_t archetype_id);

private:
    template <typename SingletonT>
    struct SingletonArchetype : public madrona::Archetype<SingletonT> {};

    using ColumnMap = StaticIntegerMap<Table::maxColumns>;
    static constexpr uint32_t max_archetype_components_ = ColumnMap::numFree();

    static inline uint32_t num_components_ = 0;
    static inline uint32_t num_archetypes_ = 0;

    static constexpr uint32_t max_components_ = 512;
    static constexpr uint32_t max_archetypes_ = 128;
    static constexpr uint32_t user_component_offset_ = 2;
    static constexpr uint32_t max_query_slots_ = 65536;
    static inline constexpr int32_t num_elems_per_sort_thread_ = 2;


    template <typename> struct RegistrationHelper;

    void registerComponent(uint32_t id, uint32_t alignment,
                           uint32_t num_bytes);
    void registerArchetype(uint32_t id, ComponentID *components,
                           uint32_t num_components);

    template <typename Fn, int32_t... Indices>
    void iterateArchetypesRawImpl(QueryRef *query_ref, Fn &&fn,
                                  std::integer_sequence<int32_t, Indices...>);

    void makeQuery(const uint32_t *components,
                   uint32_t num_components,
                   QueryRef *query_ref);

    Entity makeEntityNow(WorldID world_id, uint32_t archetype_id);
    Loc makeTemporary(WorldID world_id, uint32_t archetype_id);

    struct ArchetypeStore {
        ArchetypeStore(uint32_t offset, uint32_t num_user_components,
                       uint32_t num_columns,
                       TypeInfo *type_infos, IntegerMapPair *lookup_input);
        uint32_t componentOffset;
        uint32_t numUserComponents;
        Table tbl;
        ColumnMap columnLookup;
        bool needsSort;

        // Experimenting with sorting frequency
        CountT sortCounter;
    };

    uint32_t archetype_component_offset_ = 0;
    uint32_t query_data_offset_ = 0;
    SpinLock query_data_lock_ {};
    FixedInlineArray<Optional<TypeInfo>, max_components_> components_ {};
    std::array<uint32_t, max_archetype_components_ * max_archetypes_>
        archetype_components_ {};
    FixedInlineArray<Optional<ArchetypeStore>, max_archetypes_> archetypes_ {};
    std::array<uint32_t, max_query_slots_> query_data_ {};
    EntityStore entity_store_;
};

}

#include "state.inl"
