// AIS is an acronym for "Adaptive Interpolation Scheme". It is an
// edge-adaptive scale doubling method developed by Suheir M. Elbayoumi Harb,
// Nor Ashidi Mat Isa, and Samy Salamah. The method is described in their
// paper "New adaptive interpolation scheme for image upscaling"
// (DOI: 10.1007/s11042-015-2647-9)

#include "Image.hpp"
#include "gsl-lite.hpp"

using namespace gsl;

Image AIS_cubic(const Image& src) {
	// Create a new image that's twice the size of the original
	// minus the last column and row
	const unsigned int width  = src.dimensions().width * 2 - 1;
	const unsigned int height = src.dimensions().height * 2 - 1;
	Image              dst({width, height}, src.channels());

	// Copy pixels from the source to the destination
	for(index x = 0; x < width; x += 2) {
		for(index y = 0; y < height; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				dst.set(x, y, src.at(x, y, channel), channel);
			}
		}
	}

	// Fill the interior pixels
	for(index x = 1; x < width; x += 2) {
		for(index y = 1; y < height; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				// Find the value with solve_interior
				// TODO: Bounds checking
				// TODO: solve_interior needs to be adjusted to take channels into account
			}
		}
	}

	// Fill the aligned pixels
	/*In the second stage, the pixels Y 2i,2j+1 and Y 2i+1,2j can be interpolated
	similarly with the interpolation window slanted at 45 diagonal direction.
	Interpolation of these pixels is performed with the aid of the interpolated
	pixels in the first stage.*/
}

int G1(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int p = -1; p <= 3; p += 2) {
		for(int q = -1; q <= 3; q += 2) {
			accumulator += abs(src.at(j - q, i + p, channel) -
			                   src.at(j - q + 2, i + p - 2, channel));
		}
	}

	return accumulator;
}

int G2(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int p = -1; p <= 3; p += 2) {
		for(int q = -1; q <= 3; q += 2) {
			accumulator += abs(src.at(j + q, i + p, channel) -
			                   src.at(j + q + 2, i + p - 2, channel));
		}
	}

	return accumulator;
}

int RU(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(src.at(j - q, i - 1, channel) - src.at(j - q + 2, i - 3, channel));
	}
	accumulator +=
	  abs(src.at(j + 1, i + 1, channel) - src.at(j + 3, i - 1, channel));
	accumulator +=
	  abs(src.at(j + 1, i + 3, channel) - src.at(j + 3, i + 1, channel));

	return accumulator;
}

int RD(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(src.at(j - q, i + 1, channel) - src.at(j - q + 2, i + 3, channel));
	}
	accumulator +=
	  abs(src.at(j + 1, i - 1, channel) - src.at(j + 3, i + 1, channel));
	accumulator +=
	  abs(src.at(j + 1, i - 3, channel) - src.at(j + 3, i - 1, channel));

	return accumulator;
}

int LU(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(src.at(j - q, i - 3, channel) - src.at(j - q + 2, i - 1, channel));
	}
	accumulator +=
	  abs(src.at(j - 3, i - 1, channel) - src.at(j - 1, i + 1, channel));
	accumulator +=
	  abs(src.at(j - 3, i + 1, channel) - src.at(j - 1, i + 3, channel));

	return accumulator;
}

int LD(const Image& src, int i, int j, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(src.at(j - q, i + 3, channel) - src.at(j - q + 2, i + 1, channel));
	}
	accumulator +=
	  abs(src.at(j - 3, i + 1, channel) - src.at(j - 1, i - 1, channel));
	accumulator +=
	  abs(src.at(j - 3, i - 1, channel) - src.at(j - 1, i - 3, channel));

	return accumulator;
}

uint8_t solve_interior(const Image& src, int x, int y, int channel) {
	// TODO: Modify to use Image.at(), inline into AIS_cubic()\
	// Formulate the gradient calculation as a loop and break it out into its
	// own function to limit the nesting. Loops are trivially parallelizable

	// TODO: Calc gradients (using functions)
	int gradient_1 = G1(src, y, x, channel);
	int gradient_2 = G2(src, y, x, channel);

	const int threshold = 100;
	if(gradient_1 - gradient_2 > threshold) {
		// Edge in the -45deg direction
		return -(1.0 / 16.0) * src.at(x - 3, y - 3, channel) +
		       (9.0 / 16.0) * src.at(x - 1, y - 1, channel) +
		       (9.0 / 16.0) * src.at(x + 1, y + 1, channel) -
		       (1.0 / 16.0) * src.at(x + 3, y + 3, channel);
	} else if(gradient_2 - gradient_1 > threshold) {
		// Edge in the +45deg direction
		return -(1.0 / 16.0) * src.at(x - 3, y + 3, channel) +
		       (9.0 / 16.0) * src.at(x - 1, y + 1, channel) +
		       (9.0 / 16.0) * src.at(x + 1, y - 1, channel) -
		       (1.0 / 16.0) * src.at(x + 3, y - 3, channel);
	} else {
		// Non-strong edge
		int gradient__left_up =
		  d(28, 44) + d(14, 30) + d(0, 16) + d(2, 18) + d(4, 20);
		int gradient__right_up = RU(src, y, x, channel);
		int gradient__left_down =
		  d(14, 2) + d(28, 16) + d(42, 30) + d(44, 32) + d(46, 34);
		int gradient__right_down =
		  d(4, 20) + d(18, 34) + d(32, 48) + d(30, 46) + d(28, 44);

		double       weight_1   = 0.25 / (gradient__left_up + 1);
		double       weight_2   = 0.25 / (gradient__right_up + 1);
		double       weight_3   = 0.25 / (gradient__left_down + 1);
		double       weight_4   = 0.25 / (gradient__right_down + 1);
		const double weight_sum = weight_1 + weight_2 + weight_3 + weight_4;
		return (weight_1 / weight_sum) * block[16] +
		       (weight_2 / weight_sum) * block[18] +
		       (weight_3 / weight_sum) * block[30] +
		       (weight_4 / weight_sum) * block[32];
	}
}
