#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Image::Image(const char* filename) {
    data = stbi_load(filename, &width, &height, &channels, 3);
}

Image::~Image() {
    stbi_image_free(data);
}

std::shared_ptr<Image> Image::Open(const char* filename) {
    return std::make_shared<Image>(filename);
}