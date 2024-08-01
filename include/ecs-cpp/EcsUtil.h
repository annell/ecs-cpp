//
// Created by Stefan Annell on 2023-12-12.
//

#pragma once

template<typename TypeToCheck, typename... TypesToCheckAgainst>
concept TypeIn = (std::same_as<std::remove_cvref_t<TypeToCheck>, TypesToCheckAgainst> || ...);

template<typename TypeToCheck, typename... TypesToCheckAgainst>
constexpr bool TypeInPack() {
    return (std::same_as<typename std::remove_cvref_t<TypeToCheck>, TypesToCheckAgainst> || ...);
}

template<typename TypeToCheck, typename... TypesToCheckAgainst>
constexpr bool ComponentTypeInPack() {
    return (std::same_as<typename std::remove_cvref_t<TypeToCheck>::TComponentRange, TypesToCheckAgainst> || ...);
}

template <typename... Args>
concept NonVoidArgs = sizeof...(Args) > 0;

template<typename U, typename... T>
constexpr bool Contains(std::tuple<T...>) {
    return std::disjunction_v<std::is_same<U, T>...>;
}

template<typename T, typename Attributes>
concept ContainedIn = Contains<T>(typename Attributes::AttributeList{});

template <typename TEffect, typename Attributes>
concept IsEffect =
    requires(TEffect t, TEffect::Attribute& a, const Attributes::AttributeList& attributeList) {
    { std::get<typename TEffect::Attribute>(attributeList) };
    { t.Apply(a) };
} && Contains<typename TEffect::Attribute>(typename Attributes::AttributeList{});

template <typename TAttributes>
concept IsAttributes = requires(typename TAttributes::AttributeList attributeList) {
    { std::get<0>(attributeList) };
};

template<typename ... TComponent>
concept IsBasicType = ((
        std::default_initializable<TComponent> &&
        not std::is_pointer_v<TComponent> &&
        not std::is_reference_v<TComponent> &&
        not std::is_const_v<TComponent> &&
        not std::is_volatile_v<TComponent>) &&
        ...);

template <typename T>
void PushToVector(std::vector<T>& vector) {
    vector.push_back(T{});
}
