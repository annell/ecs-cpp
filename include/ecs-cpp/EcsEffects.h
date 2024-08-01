//
// Created by Stefan Annell on 2021-05-12.
//

#pragma once

#include "EcsCpp.h"
#include "EntityID.h"
#include "EcsUtil.h"


namespace ecs {
    template <typename T>
    class Attribute {
    public:
        Attribute(T t) : Base(t), PostEffects(t) {}
        T& Get() {
            return PostEffects;
        }

        /**
         * Internal use only
         */
        void Reset() {
            PostEffects = Base;
        }
    private:
        T PostEffects;
        T Base;
    };

    template <typename ... TAttributes>
    struct Attributes {
        using AttributeList = std::tuple<TAttributes ...>;
    };


    /**
     *  A effect is a temporary modification of a attribute, which is described in ECS.
     *  An effect can be applied to one or more entities, and will modify the attribute of the entity.
     *  The effect can be removed, and the attribute will return to its original value.
     *
     *  The EcsEffect is a addition to EcsCpp, so EcsCpp does not depend on it and
     *  to use it, you would only access your entities attributes through the
     *  EcsEffect system instead of the regular ECSManager.
     *
     *  State is a regular value, and the attribute is a value tracked in the AttributesEffectsManager.
     */
    template <IsAttributes TAttributes, IsEffect<TAttributes> ... TEffects>
    class AttributesEffectsManager {
    public:
        using EffectID = size_t;
        template <IsEffect<TAttributes> TEffect>
        struct EffectElement {
            EffectID Id = 0;
            TEffect Effect;
        };
        template <IsEffect<TAttributes> TEffect>
        using EffectList = std::vector<EffectElement<TEffect>>;

        /**
         * Add a effect to a entity.
         * @tparam TAddedEffects
         * @param entity
         * @param effects
         */
        template <IsEffect<TAttributes>... TAddedEffects>
        void Add(EntityID entity, TAddedEffects&&... effects) {
            for (auto& affectedEntity : AffectedEntities) {
                if (affectedEntity.Entity == entity) {
                    (std::get<EffectList<TAddedEffects>>(affectedEntity.Effects).push_back(EffectElement<TAddedEffects>{EffectIdCounter++, effects}), ...);
                    return;
                }
            }
            AffectedEntities.push_back({entity, EffectMatrix{}, AttributeBase});
            (std::get<EffectList<TAddedEffects>>(AffectedEntities.back().Effects).push_back(EffectElement<TAddedEffects>{EffectIdCounter++, effects}), ...);
        }

        /**
         * Remove all effects from a entity and resets its attributes to base.
         */
        void Reset(EntityID entity) {
            auto it = std::find_if(AffectedEntities.begin(), AffectedEntities.end(), [&entity](auto&& entities){
                return entities.Entity == entity;
            });
            if (it == AffectedEntities.end()) {
                return;
            }
            AffectedEntities.erase(it);
        }

        /**
         * Modify a attribute of a entity.
         */
        template <ContainedIn<TAttributes>... TModifiedAttributes>
        void Modify(EntityID entity, TModifiedAttributes&&... attributes) {
            if constexpr (sizeof...(TModifiedAttributes) == 0) {
                static_assert("Not allowed to pass 0 parameters to modify.");
            }
            auto it = std::find_if(AffectedEntities.begin(), AffectedEntities.end(), [&entity](auto&& entities){
                return entities.Entity == entity;
            });
            if (it == AffectedEntities.end()){
                AffectedEntities.push_back({entity, EffectMatrix{}, AttributeBase});
                it = AffectedEntities.end() - 1;
            }
            ((std::get<TModifiedAttributes>(it->AttributeBase) = attributes), ...);
        }

        /**
         * Get a attribute of a entity.
         */
        template <ContainedIn<TAttributes> TAttribute>
        TAttribute Get(EntityID entity) const {
            auto it = std::find_if(AffectedEntities.begin(), AffectedEntities.end(), [&entity](auto&& entities){
                return entities.Entity == entity;
            });
            if (it == AffectedEntities.end()){
                return GetBase<TAttribute>();
            }
            return it->template Apply<TAttribute>();
        }

        /**
         * Get a list of all effects on the given attribute for the entity.
         */
        template <IsEffect<TAttributes> TEffect>
        const EffectList<TEffect>& GetEffects(EntityID entity) {
            auto it = std::find_if(AffectedEntities.begin(), AffectedEntities.end(), [&entity](auto&& entities){
                return entities.Entity == entity;
            });
            if (it == AffectedEntities.end()) {
                static EffectList<TEffect> empty;
                return empty;
            }
            return std::get<EffectList<TEffect>>(it->Effects);
        }

        /**
         * Removes a list of effects from a entity.
         */
        template <IsEffect<TAttributes> TEffect>
        void Remove(EntityID entity, const EffectList<TEffect>& effects) {
            auto it = std::find_if(AffectedEntities.begin(), AffectedEntities.end(), [&entity](auto&& entities){
                return entities.Entity == entity;
            });
            if (it == AffectedEntities.end()) {
                return;
            }

            auto& effectList = std::get<EffectList<TEffect>>(it->Effects);
            for (auto& effect : effects) {
                auto effectIt = std::find_if(effectList.begin(), effectList.end(), [&effect](auto& effectElement){
                    return effectElement.Id == effect.Id;
                });
                if (effectIt != effectList.end()) {
                    effectList.erase(effectIt);
                }
            }
        }
    private:
        template <typename TAttribute>
        const TAttribute& GetBase() const {
            return std::get<TAttribute>(AttributeBase);
        }
        using EffectMatrix = std::tuple<EffectList<TEffects>...>;
        struct AffectedEntity {
            EntityID Entity;
            EffectMatrix Effects;
            TAttributes::AttributeList AttributeBase;

            template <typename TAttribute>
            TAttribute Apply() const {
                auto base = std::get<TAttribute>(AttributeBase);
                std::apply([&](auto&... effects) {
                    ((
                        std::for_each(effects.begin(), effects.end(), [&](auto& effectElement) {
                            auto& effect = effectElement.Effect;
                            using EffectAttribute = typename std::remove_reference_t<decltype(effect)>::Attribute;
                            if constexpr (std::is_same_v<TAttribute, EffectAttribute>) {
                                effect.Apply(base);
                            }
                        })
                    ), ...);
                }, Effects);
                return base;
            }
        };

        TAttributes::AttributeList AttributeBase;
        std::vector<AffectedEntity> AffectedEntities;
        EffectID EffectIdCounter = 0;
    };
}// namespace ecs
