#ifndef SEZZ_STL_SET_HPP_
#define SEZZ_STL_SET_HPP_

#include <set>

#include <sezz/type_traits.hpp>
#include <sezz/stl/index.hpp>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<T, std::set<type_traits::place_t>>
void Serialize(Archive& os, T& val) {
    SerializeIndex(os, val);
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<T, std::set<type_traits::place_t>>
T Deserialize(Archive& is) {
    return DeserializeIndex<T>(is);
}

} // namespace sezz


#endif // SEZZ_STL_SET_HPP_