// IMDDT is an acronym for "Image Mesh Data-Dependent Triangulation". It is a
// fast, edge-adaptive interpolation method developed by Dan Su and Phillip
// Willis. The method is described in their paper "Image Interpolation by
// Pixel-Level Data-Dependent Triangulation"
// (DOI: 10.1111/j.1467-8659.2004.00752.x)

// The quality isn't very good. Either the implementation is wrong, or the
// samples in the paper are misleading. I'd rather believe the former, but if
// that's the case I'm at a bit of a loss for figuring out the issue.

#ifndef IMDDT_H
#define IMDDT_H

#include "Image.hpp"

constexpr double IMDDT_single(double pixel_1,
                              double pixel_2,
                              double pixel_3,
                              double pixel_4,
                              double distance_x,
                              double distance_y);

Image IMDDT(const Image& src, const Dimensions& targetDim);

#endif
