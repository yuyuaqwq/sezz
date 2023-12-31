#include <thread>
#include <iostream>
#include <fstream>

#include <sezz/sezz.hpp>

#include <sezz/stl/array.hpp>
#include <sezz/stl/tuple.hpp>
#include <sezz/stl/set.hpp>
#include <sezz/stl/map.hpp>
#include <sezz/stl/vector.hpp>
#include <sezz/stl/string.hpp>
#include <sezz/stl/memory.hpp>

class Invasive {
public:
    Invasive() :str_() {        // Default constructor required
        int_ = 0;
    }

    Invasive(std::string_view str, int _int, std::set<int> set) :str_(str), int_(_int), set_{ set } {

    }

    ~Invasive() {

    }

    template <class Archive>
    void Serialize(Archive& ar) {
        ar.Save(set_, str_, int_);
    }

    template <class Archive>
    void Deserialize(Archive& ar) {
        ar.Load(set_, str_, int_);
    }
    
private:
    std::string str_;
    int int_;
    std::set<int> set_;
};




class NonIntrusive {
public:     // Non intrusive, requiring external access to data members
    std::string str = "";
    int aaa = 0;
};


namespace sezz {
// specialization of function templates

template <class Archive>
void Serialize(Archive& ar, NonIntrusive& val) {
    ar.Save(val.str, val.aaa);
}


// example of overloaded return values
template <class T, class Archive>
    requires std::is_same_v<T, NonIntrusive>
T Deserialize(Archive& ar) {
    NonIntrusive val;
    ar.Load(val.str, val.aaa);
    return val;
}

// example of parameter overloading
template <class Archive>
void Deserialize(Archive& ar, NonIntrusive& val) {
    ar.Load(val.str, val.aaa);
}

} // namespace sezz



int main() {

    std::fstream fs;
    fs.open("test.bin", std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
    sezz::BinaryOutputArchive outar(fs);

    // Optional version number
    outar.SaveVersion(1);


    double test_double = 114.514;
    outar.Save(test_double);

    float test_float = 114.514;
    outar.Save(test_float);

    std::shared_ptr<int> test_shared1 = std::make_shared<int>(10000);
    outar.Save(test_shared1);

    std::shared_ptr<int> test_shared2 = test_shared1;
    outar.Save(test_shared2);

    std::weak_ptr<int> test_weak {test_shared1};
    outar.Save(test_weak);

    std::unique_ptr<int> test_unique = std::make_unique<int>(10000);
    outar.Save(test_unique);

    int* test_raw_ptr = test_unique.get();
    outar.Save(test_raw_ptr);

    std::array<int, 100> test_array = { 1123070,13213,341432423,432234,42334324,43141 };
    outar.Save(test_array);

    std::tuple<std::string, int> test_tuple = { "tuple_str", 1143141323 };
    outar.Save(test_tuple);

    std::string test_str = "abc";
    outar.Save(test_str);

    std::map<std::string, std::string> test_map {
        { "pair_key_1", "pair_value_1" },
        { "pair_key_2", "pair_value_2" }
    };
    outar.Save(test_map);

    std::vector<std::string> test_vector{ 
        "vector_1", 
        "vector_2", 
        "vector_3"
    };
    outar.Save(test_vector);

    std::vector<std::vector<std::string>> test_vector2 {
        { "vector_1_1", "vector_2_2" },
        { "vector_2_1", "vector_2_2" },
        { "vector_3_1", "vector_3_2" },
    };
    outar.Save(test_vector2);

    Invasive test_invasive{ "str1", 2, {1,2,3} };
    outar.Save(test_invasive);

    NonIntrusive test_non_intrusive{ "str1", 2};
    outar.Save(test_non_intrusive);



    fs.seekg(0);
    sezz::BinaryInputArchive inar(fs);

    // Optional version number, 
    // If SaveVersion() is called, please ensure loading first
    inar.LoadVersion();
    // Please use GetVersion() where version control is required


    auto test_double_de = inar.Load<double>();
    auto test_float_de = inar.Load<float>();
    auto test_shared1_de = inar.Load<std::shared_ptr<int>>();
    auto test_shared2_de = inar.Load<std::shared_ptr<int>>();
    auto test_weak_de = inar.Load<std::weak_ptr<int>>();
    auto test_unique_de = inar.Load<std::unique_ptr<int>>();
    auto test_raw_ptr_de = inar.Load<int*>();
    auto test_array_de = inar.Load<std::array<int, 100>>();
    auto test_tuple_de = inar.Load<std::tuple<std::string, int>>();
    auto test_str_de = inar.Load<std::string>();
    // match based on return value or parameters
    auto test_map_de = inar.Load<std::map<std::string, std::string>>();
    auto test_vector_de = inar.Load<std::vector<std::string>>();
    auto test_vector_de2 = inar.Load<std::vector<std::vector<std::string>>>();
    auto test_invasive_de = inar.Load<Invasive>();
    auto test_non_intrusive_de = inar.Load<NonIntrusive>();

     //NonIntrusive test_non_intrusive_de;
     //ar.Load(fs, test_non_intrusive_de);

    std::cout << "ok\n";
}
