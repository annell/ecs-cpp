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
