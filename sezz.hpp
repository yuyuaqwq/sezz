﻿#ifndef SEZZ_SEZZ_HPP_
#define SEZZ_SEZZ_HPP_

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <typeinfo>

/*
* 鸣谢：
* 1.https://www.cnblogs.com/mmc1206x/p/11053826.html
* 2.计都
*/

namespace sezz {

namespace details {
/* 判断是否是std::pair(type版) */
template<typename T>
struct is_pair_t : std::false_type {};
template<typename T1, typename T2>
struct is_pair_t<std::pair<T1, T2>> : std::true_type {};

/* 判断是否是std::unordered_map(type版) */
template<typename T>
struct is_unordered_map_t : std::false_type {};
template<typename T1, typename T2>
struct is_unordered_map_t<std::unordered_map<T1, T2>> : std::true_type {};

/* 判断是否是std::basic_string(type版) */
template<typename T>
struct is_string_t : std::false_type {};
template<typename T>
struct is_string_t<std::basic_string<T>> : std::true_type {};

/* 判断是否可迭代(type版) */
template<typename T, typename = void>
struct is_iterable_t : std::false_type {};
template<typename T>
struct is_iterable_t<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};
} // namespace details

template <class>
constexpr bool always_false = false;

/* 判断是否可memcopy(value版) */
template <class T>
constexpr bool is_memcopyable_v = std::is_trivially_copyable_v<T>;
/* 判断是否可迭代(value版) */
template <class T>
constexpr bool is_iterable_v = details::is_iterable_t<T>::value;
/* 判断是否是std::unordered_map(value版) */
template <class T>
constexpr bool is_unordered_map_v = details::is_unordered_map_t<T>::value;
/* 判断是否是std::string(value版) */
template <class T>
constexpr bool is_string_v = details::is_string_t<T>::value;
/* 判断是否是std::pair(value版) */
template <class T>
constexpr bool is_pair_v = details::is_pair_t<T>::value;

//  可平凡复制
template <class T, typename std::enable_if_t<std::is_trivially_copyable_v<T>, int> N = 0>
void Serialize(std::ostream& os, const T& val) {
    os.write((const char*)&val, sizeof(T));
}

//  容器

template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())>&&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())>&&
    std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
void Serialize(std::ostream& os, const T& val) {
    unsigned int size = val.size();
    os.write((const char*)&size, sizeof(size));
    os.write((const char*)val.data(), size * sizeof(typename T::value_type));
}

template <class T1, class T2>
void Serialize(std::ostream& os, const std::pair<T1, T2>& val);

template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())>&&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())> &&
    !std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
void Serialize(std::ostream& os, const T& val) {
    unsigned int size = val.size();
    os.write((const char*)&size, sizeof(size));
    for (auto& v : val) { Serialize(os, v); }
}

// 对组支持
template <class T1, class T2>
void Serialize(std::ostream& os, const std::pair<T1, T2>& val) {
    Serialize(os, val.first);
    Serialize(os, val.second);
}


//  可平凡复制
template <class T>
T Deserialize(std::istream& is) {
    using DecayT = std::decay_t<T>;

    unsigned int size = 0;
    DecayT res{};

    /* 如果是std::pair类型 */
    if constexpr (is_pair_v<DecayT>) {
        auto first = Deserialize<typename T::first_type>(is);        // 反序列化first
        auto second = Deserialize<typename T::second_type>(is);      // 反序列化second
        ::new(&res) T(first, second);                                // 重构std::pair
    }
    /* 如果是可迭代的容器类型 */
    else if constexpr (is_iterable_v<DecayT>) {
        /* 如果是可直接内存复制的容器类型, 比如std::vector<char> */
        if constexpr (is_memcopyable_v<DecayT>) {
            is.read((char*)&size, sizeof(unsigned int));
            res.resize(size);
            is.read((char*)res.data(), size * sizeof(typename T::value_type));
        }
        /*
        不可直接内存复制的容器类型
        比如std::vector<std::string>
        std::string不可直接内存复制
        */
        else {
            is.read((char*)&size, sizeof(unsigned int));
            for (unsigned int i = 0; i < size; i++) {
                auto tmp = Deserialize<typename T::value_type>(is);     // 反序列化T的元素, 比如std::string
                /* 如果是std::basic_string */
                if constexpr (is_string_v<DecayT>)
                    res.insert(i, 1, tmp);
                /* 如果是std::unordered_map */
                else if constexpr (is_unordered_map_v<DecayT>)
                    res.insert(tmp);
                /* 剩下的交给你了! 注释的printf是可以在运行时查看还有什么类型没实现 */
                else {
                    //printf("T是还没有实现的容器: %s\n", typeid(DecayT).name()); throw;
                    static_assert(always_false<T>, "T是还没有实现的容器!");
                }
            }
        }
    }
    /* 如果是可直接内存复制的类型 */
    else if constexpr (is_memcopyable_v<T>) {
        is.read((char*)&res, sizeof(T));
    }
    else {
        //printf("无法解析的T类型: %s\n", typeid(T).name()); throw;
        static_assert(always_false<T>, "无法解析的T类型!");
    }
    return res;
}

} // namespace sezz



#endif // SEZZ_SEZZ_HPP_