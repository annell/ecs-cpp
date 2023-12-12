//
// Created by Stefan Annell on 2023-12-12.
//
#pragma once

#include <cstddef>
#include <stdint.h>

namespace ecs {
    /**
     * EntityID
     * Id reference to a entity.
     */
    class EntityID {
    public:
        using ID = size_t;

        constexpr EntityID() = default;

        constexpr EntityID(ID id)
                : id(id) {
        }

        [[nodiscard]] constexpr const EntityID::ID &GetId() const {
            return id;
        }

        friend constexpr bool operator==(const EntityID &a, const EntityID &b) { return a.id == b.id; };

        constexpr operator bool() const { return id != SIZE_MAX; }

    private:
        ID id = SIZE_MAX;
    };

}