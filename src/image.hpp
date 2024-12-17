#pragma once

#include <memory>
#include "color.hpp"


class Image {
public:
    Image() = delete;
    Image(const char* filename);
    ~Image();

    inline int GetWidth() const { return width; }
    inline int GetHeight() const { return height; }
    inline int GetChannels() const { return channels; }
    inline unsigned char* GetData() { return data; }

    inline RGB GetPixelRGB(uint32_t idx) const {
        return RGB{
            ((float)data[idx * 3] / 255.0f),
            ((float)data[idx * 3 + 1] / 255.0f),
            ((float)data[idx * 3 + 2] / 255.0f)
        };
    }

    inline Lab GetPixelLab(uint32_t idx) const {
        return ColorTo<Lab>(GetPixelRGB(idx));
    }

    inline LCh GetPixelLCh(uint32_t idx) const {
        return ColorTo<LCh>(GetPixelLab(idx));
    }

    static std::shared_ptr<Image> Open(const char* filename);

private:
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = nullptr;
};