// IMDDT is an acronym for "Image Mesh Data-Dependent Triangulation". It is a
// fast, edge-adaptive interpolation method developed by Dan Su and Phillip
// Willis. The method is described in their paper "Image Interpolation by
// Pixel-Level Data-Dependent Triangulation"
// (DOI: 10.1111/j.1467-8659.2004.00752.x)

// The quality isn't very good. Either the implementation is wrong, or the
// samples in the paper are misleading. I'd rather believe the former, but if
// that's the case I'm at a bit of a loss for figuring out the issue.

#include "IMDDT.hpp"

#include <cstdint>
#include <gsl\gsl-lite.hpp>

#ifdef TEST
#include <doctest\doctest.h>
using doctest::Approx;
#endif

using namespace gsl;

constexpr double IMDDT_single(double pixel_1,
                              double pixel_2,
                              double pixel_3,
                              double pixel_4,
                              double distance_x,
                              double distance_y) {
	const double ascendingDifference  = std::abs(pixel_3 - pixel_2);
	const double descendingDifference = std::abs(pixel_1 - pixel_4);

	if(ascendingDifference > descendingDifference) {
		if(distance_y > distance_x) {
			const double weight_1 = 0.5 * (1.0 - distance_y);
			const double weight_4 = 0.5 * distance_x;
			const double weight_3 = 1.0 - weight_1 - weight_4;
			return weight_1 * pixel_1 + weight_4 * pixel_4 + weight_3 * pixel_3;
		}
		const double weight_1 = 0.5 * (1.0 - distance_x);
		const double weight_4 = 0.5 * distance_y;
		const double weight_2 = 1.0 - weight_1 - weight_4;
		return weight_1 * pixel_1 + weight_2 * pixel_2 + weight_4 * pixel_4;
	} else {
		if(distance_x + distance_y < 1.0) {
			const double weight_2 = 0.5 * distance_x;
			const double weight_3 = 0.5 * distance_y;
			const double weight_1 = 1.0 - weight_2 - weight_3;
			return weight_1 * pixel_1 + weight_2 * pixel_2 + weight_3 * pixel_3;
		}
		const double weight_2 = 0.5 * (1.0 - distance_y);
		const double weight_3 = 0.5 * (1.0 - distance_x);
		const double weight_4 = 1.0 - weight_2 - weight_3;
		return weight_2 * pixel_2 + weight_4 * pixel_4 + weight_3 * pixel_3;
	}
}

Image IMDDT(const Image& src, const Dimensions& targetDim) {
	Image dst(targetDim, src.channels());

	const double scale_x =
	  static_cast<double>(src.dimensions().width) / targetDim.width;
	const double scale_y =
	  static_cast<double>(src.dimensions().height) / targetDim.height;

	for(index pos_dst_x = 0; pos_dst_x < targetDim.width; ++pos_dst_x) {
		const index pos_src_x  = scale_x * pos_dst_x;
		const index distance_x = scale_x * pos_dst_x - pos_src_x;
		for(index pos_dst_y = 0; pos_dst_y < targetDim.height; ++pos_dst_y) {
			const index  pos_src_y  = scale_y * pos_dst_y;
			const double distance_y = scale_y * pos_dst_y - pos_src_y;

			for(index channel = 0; channel < src.channels(); ++channel) {
				const index x_incremented = pos_src_x + 1 >= src.dimensions().width ?
                                      src.dimensions().width - 1 :
                                      pos_src_x + 1;
				const index y_incremented = pos_src_y + 1 >= src.dimensions().height ?
                                      src.dimensions().height - 1 :
                                      pos_src_y + 1;

				const uint8_t pixel_1 = src.at(pos_src_x, pos_src_y, channel);
				const uint8_t pixel_2 = src.at(x_incremented, pos_src_y, channel);
				const uint8_t pixel_3 = src.at(pos_src_x, y_incremented, channel);
				const uint8_t pixel_4 = src.at(x_incremented, y_incremented, channel);

				const uint8_t interpolated = IMDDT_single(
				  pixel_1, pixel_2, pixel_3, pixel_4, distance_x, distance_y);
				dst.set(pos_dst_x, pos_dst_y, interpolated, channel);
			}
		}
	}
	return dst;
}

#ifdef TEST

TEST_CASE("IMDDT_single returns expected results") {
	SUBCASE("Pixel #2 excluded") {
		CHECK(IMDDT_single(30, 0, 255, 60, 0.3, 0.7) == Approx(192));
	}
	SUBCASE("Pixel #3 excluded") {
		CHECK(IMDDT_single(30, 0, 255, 60, 0.7, 0.3) == Approx(13.5));
	}
	SUBCASE("Pixel #4 excluded") {
		CHECK(IMDDT_single(0, 30, 60, 255, 0.5, 0.3) == Approx(16.5));
	}
	SUBCASE("Pixel #1 excluded") {
		CHECK(IMDDT_single(0, 30, 60, 255, 0.9, 0.3) == Approx(166.5));
	}
}

#endif
