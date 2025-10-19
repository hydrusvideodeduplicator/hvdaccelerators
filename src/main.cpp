#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <pdq/cpp/common/pdqhashtypes.h>
#include <pdq/cpp/hashing/bufferhasher.h>
#include <vpdq/cpp/hashing/hasher.h>
#include <vpdq/cpp/hashing/vpdqHashType.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace py = pybind11;

int hamming_distance(std::string const& a, std::string const& b)
{
    return facebook::pdq::hashing::hammingDistanceStrings(a, b);
}

std::tuple<py::bytes, int> hash_frame(py::bytes& img, std::size_t width, std::size_t height)
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
using facebook::vpdq::hashing::VpdqHash;
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
    StringVideoFrame(std::string buffer, std::uint64_t frameNumber) : m_buffer(std::move(buffer)), m_frameNumber(frameNumber) {};

    /** @brief Get the frame number.
     *
     *  @return The frame number.
     **/
    std::uint64_t get_frame_number() const
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
    std::uint64_t m_frameNumber;
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
    VideoHasher(float framerate, std::uint32_t width, std::uint32_t height) : VideoHasher{ framerate, width, height, 0 }
    {
    }

    VideoHasher(float framerate, std::uint32_t width, std::uint32_t height, int thread_count)
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

    facebook::vpdq::hashing::VpdqHash finish()
    {
        return m_hasher.finish();
    }

    VideoHasher() = delete;

private:
    VpdqHasher<StringVideoFrame> m_hasher;
    std::uint64_t m_frame_num{ 0U };
};

namespace hvdaccelerators
{

/**
 * @brief Get the number of matches between two feature vectors
 *
 * @param features1 Features to match
 * @param features2 Features to match
 * @param distanceTolerance Distance tolerance of considering a match. Lower is
 * more similar.
 *
 * @return Number of matches
 */
static std::uint32_t findMatches(const std::string_view features1, const std::string_view features2, const int distanceTolerance)
{
    // Check for out of bounds.
    if (((features1.size() % VpdqHash::bytesPerPdqHash) != 0U) || ((features2.size() % VpdqHash::bytesPerPdqHash) != 0U)) {
        return 0;
    }

    std::uint32_t matchCnt = 0;
    for (std::size_t i = 0; i < features1.size(); i += VpdqHash::bytesPerPdqHash) {
        const char* const pdqHash1 = features1.data() + i;
        for (std::size_t j = 0; j < features2.size(); j += VpdqHash::bytesPerPdqHash) {
            const char* const pdqHash2 = features2.data() + j;
            if (facebook::pdq::hashing::hammingDistanceSpan(pdqHash1, pdqHash2) < distanceTolerance) {
                ++matchCnt;
                break;
            }
        }
    }
    return matchCnt;
}

int matchHash(const std::string_view qHashes, const std::string_view tHashes, const int distanceTolerance)
{
    // Avoid divide-by-zero. Also if a video had all hashes filtered out because there were too low quality
    // then the hashes may be empty.
    if (qHashes.empty() || tHashes.empty()) {
        return 0.0;
    }

    // Get count of query in target and target in query
    auto qMatchCnt = findMatches(qHashes, tHashes, distanceTolerance);

    return (qMatchCnt * 100.0) / (qHashes.size() / facebook::vpdq::hashing::VpdqHash::bytesPerPdqHash);
}

int matchHashBytes(const py::bytes& qHashes, const py::bytes& tHashes, const int distanceTolerance)
{
    return matchHash(std::string_view{ qHashes }, std::string_view{ tHashes }, distanceTolerance);
}

int matchHashVpdqHash(const facebook::vpdq::hashing::VpdqHash& qHashes, const facebook::vpdq::hashing::VpdqHash& tHashes,
                      const int distanceTolerance)
{
    return matchHash(std::string_view{ qHashes.pdqHashes }, std::string_view{ tHashes.pdqHashes }, distanceTolerance);
}

} // namespace hvdaccelerators

PYBIND11_MODULE(vpdq, m)
{
    m.doc() = "hvdaccelerators contains C++ implementations of Hydrus Video Deduplicator algorithms to improve performance.";

    py::class_<VideoHasher>(m, "VideoHasher")
        .def(py::init<float, std::uint32_t, std::uint32_t>())
        .def(py::init<float, std::uint32_t, std::uint32_t, int>())
        .def("finish", &VideoHasher::finish)
        .def("hash_frame", &VideoHasher::hash_frame);

    py::class_<facebook::vpdq::hashing::vpdqFeature>(m, "vpdqFeature")
        .def(py::init<>())
        .def_readonly("pdqHash", &vpdqFeature::pdqHash)
        .def("from_str", &vpdqFeature::from_str)
        .def("to_string", &vpdqFeature::to_string)
        .def("get_hash", &vpdqFeature::get_hash)
        .def("__str__", &vpdqFeature::to_string)
        .def("__repr__", &vpdqFeature::to_string);

    py::class_<facebook::vpdq::hashing::VpdqHash>(m, "VpdqHash")
        .def(py::init<>())
        .def_readonly("pdqHashes", &facebook::vpdq::hashing::VpdqHash::pdqHashes)
        .def("__str__", &facebook::vpdq::hashing::VpdqHash::to_string)
        .def("__repr__", &facebook::vpdq::hashing::VpdqHash::to_string)
        .def("__len__", &facebook::vpdq::hashing::VpdqHash::getHashCount)
        .def("__eq__", &facebook::vpdq::hashing::VpdqHash::operator==)
        .def("__ne__", &facebook::vpdq::hashing::VpdqHash::operator!=)
        .def("from_bytes", &facebook::vpdq::hashing::VpdqHash::from_bytes)
        .def_property(
            "bytes", [](const facebook::vpdq::hashing::VpdqHash& s) -> py::bytes { return py::bytes(s.pdqHashes); },
            [](facebook::vpdq::hashing::VpdqHash& s, py::bytes b) { s.pdqHashes = b.cast<std::string>(); })
        .def("from_string", &facebook::vpdq::hashing::VpdqHash::from_string)
        .def("empty", &facebook::vpdq::hashing::VpdqHash::empty)
        .def_readonly_static("bytesPerPdqHash", &facebook::vpdq::hashing::VpdqHash::bytesPerPdqHash);

    py::class_<facebook::pdq::hashing::Hash256>(m, "PdqHash256")
        .def(py::init<>())
        .def("fromHexString", &facebook::pdq::hashing::Hash256::fromHexString)
        .def("toHexString", &facebook::pdq::hashing::Hash256::toHexString)
        .def("hammingDistanceLE", &facebook::pdq::hashing::Hash256::hammingDistanceLE)
        .def("__str__", &facebook::pdq::hashing::Hash256::toHexString)
        .def("__repr__", &facebook::pdq::hashing::Hash256::toHexString)
        .def_readonly_static("HASH256_HEX_NUM_NYBBLES", &facebook::pdq::hashing::Hash256::HASH256_HEX_NUM_NYBBLES);

    m.def("matchHashBytes", &hvdaccelerators::matchHashBytes, "Calculate the similarity between two VPDQ hashes (raw bytes).");
    m.def("matchHash", &hvdaccelerators::matchHashVpdqHash, "Calculate the similarity between two VPDQ hashes.");
    m.def("hamming_distance", &hamming_distance, "Calculate the hamming distance between two PDQ hashes.");
    m.def("hammingDistanceStrings", &facebook::pdq::hashing::hammingDistanceStrings,
          "Calculate the hamming distance between two PDQ hashes.");
    m.def("hash_frame", &hash_frame, "hash a frame");
}
