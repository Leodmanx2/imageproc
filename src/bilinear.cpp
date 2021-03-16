#include "bilinear.hpp"
#include "gsl-lite.hpp"
#include <cstdint>

using namespace gsl;

constexpr double bilinear_single(double pixel_1,
                                 double pixel_2,
                                 double pixel_3,
                                 double pixel_4,
                                 double distance_x,
                                 double distance_y) {
	const double weight_1 = (1 - distance_x) * (1 - distance_y);
	const double weight_2 = distance_x * (1 - distance_y);
	const double weight_3 = (1 - distance_x) * distance_y;
	const double weight_4 = distance_x * distance_y;

	return weight_1 * pixel_1 + weight_2 * pixel_2 + weight_3 * pixel_3 +
	       weight_4 * pixel_4;
}

Image bilinear(const Image& src, const Dimensions& targetDim) {
	Image dst(targetDim, src.channels());

	const double scale_x =
	  static_cast<double>(src.dimensions().width) / targetDim.width;
	const double scale_y =
	  static_cast<double>(src.dimensions().height) / targetDim.height;

	for(index pos_dst_x = 0; pos_dst_x < targetDim.width; ++pos_dst_x) {
		const index  pos_src_x  = scale_x * pos_dst_x;
		const double distance_x = scale_x * pos_dst_x - pos_src_x;
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

				const uint8_t interpolated = bilinear_single(
				  pixel_1, pixel_2, pixel_3, pixel_4, distance_x, distance_y);
				dst.set(pos_dst_x, pos_dst_y, interpolated, channel);
			}
		}
	}
	return dst;
}
