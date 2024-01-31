//
// Created by Stefan Annell on 2021-05-12.
//

#pragma once

#include <vector>
#include <array>
#include <tuple>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include "EntityID.h"
#include "EcsUtil.h"

namespace ecs {
    /**
    * ECSManager
    * A ECS container that keeps track of all components
    * and entities in a scene
    *
    * Initialized with a list of components to track:
    * ECSManager<TComponent1, TComponent2>()
    *
    * AddEntity, Remove, Has got both entity and component
    * interfaces to be able to interact with both easily
    * depending on the need.
    *
    * GetSystem<Component1, ...>() is the main way for
    * systems to integrate against the ECS container as
    * it filters out the entities that contains the
    * requested components and allows for easy iteration.
     * @tparam TComponents list of components that ECS tracks.
     * TComponents needs to fufill the IsBasicType and NonVoidArgs
     * concepts.
     */
    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    class ECSManager {
    private:
        using TComponentPack = std::tuple<TComponents...>;
        using TECSManager = ECSManager<TComponents...>;

        template<typename TComponent>
        using ComponentArray = std::vector<TComponent>;
        using ComponentMatrix = std::tuple<ComponentArray<TComponents>...>;

        template<typename /*TComponent*/>
        struct AvailableComponent {
            bool active = false;
        };
        using AvailableComponents = std::tuple<AvailableComponent<TComponents>...>;

        template<typename TComponent>
        struct ComponentRange {
            using TComponentRange = TComponent;
            bool componentPresent = false;
            size_t firstSlot = SIZE_MAX;
            size_t lastSlot = 0;
        };
        using ComponentRanges = std::tuple<ComponentRange<TComponents>...>;

        struct ComponentRangesMatch {
            size_t firstSlot = SIZE_MAX;
            size_t lastSlot = 0;
        };

        /**
         * Entity
         * The entity in the ECS, tracks its own ID if it is
         * active and if it has any components added to it.
         */
        struct Entity {
            AvailableComponents activeComponents{};
            bool active = false;
            EntityID id = EntityID(0);
        };
        using EntitiesSlots = std::vector<Entity>;

        /**
         * SystemIterator
         * A iterator that loops over the matching components,
         * skipping the ones that does not have the correct
         * components active.
         * @tparam TSystemComponents list of components that
         * iterator tracks.
         */
        template<typename... TSystemComponents>
        struct SystemIterator {
        private:
            using TInternalIterator = typename TECSManager::EntitiesSlots::const_iterator;
        public:
            [[maybe_unused]] SystemIterator(TECSManager &ecs, TInternalIterator begin, TInternalIterator end) : ecs(ecs), begin(begin), end(end) {}

            auto operator*() const { return ecs.template GetSeveral<TSystemComponents ...>(begin->id); }

            SystemIterator &operator++() {
                begin++;
                while (begin != end) {
                    if (ecs.HasGivenComponents<TSystemComponents ...>(begin)) {
                        break;
                    }
                    begin++;
                }
                return *this;
            }

            friend bool operator==(const SystemIterator &a, const SystemIterator &b) { return a.begin == b.begin; };

            friend bool operator!=(const SystemIterator &a, const SystemIterator &b) { return a.begin != b.begin; };

        private:
            TECSManager &ecs;
            TInternalIterator begin;
            const TInternalIterator end;
        };

        /**
         * System
         * A class that takes a ECS as input and creates
         * SystemIterators that a user can use to loop over the
         * components.
         * The lifetime of a System needs to be shorter then
         * its underlying ecs as it stores a reference to it.
         * @tparam TSystemComponents components to filter on.
         */
        template<typename... TSystemComponents>
        struct System {
        private:
            using TSystemIterator = SystemIterator<TSystemComponents...>;
        public:
            constexpr System(TECSManager &ecs, size_t part, size_t totalParts) : ecs(ecs), part(part), totalParts(totalParts), componentRangesMatch(ecs.GetSystemFilterMatch<TSystemComponents...>()) {
                ValidateInvariant();
            }

            /**
             * Returns a iterator to the first value in the system.
             * Will match the components and skip over if entity
             * does not have the correct components.
             * @return TSystemIterator with a references to component data.
             */
            [[nodiscard]] TSystemIterator begin() const {
                if (!componentRangesMatch) {
                    return end();
                }
                auto end = ecsEnd();
                auto begin = ecsBegin();
                while (begin != end) {
                    if (ecs.HasGivenComponents<TSystemComponents ...>(begin)) {
                        break;
                    }
                    begin++;
                }
                return TSystemIterator(ecs, begin, end);
            }


            /**
             * Returns a iterator to end value in the system.
             * @return TSystemIterator to end iterator.
             */
            [[nodiscard]] TSystemIterator end() const { return TSystemIterator(ecs, ecsEnd(), ecsEnd()); }

        private:
            auto ecsEnd() const {
                return ecs.end() - endIteratorOffset() - (componentRangesMatch ? ecs.ContainerSize() - 1 - componentRangesMatch->lastSlot : 0);
            }

            auto ecsBegin() const {
                return ecs.begin() + beginIteratorOffset() + (componentRangesMatch ? componentRangesMatch->firstSlot : 0);
            }

            constexpr size_t partSize() const {
                return ecs.ContainerSize() / totalParts;
            }

            constexpr bool hasRemainder() const {
                return ecs.ContainerSize() % totalParts != 0;
            }

            size_t endIteratorOffset() const {
                if (hasRemainder()) {
                    if (part == totalParts - 1) {
                        return 0;
                    }
                }
                return ecs.ContainerSize() - (part + 1) * partSize();
            }

            size_t beginIteratorOffset() const {
                return part * partSize();
            }

            void ValidateInvariant() const {
                if (componentRangesMatch && componentRangesMatch->firstSlot > componentRangesMatch->lastSlot) {
                    throw std::logic_error("Invariant broken! FirstSlot > LastSlot");
                }
            }

            TECSManager &ecs;
            size_t part = 0;
            size_t totalParts = 1;
            std::optional<ComponentRangesMatch> componentRangesMatch{};
        };

    public:
        constexpr ECSManager() = default;

        /**
         * AddEntity a new entity to the ECS
         * @return EntityID
         */
        [[nodiscard]] constexpr inline EntityID AddEntity();

        /**
         * BuildEntity adds a entity and adds in components to the
         * new entity. So both AddEntity and Add in one call.
         * @return EntityID
         */
        template<typename... TEntityComponents>
        requires NonVoidArgs<TEntityComponents...>
        constexpr inline EntityID BuildEntity(TEntityComponents&&... args) {
            auto id = AddEntity();
            (Add<TEntityComponents>(id, std::forward<TEntityComponents>(args)), ...);
            return id;
        }

        /**
         * Adds a new component to a entity.
         * @tparam TComponent type of the new component.
         * @param entityId reference to the entity.
         * @param component the data of the component.
         */
        template<typename TComponent>
        requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
        constexpr void Add(const EntityID &entityId, const TComponent &component);

        /**
         * Remove a entity from the ECS.
         * @param entityId reference to the entity.
         */
        constexpr inline void RemoveEntity(const EntityID &entityId);

        /**
         * Remove a component from a entity.
         * @tparam TComponent type of the component to remove
         * @param entityId reference to the entity.
         */
        template<typename TComponent>
        requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
        constexpr void Remove(const EntityID &entityId);

        /**
         * Checks if ecs has the given entity
         * @param entityId reference to the entity
         * @return bool if entity is active.
         */
        [[nodiscard]] constexpr inline bool HasEntity(const EntityID &entityId) const;

        /**
         * Checks if the given entity has the components.
         * @tparam TEntityComponents The type of the components.
         * @param entityId reference to the entity
         * @return bool if component is active.
         */
        template<typename... TEntityComponents>
        requires NonVoidArgs<TEntityComponents...>
        [[nodiscard]] constexpr bool Has(const EntityID &entityId) const;

        /**
         * Checks if the ecs has the given component.
         * @tparam TEntityComponent The type of the component.
         * @return bool if component is present.
         * @note This is a static function and does not need a ecs instance.
         * @note Recomended to use the ecs::HasTypes<>() function instead,
         * to simplify interaction as it works with one or more types and has a
         * simpler syntax.
         */
        template<typename TEntityComponent>
        requires NonVoidArgs<TEntityComponent>
        [[nodiscard]] static constexpr bool HasType() {
            return (TypeInPack<TEntityComponent, TComponents>() || ...);
        }

        /**
         * Returns a reference to the requested component data.
         * @tparam TComponent the type of the component
         * @param entityId reference to the entity.
         * @return TComponent& reference to the component.
         */
        template<typename TComponent>
        requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
        [[nodiscard]] constexpr TComponent &Get(const EntityID &entityId);

        /**
         * A getter to fetch multiple components at once.
         *
         * @tparam TComponentsRequested
         * @param entityId
         * @return A tuple with components in the same order as the template arguments.
         * can use auto [a, b, c] = GetSeveral<A, B, C>(entityId); to unpack.
         */
        template<typename... TComponentsRequested>
        requires NonVoidArgs<TComponentsRequested...> && (TypeIn<TComponentsRequested, TComponents...> && ...)
        [[nodiscard]] auto GetSeveral(const EntityID &entityId) {
            return std::forward_as_tuple(Get<TComponentsRequested>(entityId)...);
        }

        /**
         * Returns a system which is a list of a set of components.
         * @tparam TSystemComponents the list of components in the system.
         * @return System<TSystemComponents...> the system of components.
         */
        template<typename ... TSystemComponents>
        requires NonVoidArgs<TSystemComponents...> && (TypeIn<TSystemComponents, TComponents...> && ...)
        [[nodiscard]] constexpr System<TSystemComponents...> GetSystem();

        /**
         * Returns a part of the system which is a list of a set of components,
         * to be used for splitting the container up for multi threading purposes.
         * @tparam TSystemComponents the list of components in the system.
         * @param part the part of the system to return. 0 indexed, so 0 is the first part.
         * @param totalParts the total number of parts the container is split up into.
         * @return System<TSystemComponents...> the system of components.
         */
        template<typename ... TSystemComponents>
        requires NonVoidArgs<TSystemComponents...> && (TypeIn<TSystemComponents, TComponents...> && ...)
        [[nodiscard]] constexpr System<TSystemComponents...> GetSystemPart(size_t part, size_t totalParts);

        /**
         * Returns number of entities in ECS
         * @return size_t
         */
        [[nodiscard]] constexpr size_t Size() const;

        /**
         * Begin iterator, first element in entities list.
         * @return iterator to begin
         */
        [[nodiscard]] typename EntitiesSlots::const_iterator begin() const;

        /**
         * End iterator, after last element in entities list.
         * @return iterator to end
         */
        [[nodiscard]] typename EntitiesSlots::const_iterator end() const;

    private:
        size_t ContainerSize() const {
            return entities.size();
        }

        template<typename... TSystemComponents>
        bool HasGivenComponents(const auto& it) const {
            return it->active && Has<TSystemComponents ...>(it->id);
        }

        template<typename TEntityComponent>
        void UpdateComponentRange(const EntityID &entityId) {
            if (!HasInternal<TEntityComponent>(entityId)) {
                throw std::logic_error("Not a valid id!");
            }
            auto &componentRange = std::get<ComponentRange<TEntityComponent>>(componentRanges);
            if (!componentRange.componentPresent) {
                componentRange.componentPresent = true;
                componentRange.firstSlot = std::min(entityId.GetId(), componentRange.firstSlot);
                componentRange.lastSlot = std::max(entityId.GetId(), componentRange.lastSlot);
            } else {
                componentRange.lastSlot = entityId.GetId();
            }
        }

        template<typename... TSystemComponents>
        std::optional<ComponentRangesMatch> GetSystemFilterMatch() {
            bool found = false;
            size_t firstSlot = 0;
            size_t lastSlot = SIZE_MAX;

            std::apply([&] <typename... TComponentRange> (TComponentRange&&... args) {
                (((ComponentTypeInPack<TComponentRange, TSystemComponents ...>() && args.componentPresent) ? found = true : 0), ...);
                (((ComponentTypeInPack<TComponentRange, TSystemComponents ...>() && args.componentPresent) ? firstSlot = std::max(args.firstSlot, firstSlot) : 0), ...);
                (((ComponentTypeInPack<TComponentRange, TSystemComponents ...>() && args.componentPresent) ? lastSlot = std::min(args.lastSlot, lastSlot) : 0), ...);
            }, componentRanges);
            if (!found) {
                return std::nullopt;
            }
            return ComponentRangesMatch{firstSlot, lastSlot};
        }

        template<TypeIn<TComponents...> TEntityComponent>
        [[nodiscard]] bool HasInternal(const EntityID &entityId) const {
            return std::get<AvailableComponent<TEntityComponent>>(entities[entityId.GetId()].activeComponents).active;
        }

        template<typename TEntityComponent>
        [[nodiscard]] AvailableComponent<TEntityComponent> &GetComponent(const EntityID &entityId) {
            return std::get<AvailableComponent<TEntityComponent>>(GetEntity(entityId.GetId()).activeComponents);
        }

        [[nodiscard]] inline Entity &GetEntity(size_t index) {
            ValidateID(index);
            return entities[index];
        }

        template<typename TEntityComponent>
        [[nodiscard]] const AvailableComponent<TEntityComponent> &ReadComponent(const EntityID &entityId) const {
            return std::get<AvailableComponent<TEntityComponent>>(entities[entityId.GetId()].activeComponents);
        }

        template<TypeIn<TComponents...> TComponent>
        [[nodiscard]] TComponent &GetComponentData(const EntityID &entityId) {
            ValidateID(entityId.GetId());
            return GetComponentDataArray<TComponent>()[entityId.GetId()];
        }

        template<TypeIn<TComponents...> TComponent>
        [[nodiscard]] ComponentArray<TComponent> &GetComponentDataArray() {
            return std::get<ComponentArray<TComponent>>(componentArrays);
        }

        inline void ValidateID(size_t index) const {
            if (index >= endSlot) {
                throw std::out_of_range("Accessing outside of endSlot!");
            }
        }

        inline void ValidateEntityID(EntityID id) const {
            if (not id) {
                throw std::logic_error("ID not initialized!");
            }
        }

        size_t GetFirstEmptySlot() {
            size_t slot = 0;
            while (slot < endSlot && entities[slot].active) {
                slot++;
            }
            if (slot == endSlot) {
                endSlot++;
            }
            return slot;
        }

        [[nodiscard]] size_t GetLastSlot() const {
            if (endSlot == 0) {
                return 0;
            }
            return endSlot - 1;
        }

        size_t endSlot = 0;
        size_t nrEntities = 0;
        EntitiesSlots entities;
        ComponentMatrix componentArrays{};
        ComponentRanges componentRanges{};
    };

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    constexpr EntityID ECSManager<TComponents...>::AddEntity() {
        auto slot = GetFirstEmptySlot();
        if (slot == entities.size()) {
            entities.push_back({.id=EntityID(slot)});
            std::apply([](auto &&...args) { ((PushToVector(args)), ...); }, componentArrays);
        }
        auto &entity = GetEntity(slot);
        entity.active = true;
        std::apply([](auto &&...args) { ((args.active = false), ...); }, entity.activeComponents);
        nrEntities++;
        if constexpr ((std::is_same<EntityID, TComponents>::value || ...)) {
            Add<EntityID>(entity.id, entity.id);
        }
        return entity.id;
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename TComponent>
    requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
    constexpr void ECSManager<TComponents...>::Add(const EntityID &entityId, const TComponent &component) {
        ValidateEntityID(entityId);
        auto &isActive = GetComponent<TComponent>(entityId).active;
        if (isActive) {
            throw std::logic_error("Component already added!");
        }
        isActive = true;
        GetComponentData<TComponent>(entityId) = component;
        UpdateComponentRange<TComponent>(entityId);
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    constexpr void ECSManager<TComponents...>::RemoveEntity(const EntityID &entityId) {
        ValidateEntityID(entityId);
        auto &entity = GetEntity(entityId.GetId());
        if (!entity.active) {
            throw std::logic_error("Entity not active!");
        }
        entity.active = false;
        if (GetLastSlot() == entity.id.GetId()) {
            endSlot--;
        }
        nrEntities--;
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename TComponent>
    requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
    constexpr void ECSManager<TComponents...>::Remove(const EntityID &entityId) {
        ValidateEntityID(entityId);
        auto &isActive = GetComponent<TComponent>(entityId).active;
        if (!isActive) {
            throw std::logic_error("Component not active!");
        }
        isActive = false;
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    constexpr bool ECSManager<TComponents...>::HasEntity(const EntityID &entityId) const {
        ValidateEntityID(entityId);
        if (entityId.GetId() >= entities.size()) {
            throw std::out_of_range("Trying to access out of bounds!");
        }
        return entities[entityId.GetId()].active;
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename... TEntityComponents>
    requires NonVoidArgs<TEntityComponents...>
    constexpr bool ECSManager<TComponents...>::Has(const EntityID &entityId) const {
        ValidateEntityID(entityId);
        return (HasInternal<TEntityComponents>(entityId) && ...);
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename TComponent>
    requires NonVoidArgs<TComponents...> && TypeIn<TComponent, TComponents...>
    constexpr TComponent &ECSManager<TComponents...>::Get(const EntityID &entityId) {
        ValidateEntityID(entityId);
        if (!ReadComponent<TComponent>(entityId).active) {
            throw std::invalid_argument("Bad access, component not present on this entity.");
        }
        return GetComponentData<TComponent>(entityId);
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename ... TSystemComponents>
    requires NonVoidArgs<TSystemComponents...> && (TypeIn<TSystemComponents, TComponents...> && ...)
    constexpr typename ECSManager<TComponents...>::template System<TSystemComponents...> ECSManager<TComponents...>::GetSystem() {
        return GetSystemPart<TSystemComponents...>(0, 1);
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    template<typename ... TSystemComponents>
    requires NonVoidArgs<TSystemComponents...> && (TypeIn<TSystemComponents, TComponents...> && ...)
    constexpr typename ECSManager<TComponents...>::template System<TSystemComponents...> ECSManager<TComponents...>::GetSystemPart(size_t part, size_t totalParts) {
        return System<TSystemComponents...>(*this, part, totalParts);
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    constexpr size_t ECSManager<TComponents...>::Size() const {
        return nrEntities;
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    [[nodiscard]] typename ECSManager<TComponents...>::EntitiesSlots::const_iterator
    ECSManager<TComponents...>::begin() const {
        return entities.begin();
    }

    template<typename... TComponents>
    requires NonVoidArgs<TComponents...> && IsBasicType<TComponents...>
    [[nodiscard]] typename ECSManager<TComponents...>::EntitiesSlots::const_iterator
    ECSManager<TComponents...>::end() const {
        return entities.begin() + endSlot;
    }

    template <typename TECSManager, typename... TEntityComponents>
    constexpr bool HasTypes() {
        return (TECSManager::template HasType<TEntityComponents>() && ...);
    }
}// namespace ecs
