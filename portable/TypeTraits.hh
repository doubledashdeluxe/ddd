#pragma once

namespace TypeTraits {

template <bool Cond, typename T = void>
struct EnableIf {};

template <typename T>
struct EnableIf<true, T> {
    typedef T type;
};

template <typename T>
struct RemoveCV {
    typedef T type;
};

template <typename T>
struct RemoveCV<const T> {
    typedef T type;
};

template <typename T>
struct RemoveCV<volatile T> {
    typedef T type;
};

template <typename T>
struct RemoveCV<const volatile T> {
    typedef T type;
};

namespace detail {

template <typename T>
struct IsGPROptimizedImpl {
    static const bool value = false;
};

template <>
struct IsGPROptimizedImpl<bool> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<char> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<signed char> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<unsigned char> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<wchar_t> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<short> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<unsigned short> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<int> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<unsigned int> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<long> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<unsigned long> {
    static const bool value = true;
};

template <typename T>
struct IsGPROptimizedImpl<T *> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<long long> {
    static const bool value = true;
};

template <>
struct IsGPROptimizedImpl<unsigned long long> {
    static const bool value = true;
};

template <typename T>
struct IsFPROptimizedImpl {
    static const bool value = false;
};

template <>
struct IsFPROptimizedImpl<float> {
    static const bool value = true;
};

template <>
struct IsFPROptimizedImpl<double> {
    static const bool value = true;
};

} // namespace detail

// This doesn't catch enums
template <typename T>
struct IsGPROptimized {
    static const bool value = detail::IsGPROptimizedImpl<typename RemoveCV<T>::type>::value;
};

template <typename T>
struct IsFPROptimized {
    static const bool value = detail::IsFPROptimizedImpl<typename RemoveCV<T>::type>::value;
};

template <typename T>
struct IsRegOptimized {
    static const bool value = IsGPROptimized<T>::value || IsFPROptimized<T>::value;
};

} // namespace TypeTraits
