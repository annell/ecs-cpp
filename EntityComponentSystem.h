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

        bool operator==(const EntityID &rhs) const {
            return id == rhs.id;
        }
    private:
        ID id;
    };

    template<typename TypeToCheck, typename... TypesToCheckAgainst>
    concept type_in = (std::same_as<std::remove_cvref_t<TypeToCheck>, TypesToCheckAgainst> || ...);

    template<typename... TComponents>
    class ECSManager {
    public:
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

        [[nodiscard]] auto begin() const {
            return entities.begin();
        }

        [[nodiscard]] auto end() const {
            return entities.begin() + endSlot;
        }

        template<typename TComponent>
        struct ComponentActive {
            bool active = false;
        };
        using ComponentsActive = std::tuple<ComponentActive<TComponents>...>;

        template<typename TComponent>
        [[nodiscard]] bool IsComponentActive(const ComponentsActive &components) const {
            return std::get<ComponentActive<TComponent>>(components).active;
        }

    private:
        constexpr static int slots = 1024;

        template<typename TComponent>
        using ComponentArray = std::array<TComponent, slots>;
        using ComponentArrays = std::tuple<ComponentArray<TComponents>...>;

        struct Entity {
            ComponentsActive componentsActive{};
            bool isEntityActive = false;
            EntityID id = EntityID(0);
        };
        using EntitiesSlots = std::array<Entity, slots>;

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

    template<typename... TComponents>
    struct Iterator {
        using value_type = std::tuple<TComponents...>;

        Iterator(size_t slot, size_t usedSlots) : slot(slot), usedSlots(usedSlots) {}

        value_type &operator*() const { return *m_ptr; }
        value_type *operator->() { return m_ptr; }
        Iterator &operator++() {
            slot++;
            return *this;
        }
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        friend bool operator==(const Iterator &a, const Iterator &b) { return a.slot == b.slot; };
        friend bool operator!=(const Iterator &a, const Iterator &b) { return a.slot != b.slot; };

    private:
        size_t slot = 0;
        const size_t usedSlots = 0;
        value_type *m_ptr = nullptr;
    };

    template<typename... TComponents>
    struct Filter {

        explicit Filter(auto &ecs) {}
        [[nodiscard]] Iterator<TComponents...> begin() { return Iterator<TComponents...>(0, 0); }
        [[nodiscard]] Iterator<TComponents...> end() { return Iterator<TComponents...>(0, 0); }

    private:
        //auto* ecs;
    };
}// namespace ecs
