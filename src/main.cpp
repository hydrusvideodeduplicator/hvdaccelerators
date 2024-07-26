#include <pybind11/pybind11.h>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/hashing/bufferhasher.h>

namespace py = pybind11;

int add(int i, int j)
{
    return i + j;
}

int hamming_distance(std::string const &a, std::string const &b)
{
    facebook::pdq::hashing::Hash256 hash_a{a.c_str()};
    facebook::pdq::hashing::Hash256 hash_b{b.c_str()};
    return hash_a.hammingDistance(hash_b);
}

py::bytes hash_frame(py::bytes &img, size_t width, size_t height)
{
    auto hasher = facebook::vpdq::hashing::FrameBufferHasherFactory::createFrameHasher(width, height);
    facebook::pdq::hashing::Hash256 result{};
    int quality{};
    std::string img_str{img};
    hasher->hashFrame(reinterpret_cast<unsigned char *>(img_str.data()), result, quality);
    return result.format();
}

PYBIND11_MODULE(stuff, m)
{
    m.doc() = "hvdaccelerators plugin to make stuff fast";

    m.def("add", &add, "A function that adds two numbers");
    m.def("hamming_distance", &hamming_distance, "A function that adds two numbers");
    m.def("hash_frame", &hash_frame, "hash a frame");
}
