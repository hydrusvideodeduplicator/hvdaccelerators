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
    return facebook::pdq::hashing::hammingDistanceStrings(a, b);
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

namespace hvdaccelerators
{

int matchHash(const std::vector<facebook::vpdq::hashing::vpdqFeature>& qHashes,
              const std::vector<facebook::vpdq::hashing::vpdqFeature>& tHashes, const int distanceTolerance, const int qualityTolerance);

/**
 * @brief Filter low quality hashes from a feature vector
 *
 * @param features Features to filter
 * @param qualityTolerance Quality tolerance of comparing two hashes. If lower
 * then it won't be included in the result
 * @param verbose Print skipped hashes
 *
 * @return Feature vector without features with quality lower than
 * qualityTolerance
 */
static std::vector<facebook::vpdq::hashing::vpdqFeature> filterFeatures(const std::vector<facebook::vpdq::hashing::vpdqFeature>& features,
                                                                        const int qualityTolerance, const bool verbose)
{
    std::vector<facebook::vpdq::hashing::vpdqFeature> filteredHashes;
    for (const auto& feature : features) {
        if (feature.quality >= qualityTolerance) {
            filteredHashes.push_back(feature);
        } else if (verbose) {
            auto index = &feature - &features[0];
            std::cout << "Skipping Line " << index << " Skipping Hash: " << feature.pdqHash.format()
                      << ", because of low quality: " << feature.quality << std::endl;
        }
    }
    return filteredHashes;
}

/**
 * @brief Get the number of matches between two feature vectors
 *
 * @param features1 Features to match
 * @param features2 Features to match
 * @param distanceTolerance Distance tolerance of considering a match. Lower is
 * more similar.
 * @param verbose Print features with matching hashes
 *
 * @return Number of matches
 */
static std::vector<facebook::vpdq::hashing::vpdqFeature>::size_type
findMatches(const std::vector<facebook::vpdq::hashing::vpdqFeature>& features1,
            const std::vector<facebook::vpdq::hashing::vpdqFeature>& features2, const int distanceTolerance, const bool verbose)
{
    unsigned int matchCnt = 0;
    for (const auto& feature1 : features1) {
        for (const auto& feature2 : features2) {
            if (feature1.pdqHash.hammingDistance(feature2.pdqHash) < distanceTolerance) {
                matchCnt++;
                if (verbose) {
                    std::cout << "Query Hash: " << feature1.pdqHash.format() << " Target Hash: " << feature2.pdqHash.format() << " match "
                              << std::endl;
                }
                break;
            }
        }
    }
    return matchCnt;
}

int matchHash(const std::vector<facebook::vpdq::hashing::vpdqFeature>& qHashes,
              const std::vector<facebook::vpdq::hashing::vpdqFeature>& tHashes, const int distanceTolerance, const int qualityTolerance)
{
    // Filter low quality hashes
    auto queryFiltered  = filterFeatures(qHashes, qualityTolerance, false);
    auto targetFiltered = filterFeatures(tHashes, qualityTolerance, false);

    // Avoid divide-by-zero
    if (queryFiltered.empty() || targetFiltered.empty()) {
        return 0.0;
    }

    // Get count of query in target and target in query
    auto qMatchCnt = findMatches(queryFiltered, targetFiltered, distanceTolerance, false);

    return (qMatchCnt * 100.0) / queryFiltered.size();
}

int matchHashPybind(const py::list& qHashes, const py::list& tHashes, const int distanceTolerance, const int qualityTolerance)
{
    // TODO: Iterate over py::list directly. Don't copy this.
    std::vector<facebook::vpdq::hashing::vpdqFeature> qHashesV;
    qHashesV.reserve(qHashes.size());
    for (const auto& item : qHashes) {
        qHashesV.push_back(item.cast<facebook::vpdq::hashing::vpdqFeature>());
    }

    std::vector<facebook::vpdq::hashing::vpdqFeature> tHashesV;
    tHashesV.reserve(tHashes.size());
    for (const auto& item : tHashes) {
        tHashesV.push_back(item.cast<facebook::vpdq::hashing::vpdqFeature>());
    }

    return matchHash(qHashesV, tHashesV, distanceTolerance, qualityTolerance);
}

} // namespace hvdaccelerators

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
        .def_readonly("pdq_hash", &vpdqFeature::pdqHash)
        .def_readonly("quality", &vpdqFeature::quality)
        .def_readonly("frame_number", &vpdqFeature::frameNumber)
        .def("from_str", &vpdqFeature::from_str)
        .def("to_string", &vpdqFeature::to_string)
        .def("is_valid", &vpdqFeature::is_valid)
        .def("get_hash", &vpdqFeature::get_hash)
        .def("get_frame_number", &vpdqFeature::get_frame_number)
        .def("get_quality", &vpdqFeature::get_quality)
        .def("__str__", &vpdqFeature::to_string)
        .def("__repr__", &vpdqFeature::to_string);

    py::class_<facebook::pdq::hashing::Hash256>(m, "PdqHash256")
        .def(py::init<>())
        .def("fromHexString", &facebook::pdq::hashing::Hash256::fromHexString)
        .def("toHexString", &facebook::pdq::hashing::Hash256::toHexString)
        .def("hammingDistanceLE", &facebook::pdq::hashing::Hash256::hammingDistanceLE)
        .def("__str__", &facebook::pdq::hashing::Hash256::toHexString)
        .def("__repr__", &facebook::pdq::hashing::Hash256::toHexString)
        .def_readonly_static("HASH256_HEX_NUM_NYBBLES", &facebook::pdq::hashing::Hash256::HASH256_HEX_NUM_NYBBLES);

    m.def("matchHash", &hvdaccelerators::matchHashPybind, "TODO");
    m.def("hamming_distance", &hamming_distance, "Calculate the hamming distance between two PDQ hashes.");
    m.def("hash_frame", &hash_frame, "hash a frame");
}
