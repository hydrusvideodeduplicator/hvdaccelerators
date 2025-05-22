#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/hashing/bufferhasher.h>
#include <vpdq/cpp/hashing/hasher.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace py = pybind11;

int hamming_distance(std::string const& a, std::string const& b)
{
    facebook::pdq::hashing::Hash256 hash_a{ a.c_str() };
    facebook::pdq::hashing::Hash256 hash_b{ b.c_str() };
    return hash_a.hammingDistance(hash_b);
}

std::tuple<py::bytes, int> hash_frame(py::bytes& img, size_t width, size_t height)
{
    auto hasher = facebook::vpdq::hashing::FrameBufferHasherFactory::createFrameHasher(width, height);
    facebook::pdq::hashing::Hash256 result{};
    std::string img_str{ img };
    int quality{};
    hasher->hashFrame(reinterpret_cast<unsigned char*>(img_str.data()), result, quality);
    return std::make_tuple(result.format(), quality);
}

using facebook::vpdq::hashing::GenericFrame;
using facebook::vpdq::hashing::VideoMetadata;
using facebook::vpdq::hashing::vpdqFeature;
using facebook::vpdq::hashing::VpdqHasher;

/** @brief String class for video frames. Stores pixels in its buffer which are
 *         used by PDQ for hashing.
 **/
class StringVideoFrame
{
public:
    /** @brief Constructor
     *
     *  @param buffer The pixel buffer used for PDQ hashing
     *  @param frameNumber The frame number in the video.
     **/
    StringVideoFrame(std::string buffer, uint64_t frameNumber) : m_buffer(std::move(buffer)), m_frameNumber(frameNumber) {};

    /** @brief Get the frame number.
     *
     *  @return The frame number.
     **/
    uint64_t get_frame_number() const
    {
        return m_frameNumber;
    }

    /** @brief Get the pointer to the frame data buffer to be used for hashing.
     *
     *  @return Pointer to the frame data buffer.
     **/
    unsigned char* get_buffer_ptr()
    {
        return reinterpret_cast<unsigned char*>(m_buffer.data());
    }

    std::string m_buffer;
    uint64_t m_frameNumber;
};

/// @brief Calculate the number of threads to pass to vpdq.
///
/// HVD allows the user to pass in a negative number for the job count, which means "all but n" cores available
/// on their PC. For example, if they have 8 cores, and they pass in -2 for the job count, then the hasher should
/// use 8 - 2 = 6 threads. This function does this calculation.
static unsigned int fix_negative_thread_count(int thread_count)
{
    // vpdq will determine the thread count if >=0, so just return.
    if (thread_count >= 0) {
        return thread_count;
    }

    auto const num_hardware_threads = std::thread::hardware_concurrency();

    // Some platforms may return 0 for hardware_concurrency(), per the cpp standard.
    // If that occurs, set it to single-threaded.
    if (num_hardware_threads == 0) {
        return 1;
    }

    // If we are subtracting too many, then set it to single-threaded.
    auto const abs_thread_count = std::abs(thread_count);
    if (abs_thread_count >= num_hardware_threads) {
        return 1;
    }

    // Otherwise, use all but n threads.
    return num_hardware_threads - abs_thread_count;
}

class VideoHasher
{
public:
    VideoHasher(float framerate, uint32_t width, uint32_t height) : VideoHasher{ framerate, width, height, 0 }
    {
    }

    VideoHasher(float framerate, uint32_t width, uint32_t height, int thread_count)
        : m_hasher{ fix_negative_thread_count(thread_count), VideoMetadata{ framerate, width, height } }
    {
    }

    void hash_frame(py::bytes& img)
    {
        auto make_frame = [this, &img]() {
            StringVideoFrame frame{ img, m_frame_num };
            ++m_frame_num;
            return frame;
        };

        m_hasher.push_back(make_frame());
    }

    std::vector<vpdqFeature> finish()
    {
        return m_hasher.finish();
    }

    VideoHasher() = delete;

private:
    VpdqHasher<StringVideoFrame> m_hasher;
    uint64_t m_frame_num{ 0U };
};

PYBIND11_MODULE(vpdq, m)
{
    m.doc() = "hvdaccelerators plugin to make stuff fast";

    py::class_<VideoHasher>(m, "VideoHasher")
        .def(py::init<float, uint32_t, uint32_t>())
        .def(py::init<float, uint32_t, uint32_t, int>())
        .def("finish", &VideoHasher::finish)
        .def("hash_frame", &VideoHasher::hash_frame);

    py::class_<facebook::vpdq::hashing::vpdqFeature>(m, "vpdqFeature")
        .def(py::init<>())
        .def("get_hash", &vpdqFeature::get_hash)
        .def("get_frame_number", &vpdqFeature::get_frame_number)
        .def("get_quality", &vpdqFeature::get_quality);

    m.def("hamming_distance", &hamming_distance, "Calculate the hamming distance between two PDQ hashes.");
    m.def("hash_frame", &hash_frame, "hash a frame");
}
