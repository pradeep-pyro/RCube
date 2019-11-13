#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

namespace rcube {

class Image {
public:
    Image() = default;

    static Image fromFile(const std::string &filename, int n_channels) {
        int w, h, c;
        int desired_c = n_channels;
        unsigned char *pix = stbi_load(filename.c_str(), &w, &h, &c, desired_c);
        if (pix == nullptr) {
            throw std::runtime_error("Unable to load image. Check file path: " + filename);
        }
        Image im;
        im.width_ = w;
        im.height_ = h;
        im.channels_ = desired_c;
        im.pixels_.assign(pix, pix + w * h * desired_c);
        stbi_image_free(pix);
        return im;
    }

    void saveBMP(const std::string &filename, bool flip_vertically=false) {
        stbi_flip_vertically_on_write(flip_vertically);
        stbi_write_bmp(filename.c_str(), width_, height_, channels_, pixels_.data());
    }

    int width() const {
        return width_;
    }

    int height() const {
        return height_;
    }

    int channels() const {
        return channels_;
    }

    const std::vector<unsigned char> & pixels() const {
        return pixels_;
    }

    std::vector<unsigned char> & pixels() {
        return pixels_;
    }

    void setPixels(int width, int height, int channels, std::vector<unsigned char> &data) {
        pixels_ = data;
        width_ = width;
        height_ = height;
        channels_ = channels;
    }

    void setPixels(int width, int height, int channels, unsigned char *data) {
        pixels_.assign(data, data + width * height * channels);
        width_ = width;
        height_ = height;
        channels_ = channels;
    }
private:
    int width_, height_, channels_;
    std::vector<unsigned char> pixels_;
};

} // namespace rcube

#endif // IMAGE_H
