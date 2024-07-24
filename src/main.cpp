#include <pybind11/pybind11.h>

#include <pdq/cpp/common/pdqhashtypes.h>

int add(int i, int j) {
    return i + j;
}

int hamming_distance(std::string const& a, std::string const& b) {
    facebook::pdq::hashing::Hash256 hash_a{a.c_str()};
    facebook::pdq::hashing::Hash256 hash_b{b.c_str()};
    return hash_a.hammingDistance(hash_b);
}

PYBIND11_MODULE(stuff, m) {
    m.doc() = "hvdaccelerators plugin to make stuff fast"; // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
    m.def("hamming_distance", &hamming_distance, "A function that adds two numbers");
}
