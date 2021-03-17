// AIS is an acronym for "Adaptive Interpolation Scheme". It is an
// edge-adaptive scale doubling method developed by Suheir M. Elbayoumi Harb,
// Nor Ashidi Mat Isa, and Samy Salamah. The method is described in their
// paper "New adaptive interpolation scheme for image upscaling"
// (DOI: 10.1007/s11042-015-2647-9)

#include "AIS_cubic.hpp"
#include "Image.hpp"
#include "gsl-lite.hpp"

using namespace gsl;

int G1(const Image& dst, int x, int y, int channel) {
	Expects(x >= 3);
	Expects(x + 3 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 3);
	Expects(y + 3 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx + 2, y + dy - 2, channel));
	};

	int accumulator = 0;

	// Column 1
	accumulator += delta(-3, -1);
	accumulator += delta(-3, +1);

	// Column 2
	accumulator += delta(-1, -1);
	accumulator += delta(-1, +1);
	accumulator += delta(-1, +3);

	// Column 3
	accumulator += delta(+1, +1);
	accumulator += delta(+1, +3);

	return accumulator;
}

int G2(const Image& dst, int x, int y, int channel) {
	Expects(x >= 3);
	Expects(x + 3 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 3);
	Expects(y + 3 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx + 2, y + dy + 2, channel));
	};

	int accumulator = 0;

	// Column 1
	accumulator += delta(-3, -1);
	accumulator += delta(-3, +1);

	// Column 2
	accumulator += delta(-1, -3);
	accumulator += delta(-1, -1);
	accumulator += delta(-1, +1);

	// Column 3
	accumulator += delta(+1, -3);
	accumulator += delta(+1, -1);

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
	// TODO: Inline into AIS_cubic()
	// TODO: Create mapping for stage 2
	// Formulate the gradient calculation as a loop and break it out into its
	// own function to limit the nesting. Loops are trivially parallelizable

	int gradient_1 = G1(src, x, y, channel);
	int gradient_2 = G2(src, x, y, channel);

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
		int gradient__left_up    = LU(src, y, x, channel);
		int gradient__right_up   = RU(src, y, x, channel);
		int gradient__left_down  = LD(src, y, x, channel);
		int gradient__right_down = RD(src, y, x, channel);

		double       weight_1   = 0.25 / (gradient__left_up + 1);
		double       weight_2   = 0.25 / (gradient__right_up + 1);
		double       weight_3   = 0.25 / (gradient__left_down + 1);
		double       weight_4   = 0.25 / (gradient__right_down + 1);
		const double weight_sum = weight_1 + weight_2 + weight_3 + weight_4;
		return (weight_1 / weight_sum) * src.at(x - 1, y - 1, channel) +
		       (weight_2 / weight_sum) * src.at(x + 1, y - 1, channel) +
		       (weight_3 / weight_sum) * src.at(x - 1, y + 1, channel) +
		       (weight_4 / weight_sum) * src.at(x + 1, y + 1, channel);
	}
}

Image AIS_cubic(const Image& src) {
	// Create a new image that's twice the size of the original
	// minus the last column and row
	const unsigned int width  = src.dimensions().width * 2 - 1;
	const unsigned int height = src.dimensions().height * 2 - 1;
	Image              dst({width, height}, src.channels());

	std::clog << "copying pixels\n";
	// Copy pixels from the source to the destination
	for(index x = 0; x < width; x += 2) {
		for(index y = 0; y < height; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				dst.set(x, y, src.at(x / 2, y / 2, channel), channel);
			}
		}
	}

	std::clog << "filling interior\n";
	// Fill the interior pixels
	for(index x = 3; x < width - 3; x += 2) {
		for(index y = 3; y < height - 3; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				// Find the value with solve_interior
				uint8_t value = solve_interior(dst, x, y, channel);
				dst.set(x, y, value, channel);
			}
		}
	}

	std::clog << "filling aligned\n";
	// Fill the aligned pixels
	/*In the second stage, the pixels Y 2i,2j+1 and Y 2i+1,2j can be interpolated
	similarly with the interpolation window slanted at 45 diagonal direction.
	Interpolation of these pixels is performed with the aid of the interpolated
	pixels in the first stage.*/

	return dst;
}
