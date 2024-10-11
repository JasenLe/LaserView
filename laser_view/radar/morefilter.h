#ifndef MOREFILTER_H
#define MOREFILTER_H
#include <stdint.h>
#include <vector>
#include <math.h>
#include <set>
#include <algorithm>
#include <iostream>
#include "FullScanFilter.h"

class Tofbf {
private:
    const int kConfidenceLow = 15;  // Low confidence threshold
    const int kConfidenceSingle = 220;  // Discrete points require higher confidence
    const int kScanFrequency = 4500;  // Default scan frequency, to change, read
        // according to radar protocol
    const uint16_t kdisMin{0};
    const uint16_t kdisMax{300};

    double curr_speed_;
    Tofbf() = delete;
    Tofbf(const Tofbf &) = delete;
    Tofbf &operator=(const Tofbf &) = delete;

public:
    Tofbf(int speed);
    std::vector<PointData> NearFilter(const std::vector<PointData> &tmp) const;
    std::vector<PointData> NoiseFilter(const std::vector<PointData> &tmp) const;
    ~Tofbf();

    std::vector<PointData> ShadowsFilter(const std::vector<PointData> &scan_in) const;
    std::vector<PointData> MedianFilter(const std::vector<PointData> &scan_in) const;
    std::vector<PointData> WanderFilter(const std::vector<PointData> &scan_in) const;
    std::vector<PointData> TineFilter(const std::vector<PointData> &scan_in) const;
};

#endif // MOREFILTER_H
