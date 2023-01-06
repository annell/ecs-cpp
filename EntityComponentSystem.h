//
// Created by Stefan Annell on 2021-05-12.
//

#pragma once
#include <array>

namespace ecs {
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
    concept type_in = (std::same_as<std::remove_cvref_t<TypeToCheck>, TypesToCheckAgainst> || ...);

    template<typename... TComponents>
    class ECSManager {
    public:
        constexpr static int slots = 1024;

        template<typename TComponent>
        using ComponentArray = std::array<TComponent, slots>;
        using ComponentArrays = std::tuple<ComponentArray<TComponents>...>;

        template<typename TComponent>
        struct ComponentActive {
            bool active = false;
        };
        using ComponentsActive = std::tuple<ComponentActive<TComponents>...>;

        struct Entity {
            ComponentsActive componentsActive{};
            bool isEntityActive = false;
            EntityID id = EntityID(0);
        };
        using EntitiesSlots = std::array<Entity, slots>;

        ECSManager() {
            InitializeEntities();
        }

        [[nodiscard]] inline EntityID MakeEntity() {
            auto &entity = GetEntity(endSlot++);
            entity.isEntityActive = true;
            std::apply([](auto &&...args) { ((args.active = false), ...); }, entity.componentsActive);
            return entity.id;
        }

        inline void Remove(const EntityID &entityId) {
            auto &entity = GetEntity(entityId.GetId());
            if (!entity.isEntityActive) {
                throw std::logic_error("Entity not active!");
            }
            entity.isEntityActive = false;
            if (GetLastSlot() == entity.id.GetId()) {
                endSlot--;
            }
        }

        template<typename TComponent>
            requires type_in<TComponent, TComponents...>
        inline void Remove(const EntityID &entityId) {
            auto &isActive = GetComponent<TComponent>(entityId).active;
            if (!isActive) {
                throw std::logic_error("Component not active!");
            }
            isActive = false;
        }

        template<typename TComponent>
            requires type_in<TComponent, TComponents...>
        void Add(const EntityID &entityId, const TComponent &component) {
            auto &isActive = GetComponent<TComponent>(entityId).active;
            if (isActive) {
                throw std::logic_error("Component already added!");
            }
            isActive = true;
            GetComponentData<TComponent>(entityId) = component;
        }

        template<typename... TEntityComponents>
        [[nodiscard]] bool Has(const EntityID &entityId) const {
            return (HasInternal<TEntityComponents>(entityId) && ...);
        }

        [[nodiscard]] inline bool Has(const EntityID &entityId) const {
            if (entityId.GetId() >= slots) {
                throw std::out_of_range("Trying to access out of bounds!");
            }
            return entities[entityId.GetId()].isEntityActive;
        }

        template<typename TComponent>
            requires type_in<TComponent, TComponents...>
        [[nodiscard]] TComponent &Get(const EntityID &entityId) {
            if (!ReadComponent<TComponent>(entityId).active) {
                throw std::invalid_argument("Bad access, component not present on this entity.");
            }
            return GetComponentData<TComponent>(entityId);
        }

        template<typename... TComponentsRequested>
        [[nodiscard]] auto GetSeveral(const EntityID &entityId) {
            return std::forward_as_tuple(Get<TComponentsRequested>(entityId)...);
        }

        [[nodiscard]] size_t Size() const {
            return endSlot;
        }

        [[nodiscard]] typename EntitiesSlots::const_iterator begin() const {
            return entities.begin();
        }

        [[nodiscard]] typename EntitiesSlots::const_iterator end() const {
            return entities.begin() + endSlot;
        }

        template<typename TComponent>
        [[nodiscard]] bool IsComponentActive(const ComponentsActive &components) const {
            return std::get<ComponentActive<TComponent>>(components).active;
        }

    private:

        inline void InitializeEntities() {
            int id = 0;
            for (auto &entity : entities) {
                entity.id = EntityID(id++);
            }
        }

        template<typename TEntityComponent>
            requires type_in<TEntityComponent, TComponents...>
        [[nodiscard]] bool HasInternal(const EntityID &entityId) const {
            return ReadComponent<TEntityComponent>(entityId).active;
        }

        template<typename TEntityComponent>
        [[nodiscard]] ComponentActive<TEntityComponent> &GetComponent(const EntityID &entityId) {
            return std::get<ComponentActive<TEntityComponent>>(GetEntity(entityId.GetId()).componentsActive);
        }

        [[nodiscard]] inline Entity &GetEntity(size_t index) {
            ValidateID(index);
            return entities[index];
        }

        template<typename TEntityComponent>
        [[nodiscard]] const ComponentActive<TEntityComponent> &ReadComponent(const EntityID &entityId) const {
            return std::get<ComponentActive<TEntityComponent>>(ReadEntity(entityId.GetId()).componentsActive);
        }

        [[nodiscard]] inline const Entity &ReadEntity(size_t index) const {
            ValidateID(index);
            return entities[index];
        }

        template<typename TComponent>
            requires type_in<TComponent, TComponents...>
        [[nodiscard]] TComponent &GetComponentData(const EntityID &entityId) {
            ValidateID(entityId.GetId());
            return GetComponentDataArray<TComponent>()[entityId.GetId()];
        }

        template<typename TComponent>
            requires type_in<TComponent, TComponents...>
        [[nodiscard]] ComponentArray<TComponent> &GetComponentDataArray() {
            return std::get<ComponentArray<TComponent>>(componentArrays);
        }

        inline void ValidateID(size_t index) const {
            if (index >= endSlot) {
                throw std::out_of_range("Accessing outside of endSlot!");
            }
            if (endSlot >= slots) {
                throw std::out_of_range("Used up all slots!");
            }
        }

        [[nodiscard]] size_t GetLastSlot() const {
            if (endSlot == 0) {
                return 0;
            }
            return endSlot - 1;
        }

        size_t endSlot = 0;
        EntitiesSlots entities;
        ComponentArrays componentArrays{};
    };

    template<typename TECSManager, typename... TComponents>
    struct FilterIterator {
    private:
        using TInternalIterator = typename TECSManager::EntitiesSlots::const_iterator;
    public:
        FilterIterator(TECSManager &ecs, TInternalIterator it) : ecs(ecs), it(it) {}

        auto operator*() const { return ecs.template GetSeveral<TComponents ...>(it->id); }
        FilterIterator &operator++() {
            it++;
            auto endIt = ecs.end();
            while (it != endIt && (!it->isEntityActive || !ecs.template Has<TComponents ...>(it->id))) {
                it++;
            }
            return *this;
        }
        friend bool operator==(const FilterIterator &a, const FilterIterator &b) { return a.it == b.it; };
        friend bool operator!=(const FilterIterator &a, const FilterIterator &b) { return a.it != b.it; };

    private:
        TECSManager& ecs;
        TInternalIterator it;
    };

    template<typename TECSManager, typename... TComponents>
    struct Filter {
        using TFilterIterator = FilterIterator<TECSManager, TComponents...>;

        explicit Filter(TECSManager &ecs) : ecs(ecs) {}
        [[nodiscard]] TFilterIterator begin() {
            auto endIt = ecs.end();
            auto it = ecs.begin();
            while (it != endIt && (!it->isEntityActive || !ecs.template Has<TComponents ...>(it->id))) {
                it++;
            }
            return TFilterIterator(ecs, it);
        }
        [[nodiscard]] TFilterIterator end() { return TFilterIterator(ecs, ecs.end()); }

    private:
        TECSManager& ecs;
    };
}// namespace ecs
