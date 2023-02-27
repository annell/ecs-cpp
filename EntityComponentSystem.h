//
// Created by Stefan Annell on 2021-05-12.
//

#pragma once

#include <array>

namespace ecs {
    /**
     * EntityID
     * Id reference to a entity.
     */
    class EntityID {
    public:
        using ID = size_t;

        explicit EntityID(ID id)
                : id(id) {
        }

        [[nodiscard]] EntityID::ID GetId() const {
            return id;
        }

        friend bool operator==(const EntityID &a, const EntityID &b) { return a.id == b.id; };
    private:
        ID id;
    };

    template<typename TypeToCheck, typename... TypesToCheckAgainst>
    concept TypeIn = (std::same_as<std::remove_cvref_t<TypeToCheck>, TypesToCheckAgainst> || ...);

    template<typename ... TComponent>
    concept IsBasicType = ((
            std::default_initializable<TComponent> &&
            not std::is_pointer_v<TComponent> &&
            not std::is_reference_v<TComponent> &&
            not std::is_const_v<TComponent> &&
            not std::is_volatile_v<TComponent>) &&
            ...);

    /**
    * ECSManager
    * A ECS container that keeps track of all components
    * and entities in a scene
    *
    * Initialized with a list of components to track:
    * ECSManager<TComponent1, TComponent2>()
    *
    * Has place for up to NumberOfSlots = 1024 entities.
    *
    * Add, Remove, Has got both entity and component
    * interfaces to be able to interact with both easily
    * depending on the need.
    *
    * GetSystem<Component1, ...>() is the main way for
    * systems to integrate against the ECS container as
    * it filters out the entities that contains the
    * requested components and allows for easy iteration.
     * @tparam TComponents list of components that ECS tracks.
     * TComponents needs to fufill the IsBasicType concept.
     */
    template<IsBasicType ... TComponents>
    class ECSManager {
    public:
        using TECSManager = ECSManager<TComponents...>;
        constexpr static int NumberOfSlots = 1024;

        template<typename TComponent>
        using ComponentArray = std::array<TComponent, NumberOfSlots>;
        using ComponentMatrix = std::tuple<ComponentArray<TComponents>...>;

        template<typename /*TComponent*/>
        struct AvailableComponent {
            bool active = false;
        };
        using AvailableComponents = std::tuple<AvailableComponent<TComponents>...>;

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
        using EntitiesSlots = std::array<Entity, NumberOfSlots>;

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
            [[maybe_unused]] SystemIterator(TECSManager &ecs, TInternalIterator it) : ecs(ecs), it(it) {}

            auto operator*() const { return ecs.template GetSeveral<TSystemComponents ...>(it->id); }

            SystemIterator &operator++() {
                it++;
                auto endIt = ecs.end();
                while (it != endIt && (!it->active || !ecs.template Has<TSystemComponents ...>(it->id))) {
                    it++;
                }
                return *this;
            }

            friend bool operator==(const SystemIterator &a, const SystemIterator &b) { return a.it == b.it; };

            friend bool operator!=(const SystemIterator &a, const SystemIterator &b) { return a.it != b.it; };

        private:
            TECSManager &ecs;
            TInternalIterator it;
        };

        /**
         * System
         * A class that takes a ECS as input and creates
         * SystemIterators that a user can use to loop over the
         * components.
         * @tparam TSystemComponents components to filter on.
         */
        template<typename... TSystemComponents>
        struct System {
        private:
            using TSystemIterator = SystemIterator<TSystemComponents...>;
        public:
            explicit System(TECSManager &ecs) : ecs(ecs) {}

            /**
             * Returns a iterator to the first value in the system.
             * Will match the components and skip over if entity
             * does not have the correct components.
             * @return TSystemIterator with a references to component data.
             */
            [[nodiscard]] TSystemIterator begin() const {
                auto endIt = ecs.end();
                auto it = ecs.begin();
                while (it != endIt && (!it->active || !ecs.template Has<TSystemComponents ...>(it->id))) {
                    it++;
                }
                return TSystemIterator(ecs, it);
            }


            /**
             * Returns a iterator to end value in the system.
             * @return TSystemIterator to end iterator.
             */
            [[nodiscard]] TSystemIterator end() const { return TSystemIterator(ecs, ecs.end()); }

        private:
            TECSManager &ecs;
        };

        ECSManager();

        /**
         * Add a new entity to the ECS
         * @return EntityID
         */
        [[nodiscard]] inline EntityID Add();

        /**
         * Adds a new component to a entity.
         * @tparam TComponent type of the new component.
         * @param entityId reference to the entity.
         * @param component the data of the component.
         */
        template<TypeIn<TComponents...> TComponent>
        void Add(const EntityID &entityId, const TComponent &component);

        /**
         * Remove a entity from the ECS.
         * @param entityId reference to the entity.
         */
        inline void Remove(const EntityID &entityId);

        /**
         * Remove a component from a entity.
         * @tparam TComponent type of the component to remove
         * @param entityId reference to the entity.
         */
        template<TypeIn<TComponents...> TComponent>
        void Remove(const EntityID &entityId);

