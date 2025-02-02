#pragma once

#include <madrona/table.hpp>
#include <madrona/optional.hpp>

namespace madrona {

struct QueryRef {
    std::atomic_uint32_t numReferences;
    uint32_t offset;
    uint32_t numMatchingArchetypes;
};

template <typename... ComponentTs>
class Query {
public:
    Query();
    Query(const Query &) = delete;
    Query(Query &&o);

    ~Query();

    Query & operator=(const Query &) = delete;
    Query & operator=(Query &&o);

    inline uint32_t numMatchingArchetypes() const;

private:
    Query(bool initialized);
    bool initialized_;

    static QueryRef ref_;

friend class StateManager;
};

template <typename T>
class ResultRef {
public:
    inline ResultRef(T *ptr);

    inline T & value();
    inline bool valid() const;

private:
    T *ptr_;
};

template <typename ComponentT>
class ComponentRef {
public:
    ComponentRef(Table *tbl, uint32_t col_idx);

    inline ComponentT & operator[](uint32_t row) const;

    inline ComponentT * data() const;
    inline uint32_t size() const;

    inline ComponentT * begin() const;
    inline ComponentT * end() const;

private:
    Table *tbl_;
    uint32_t col_idx_;
};

template <typename ArchetypeT>
class ArchetypeRef {
public:
    inline ArchetypeRef(Table *tbl);

    template <typename ComponentT>
    inline ComponentRef<ComponentT> component();

    template <typename ComponentT>
    inline ComponentRef<const ComponentT> component() const;

    template <typename ComponentT>
    inline ComponentT & get(uint32_t idx);
    template <typename ComponentT>
    inline const ComponentT & get(uint32_t idx) const;

    inline uint32_t size() const;

private:
    template <typename ComponentT> struct ComponentLookup {};

    template <typename ComponentT>
    inline uint32_t getComponentIndex() const;

    Table *tbl_;

friend class StateManager;
};

}

#include "query.inl"
