#include "quantizer.hpp"
#include <algorithm>
#include <vector>
#include <iostream>

void MedianCut::Quantize(std::shared_ptr<Image> img, Lab* colors, uint32_t size) {
    size_t d = 8;
    std::vector<Lab> data( (size_t)(img->GetWidth() * img->GetHeight()) / d );
    for (size_t i = 0; i < data.size(); ++i)
        data[i] = img->GetPixelLab(i * d);

    struct BucketRange {
        Lab* start;
        Lab* end;
    };

    uint32_t bucketsCount = 1;
    std::vector<BucketRange> bucketsRange( (size_t)size );
    bucketsRange[0].start = &data[0];
    bucketsRange[0].end = &data[0] + data.size();

    while (bucketsCount < size) {
        uint32_t bucketIndex = 0;
        float bucketLargestRangeChannelDiff = -1000.0f;
        uint32_t bucketLargestRangeChannelIndex = 0;

        for (uint32_t i = 0; i < bucketsCount; ++i) {
            if (bucketsRange[i].end - bucketsRange[i].start < 16)
                continue;
            float largestRangeChannelDiff;
            uint32_t currentLargestRangeChannelIndex = GetChannelMaxRange((float*)bucketsRange[i].start, (float*)bucketsRange[i].end, 3, &largestRangeChannelDiff);
            if (largestRangeChannelDiff > bucketLargestRangeChannelDiff) {
                bucketLargestRangeChannelDiff = largestRangeChannelDiff;
                bucketIndex = i;
                bucketLargestRangeChannelIndex = currentLargestRangeChannelIndex;
            }
        }

        Lab* start = bucketsRange[bucketIndex].start;
        Lab* end = bucketsRange[bucketIndex].end;
        Lab* mid = start + (end - start) / 2;

        std::sort(start, end, [&](const Lab& a, const Lab& b) {
            return ((float*)&a.L)[bucketLargestRangeChannelIndex] < ((float*)&b.L)[bucketLargestRangeChannelIndex];
        });

        float midValue = ((float*)mid)[bucketLargestRangeChannelIndex];
        Lab* newBucketStart = start;
        for (; ((float*)newBucketStart)[bucketLargestRangeChannelIndex] < midValue; ++newBucketStart) {}

        bucketsRange[bucketIndex].start = start;
        bucketsRange[bucketIndex].end = mid;
        bucketsRange[bucketsCount].start = mid + 1;
        bucketsRange[bucketsCount].end = end;
        ++bucketsCount;
    }

    for (uint32_t i = 0; i < size; ++i) {
        Lab* start = bucketsRange[i].start;
        Lab* end = bucketsRange[i].end;
        Lab c{ 0.0f, 0.0f, 0.0f };
        for (Lab* it = start; it < end; ++it) {
            c.L += it->L;
            c.a += it->a;
            c.b += it->b;
        }
        float bucketSize = (float)(end - start);
        colors[i] = Lab{ c.L / (float)bucketSize, c.a / (float)bucketSize, c.b / (float)bucketSize };
    }

    std::sort(colors, colors + size, [&](const Lab& a, const Lab& b) {
        return a.L < b.L;
    });
}

uint32_t MedianCut::GetChannelMaxRange(float* start, float* end, uint32_t channelCount, float* largestChannelDiff) {
    struct MinMax {
        float min = 1000.0f;
        float max = -1000.0f;
    };

    std::vector<MinMax> channelsMinMax( (size_t)channelCount );

    for (float* it = start; it < end; it += channelCount) {
        for (uint32_t j = 0; j < channelCount; ++j) {
            float currentChannelValue = it[j];
            if (currentChannelValue < channelsMinMax[j].min)
                channelsMinMax[j].min = currentChannelValue;
            if (currentChannelValue > channelsMinMax[j].max)
                channelsMinMax[j].max = currentChannelValue;
        }
    }

    uint32_t largestRangeChannelIndex = 0;
    float largestRangeChannelDiff = 0.0f;

    for (uint32_t i = 0; i < channelCount; ++i) {
        float currentChannelDiff = channelsMinMax[i].max - channelsMinMax[i].min;
        if (currentChannelDiff > largestRangeChannelDiff) {
            largestRangeChannelIndex = i;
            largestRangeChannelDiff = currentChannelDiff;
        }
    }
    if (largestChannelDiff)
        *largestChannelDiff = largestRangeChannelDiff;
    return largestRangeChannelIndex;
}

void KMean::Quantize(std::shared_ptr<Image> img, Lab* colors, uint32_t size) {
    struct Point {
        Lab position = { 0.0f, 0.0f, 0.0f };
        int cluster = -1;
        float minDist = 1000.0;

        float GetDistance(Point p) {
            return (p.position.L - position.L)* (p.position.L - position.L) + 
                (p.position.a - position.a) * (p.position.a - position.a) + 
                (p.position.b - position.b) * (p.position.b - position.b);
        }
    };
    struct Cluster {
        Point centroid = {};
        int pointsCount = 0;
        Lab sumPosition = { 0.0f, 0.0f, 0.0f };
    };

    size_t epochs = 10;
    size_t d = 8;
    std::vector<Point> points( (size_t)(img->GetWidth() * img->GetHeight()) / d );
    for (size_t i = 0; i < points.size(); ++i)
        points[i].position = img->GetPixelLab(i * d);
    
    std::vector<Cluster> clusters(size);
    srand(time(0));

    for (uint32_t i = 0; i < clusters.size(); ++i) {
        clusters[i].centroid.position = points[rand() % points.size()].position;
    }

    for (uint32_t e = 0; e < epochs; ++e) {
        for (uint32_t i = 0; i < points.size(); ++i) {
            for (uint32_t j = 0; j < clusters.size(); ++j) {
                float dist = points[i].GetDistance(clusters[j].centroid);
                if (points[i].minDist > dist) {
                    points[i].minDist = dist;
                    points[i].cluster = j;
                }
            }
            ++clusters[points[i].cluster].pointsCount;
            clusters[points[i].cluster].sumPosition.L += points[i].position.L;
            clusters[points[i].cluster].sumPosition.a += points[i].position.a;
            clusters[points[i].cluster].sumPosition.b += points[i].position.b;
            points[i].minDist = 1000.0f;
        }

        for (uint32_t i = 0; i < clusters.size(); ++i) {
            if (clusters[i].pointsCount <= 0) {
                clusters[i].centroid.position = points[rand() % points.size()].position;
                continue;
            }
            clusters[i].centroid.position.L = clusters[i].sumPosition.L / clusters[i].pointsCount;
            clusters[i].centroid.position.a = clusters[i].sumPosition.a / clusters[i].pointsCount;
            clusters[i].centroid.position.b = clusters[i].sumPosition.b / clusters[i].pointsCount;
            clusters[i].pointsCount = 0;
            clusters[i].sumPosition = { 0.0f, 0.0f, 0.0f };
        }
    }

    for (uint32_t i = 0; i < clusters.size(); ++i) {
        colors[i] = clusters[i].centroid.position;
    }

    std::sort(colors, colors + size, [&](const Lab& a, const Lab& b) {
        return a.L < b.L;
    });
}
