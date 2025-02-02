#pragma once

#include <cstdint>

namespace madrona {

class TypeTracker {
public:
    template <typename T>
    static inline uint32_t typeID();

    template <typename T>
    static void registerType(uint32_t *next_id_ptr);

    static constexpr uint32_t unassignedTypeID = ~0u;
private:
    template <typename T>
    struct TypeID {
        static uint32_t id;
    };

    template <typename T>
    static uint32_t trackType(uint32_t *ptr);
    static uint32_t trackByName(uint32_t *ptr, const char *compiler_name);

    static void registerType(uint32_t *id_ptr, uint32_t *next_id_ptr);
};

}

#include "type_tracker.inl"