        /**
         * Checks if ecs has the given entity
         * @param entityId reference to the entity
         * @return bool if entity is active.
         */
        [[nodiscard]] inline bool Has(const EntityID &entityId) const;

        /**
         * Checks if the given entity has the components.
         * @tparam TEntityComponents The type of the components.
         * @param entityId reference to the entity
         * @return bool if component is active.
         */
        template<typename... TEntityComponents>
        [[nodiscard]] bool Has(const EntityID &entityId) const;

        /**
         * Returns a reference to the requested component data.
         * @tparam TComponent the type of the component
         * @param entityId reference to the entity.
         * @return TComponent& reference to the component.
         */
        template<TypeIn<TComponents...> TComponent>
        [[nodiscard]] TComponent &Get(const EntityID &entityId);

        /**
         * Returns a system which is a list of a set of components.
         * @tparam TSystemComponents the list of components in the system.
         * @return System<TSystemComponents...> the system of components.
         */
        template<typename ... TSystemComponents>
        [[nodiscard]] System<TSystemComponents...> GetSystem();

        /**
         * Returns number of entities in ECS
         * @return size_t
         */
        [[nodiscard]] size_t Size() const;

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
        template<typename... TComponentsRequested>
        [[nodiscard]] auto GetSeveral(const EntityID &entityId) {
            return std::forward_as_tuple(Get<TComponentsRequested>(entityId)...);
        }

        template<TypeIn<TComponents...> TEntityComponent>
        [[nodiscard]] bool HasInternal(const EntityID &entityId) const {
            return ReadComponent<TEntityComponent>(entityId).active;
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
            return std::get<AvailableComponent<TEntityComponent>>(ReadEntity(entityId.GetId()).activeComponents);
        }

        [[nodiscard]] inline const Entity &ReadEntity(size_t index) const {
            ValidateID(index);
            return entities[index];
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
            if (endSlot >= NumberOfSlots) {
                throw std::out_of_range("Used up all NumberOfSlots!");
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
    };

    template<IsBasicType... TComponents>
    ECSManager<TComponents...>::ECSManager() {
        int id = 0;
        for (auto &entity: entities) {
            entity.id = EntityID(id++);
        }
    }

    template<IsBasicType... TComponents>
    EntityID ECSManager<TComponents...>::Add() {
        auto slot = GetFirstEmptySlot();
        auto &entity = GetEntity(slot);
        entity.active = true;
        std::apply([](auto &&...args) { ((args.active = false), ...); }, entity.activeComponents);
        nrEntities++;
        return entity.id;
    }

    template<IsBasicType... TComponents>
    template<TypeIn<TComponents...> TComponent>
    void ECSManager<TComponents...>::Add(const EntityID &entityId, const TComponent &component) {
        auto &isActive = GetComponent<TComponent>(entityId).active;
        if (isActive) {
            throw std::logic_error("Component already added!");
        }
        isActive = true;
        GetComponentData<TComponent>(entityId) = component;
    }

    template<IsBasicType... TComponents>
    void ECSManager<TComponents...>::Remove(const EntityID &entityId) {
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

    template<IsBasicType... TComponents>
    template<TypeIn<TComponents...> TComponent>
    void ECSManager<TComponents...>::Remove(const EntityID &entityId) {
        auto &isActive = GetComponent<TComponent>(entityId).active;
        if (!isActive) {
            throw std::logic_error("Component not active!");
        }
        isActive = false;
    }

    template<IsBasicType... TComponents>
    bool ECSManager<TComponents...>::Has(const EntityID &entityId) const {
        if (entityId.GetId() >= NumberOfSlots) {
            throw std::out_of_range("Trying to access out of bounds!");
        }
        return entities[entityId.GetId()].active;
    }

    template<IsBasicType... TComponents>
    template<typename... TEntityComponents>
    bool ECSManager<TComponents...>::Has(const EntityID &entityId) const {
        return (HasInternal<TEntityComponents>(entityId) && ...);
    }

    template<IsBasicType... TComponents>
    template<TypeIn<TComponents...> TComponent>
    TComponent &ECSManager<TComponents...>::Get(const EntityID &entityId) {
        if (!ReadComponent<TComponent>(entityId).active) {
            throw std::invalid_argument("Bad access, component not present on this entity.");
        }
        return GetComponentData<TComponent>(entityId);
    }

    template<IsBasicType... TComponents>
    template<typename ... TSystemComponents>
    typename ECSManager<TComponents...>::template System<TSystemComponents...> ECSManager<TComponents...>::GetSystem() {
        return System<TSystemComponents...>(*this);
    }

    template<IsBasicType... TComponents>
    size_t ECSManager<TComponents...>::Size() const {
        return nrEntities;
    }

    template<IsBasicType... TComponents>
    [[nodiscard]] typename ECSManager<TComponents...>::EntitiesSlots::const_iterator
    ECSManager<TComponents...>::begin() const {
        return entities.begin();
    }

    template<IsBasicType... TComponents>
    [[nodiscard]] typename ECSManager<TComponents...>::EntitiesSlots::const_iterator
    ECSManager<TComponents...>::end() const {
        return entities.begin() + endSlot;
    }
}// namespace ecs
