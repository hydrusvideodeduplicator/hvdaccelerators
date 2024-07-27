#include <algorithm>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/hashing/bufferhasher.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>
#include <vpdq/cpp/hashing/hasher.h>

#include <tuple>
#include <vector>
#include <string>

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

std::tuple<py::bytes, int> hash_frame(py::bytes &img, size_t width, size_t height)
{
    auto hasher = facebook::vpdq::hashing::FrameBufferHasherFactory::createFrameHasher(width, height);
    facebook::pdq::hashing::Hash256 result{};
    // TODO: This will be ridiculously slow. Figure out how to use the raw bytes without copying.
    std::string img_str{img};
    int quality{};
    hasher->hashFrame(reinterpret_cast<unsigned char *>(img_str.data()), result, quality);
    facebook::vpdq::hashing::vpdqFeature{};
    return std::make_tuple(result.format(), quality);
}

using facebook::vpdq::hashing::GenericFrame;
using facebook::vpdq::hashing::VideoMetadata;
using facebook::vpdq::hashing::vpdqFeature;
using facebook::vpdq::hashing::VpdqHasher;

/** @brief String class for video frames. Stores pixels in its buffer which are
 *         used by PDQ for hashing.
 **/
class StringVideoFrame {
 public:
  /** @brief Constructor
   *
   *  @param buffer The pixel buffer used for PDQ hashing
   *  @param frameNumber The frame number in the video.
   **/
  StringVideoFrame(std::string buffer, uint64_t frameNumber)
      : m_buffer(std::move(buffer)), m_frameNumber(frameNumber){};

  /** @brief Get the frame number.
   *
   *  @return The frame number.
   **/
  uint64_t get_frame_number() const { return m_frameNumber; }

  /** @brief Get the pointer to the frame data buffer to be used for hashing.
   *
   *  @return Pointer to the frame data buffer.
   **/
  unsigned char* get_buffer_ptr() { return reinterpret_cast<unsigned char*>(m_buffer.data()); }

  std::string m_buffer;
  uint64_t m_frameNumber;
};

class Hasher
{
public:
    Hasher(float framerate, uint32_t width, uint32_t height) : Hasher{framerate, width, height, 0}
    {
    }

    Hasher(float framerate, uint32_t width, uint32_t height, uint32_t thread_count) : m_hasher{thread_count, VideoMetadata{
                                                                                                                 framerate, width, height}}
    {
    }

    void hash_frame(py::bytes &img)
    {
        auto make_frame = [this, &img]()
        {
            // TODO: This will be ridiculously slow. Figure out how to use the raw bytes without copying.
            std::string img_str{img};
            StringVideoFrame frame{img_str, m_frame_num};
            ++m_frame_num;
            return frame;
        };

        m_hasher.push_back(make_frame());
    }

    std::vector<vpdqFeature> finish()
    {
        return m_hasher.finish();
    }


    std::string go()
    {
        return "foo";
    }

private:
    VpdqHasher<StringVideoFrame> m_hasher;
    uint64_t m_frame_num{0U};
};

PYBIND11_MODULE(stuff, m)
{
    m.doc() = "hvdaccelerators plugin to make stuff fast";

    py::class_<Hasher>(m, "Hasher").def(py::init<uint32_t, float, uint32_t, uint32_t>()).def(py::init<uint32_t, float, uint32_t>()).def("go", &Hasher::go).def("finish", &Hasher::finish).def("hash_frame", &Hasher::hash_frame);

    py::class_<facebook::vpdq::hashing::vpdqFeature>(m, "vpdqFeature").def(py::init<>()).def("get_hash", &vpdqFeature::get_hash).def("get_frame_number", &vpdqFeature::get_frame_number).def("get_quality", &vpdqFeature::get_quality).def("get_timestamp", &vpdqFeature::get_timestamp);

    m.def("add", &add, "A function that adds two numbers");
    m.def("hamming_distance", &hamming_distance, "A function that adds two numbers");
    m.def("hash_frame", &hash_frame, "hash a frame");
}
