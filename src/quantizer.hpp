#pragma once

#include "color.hpp"
#include "image.hpp"


class Quantizer {
public:
    virtual ~Quantizer() {};

    //Quantize image in size ammount of color. store all the color in the array colors of specified size.
    //May contain really close or even duplicated color if the image doesn't have enough.
    virtual void Quantize(std::shared_ptr<Image> img, Lab* colors, uint32_t size) = 0;
};

class MedianCut : public Quantizer {
public:
    void Quantize(std::shared_ptr<Image> img, Lab* colors, uint32_t size) override;

private:
    uint32_t GetChannelMaxRange(float* start, float* end, uint32_t channelCount, float* largestChannelDiff = nullptr);
};

class KMean : public Quantizer {
public:
    void Quantize(std::shared_ptr<Image> img, Lab* colors, uint32_t size) override;
};