//
// Created by 19210 on 2024/9/27.
//

#ifndef PIECEWISEGEOMETRICMODELINDEX_CLOSURE_ALGORITHM_H
#define PIECEWISEGEOMETRICMODELINDEX_CLOSURE_ALGORITHM_H
#include "piecewise_linear_model.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>
#include <limits>

namespace dili {

    template<typename K, size_t Epsilon = 64, typename Floating = double >
    class ClosureAlgorithm {
    private:
        static_assert(Epsilon > 0);
        /// Sentinel value to avoid bounds checking.
        static constexpr K sentinel = std::numeric_limits<K>::has_infinity ? std::numeric_limits<K>::infinity()
                                                                           : std::numeric_limits<K>::max();

        struct Segment {
            K key;              ///< The first key that the segment indexes.
            Floating slope;     ///< The slope of the segment.
            uint32_t intercept; ///< The intercept of the segment.
            Segment() = default;
            Segment(K key_, Floating slope_, uint32_t intercept_): key(key_), slope(slope_), intercept(intercept_) {}
            explicit Segment(const typename OptimalPiecewiseLinearModel<K, size_t>::CanonicalSegment &cs)
                    : key(cs.get_first_x()) {
                auto[cs_slope, cs_intercept] = cs.get_floating_point_segment(key);
                if (cs_intercept > std::numeric_limits<decltype(intercept)>::max())
                    throw std::overflow_error("Change the type of Segment::intercept to uint64");
                if (cs_intercept < 0)
                    throw std::overflow_error("Unexpected intercept < 0");
                slope = cs_slope;
                intercept = cs_intercept;
            }
            operator K() { return key;}
            size_t operator()(const K &k) const {
                size_t pos;
                if constexpr (std::is_same_v<K, int64_t> || std::is_same_v<K, int32_t>)
                    pos = size_t(slope * double(std::make_unsigned_t<K>(k) - key));
                else
                    pos = size_t(slope * double(k - key));
                return pos + intercept;
            }
        };
        std::vector<Segment> segments;

    public:
        ClosureAlgorithm(const std::vector<K>& data): ClosureAlgorithm(data.begin(), data.end()) {}

        template<typename RandomIt>
        ClosureAlgorithm(RandomIt first, RandomIt last) {
            build(first, last);
        }

        std::vector<Segment>& getSegments() { return segments; }

    private:
        template<typename RandomIt>
        void build(RandomIt first, RandomIt last) {
            auto n = (size_t) std::distance(first, last);
            if (n == 0) {//data数量为0
                return;
            }
            segments.reserve(n / 2 * Epsilon);

            auto in_fun = [&](auto i) { return K(first[i]); };
            auto out_fun = [&](auto cs) { segments.emplace_back(cs); };

            auto n_segments = make_segmentation_par(n, Epsilon, in_fun, out_fun);
            if (segments.back() == sentinel)
                --n_segments;
            else {
                if (segments.back()(sentinel - 1) < n)
                    segments.emplace_back(*std::prev(last) + 1, 0, n); // Ensure keys > last are mapped to last_n
                segments.emplace_back(sentinel, 0, n);
            }
        }
    };
}

#endif //PIECEWISEGEOMETRICMODELINDEX_CLOSURE_ALGORITHM_H
