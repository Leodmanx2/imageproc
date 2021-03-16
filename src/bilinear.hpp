#ifndef BILINEAR_H
#define BILINEAR_H

#include "Image.hpp"

constexpr double bilinear_single(double pixel_1,
                                 double pixel_2,
                                 double pixel_3,
                                 double pixel_4,
                                 double distance_x,
                                 double distance_y);

Image bilinear(const Image& src, const Dimensions& targetDim);

#endif
