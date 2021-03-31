// AIS is an acronym for "Adaptive Interpolation Scheme". It is an
// edge-adaptive scale doubling method developed by Suheir M. Elbayoumi Harb,
// Nor Ashidi Mat Isa, and Samy Salamah. The method is described in their
// paper "New adaptive interpolation scheme for image upscaling"
// (DOI: 10.1007/s11042-015-2647-9)

#include "AIS_cubic.hpp"
#include "Image.hpp"
#include <gsl\gsl-lite.hpp>

#include <doctest\doctest.h>

using namespace gsl;

// Stage 1 Strong Edge Detection
// -----------------------------

int G1_stage1(const Image& dst, int x, int y, int channel) {
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

int G1_stage2(const Image& dst, int x, int y, int channel) {
	Expects(x >= 2);
	Expects(x + 2 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 2);
	Expects(y + 2 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx + 2, y + dy, channel));
	};

	int accumulator = 0;

	accumulator += delta(-1, -2);
	accumulator += delta(-2, -1);
	accumulator += delta(+0, -1);
	accumulator += delta(-1, +0);
	accumulator += delta(-2, +1);
	accumulator += delta(+0, +1);
	accumulator += delta(-1, +2);

	return accumulator;
}

// Stage 2 Strong Edge Detection
// -----------------------------

int G2_stage1(const Image& dst, int x, int y, int channel) {
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

int G2_stage2(const Image& dst, int x, int y, int channel) {
	Expects(x >= 2);
	Expects(x + 2 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 2);
	Expects(y + 2 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx, y + dy + 2, channel));
	};

	int accumulator = 0;

	accumulator += delta(-2, -1);
	accumulator += delta(-1, -2);
	accumulator += delta(-1, +0);
	accumulator += delta(+0, -1);
	accumulator += delta(+1, -2);
	accumulator += delta(+1, +0);
	accumulator += delta(+2, -1);

	return accumulator;
}

// Stage 1 Weak Edge Interpolation
// -------------------------------

int RU(const Image& dst, int x, int y, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(dst.at(x - q, y - 1, channel) - dst.at(x - q + 2, y - 3, channel));
	}
	accumulator +=
	  abs(dst.at(x + 1, y + 1, channel) - dst.at(x + 3, y - 1, channel));
	accumulator +=
	  abs(dst.at(x + 1, y + 3, channel) - dst.at(x + 3, y + 1, channel));

	return accumulator;
}

int RD(const Image& dst, int x, int y, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(dst.at(x - q, y + 1, channel) - dst.at(x - q + 2, y + 3, channel));
	}
	accumulator +=
	  abs(dst.at(x + 1, y - 1, channel) - dst.at(x + 3, y + 1, channel));
	accumulator +=
	  abs(dst.at(x + 1, y - 3, channel) - dst.at(x + 3, y - 1, channel));

	return accumulator;
}

int LU(const Image& dst, int x, int y, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(dst.at(x - q, y - 3, channel) - dst.at(x - q + 2, y - 1, channel));
	}
	accumulator +=
	  abs(dst.at(x - 3, y - 1, channel) - dst.at(x - 1, y + 1, channel));
	accumulator +=
	  abs(dst.at(x - 3, y + 1, channel) - dst.at(x - 1, y + 3, channel));

	return accumulator;
}

int LD(const Image& dst, int x, int y, int channel) {
	int accumulator = 0;

	for(int q = -1; q <= 3; q += 2) {
		accumulator +=
		  abs(dst.at(x - q, y + 3, channel) - dst.at(x - q + 2, y + 1, channel));
	}
	accumulator +=
	  abs(dst.at(x - 3, y + 1, channel) - dst.at(x - 1, y - 1, channel));
	accumulator +=
	  abs(dst.at(x - 3, y - 1, channel) - dst.at(x - 1, y - 3, channel));

	return accumulator;
}

// Stage 2 Weak Edge Interpolation
// -------------------------------

int R(const Image& dst, int x, int y, int channel) {
	Expects(x >= 1);
	Expects(x + 3 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 2);
	Expects(y + 2 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx + 2, y + dy, channel));
	};

	int accumulator = 0;

	accumulator += delta(-1, -2);
	accumulator += delta(+0, -1);
	accumulator += delta(+1, +0);
	accumulator += delta(+0, +1);
	accumulator += delta(-1, +2);

	return accumulator;
}

int D(const Image& dst, int x, int y, int channel) {
	Expects(x >= 2);
	Expects(x + 2 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 1);
	Expects(y + 3 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx, y + dy + 2, channel));
	};

	int accumulator = 0;

	accumulator += delta(-2, -1);
	accumulator += delta(-1, +0);
	accumulator += delta(+0, +1);
	accumulator += delta(+1, +0);
	accumulator += delta(+2, -1);

	return accumulator;
}

int L(const Image& dst, int x, int y, int channel) {
	Expects(x >= 3);
	Expects(x + 1 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 2);
	Expects(y + 2 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx + 2, y + dy, channel));
	};

	int accumulator = 0;

	accumulator += delta(-1, -2);
	accumulator += delta(-2, -1);
	accumulator += delta(-3, +0);
	accumulator += delta(-2, +1);
	accumulator += delta(-1, +2);

	return accumulator;
}

int U(const Image& dst, int x, int y, int channel) {
	Expects(x >= 2);
	Expects(x + 2 < static_cast<long long>(dst.dimensions().width));
	Expects(y >= 3);
	Expects(y + 1 < static_cast<long long>(dst.dimensions().height));

	auto delta = [&](int dx, int dy) {
		return abs(dst.at(x + dx, y + dy, channel) -
		           dst.at(x + dx, y + dy + 2, channel));
	};

	int accumulator = 0;

	accumulator += delta(-2, -1);
	accumulator += delta(-1, -2);
	accumulator += delta(+0, -3);
	accumulator += delta(+1, -2);
	accumulator += delta(+2, -1);

	return accumulator;
}

// Solvers
// -------

uint8_t solve_interior_stage1(const Image& dst, int x, int y, int channel) {
	int gradient_1 = G1_stage1(dst, x, y, channel);
	int gradient_2 = G2_stage1(dst, x, y, channel);

	double    result    = 0;
	const int threshold = 100;
	if(gradient_1 - gradient_2 > threshold) {
		// Edge in the -45deg direction
		result = -(1.0 / 16.0) * dst.at(x - 3, y - 3, channel) +
		         (9.0 / 16.0) * dst.at(x - 1, y - 1, channel) +
		         (9.0 / 16.0) * dst.at(x + 1, y + 1, channel) -
		         (1.0 / 16.0) * dst.at(x + 3, y + 3, channel);
	} else if(gradient_2 - gradient_1 > threshold) {
		// Edge in the +45deg direction
		result = -(1.0 / 16.0) * dst.at(x - 3, y + 3, channel) +
		         (9.0 / 16.0) * dst.at(x - 1, y + 1, channel) +
		         (9.0 / 16.0) * dst.at(x + 1, y - 1, channel) -
		         (1.0 / 16.0) * dst.at(x + 3, y - 3, channel);
	} else {
		// Non-strong edge
		int gradient__left_up    = LU(dst, x, y, channel);
		int gradient__right_up   = RU(dst, x, y, channel);
		int gradient__left_down  = LD(dst, x, y, channel);
		int gradient__right_down = RD(dst, x, y, channel);

		double       weight_1   = 0.25 / (gradient__left_up + 1);
		double       weight_2   = 0.25 / (gradient__right_up + 1);
		double       weight_3   = 0.25 / (gradient__left_down + 1);
		double       weight_4   = 0.25 / (gradient__right_down + 1);
		const double weight_sum = weight_1 + weight_2 + weight_3 + weight_4;
		result = (weight_1 / weight_sum) * dst.at(x - 1, y - 1, channel) +
		         (weight_2 / weight_sum) * dst.at(x + 1, y - 1, channel) +
		         (weight_3 / weight_sum) * dst.at(x - 1, y + 1, channel) +
		         (weight_4 / weight_sum) * dst.at(x + 1, y + 1, channel);
	}
	if(result > 255) {
		return 255;
	} else if(result < 0) {
		return 0;
	} else {
		return result;
	}
}

uint8_t solve_interior_stage2(const Image& dst, int x, int y, int channel) {
	int gradient_1 = G1_stage2(dst, x, y, channel);
	int gradient_2 = G2_stage2(dst, x, y, channel);

	double    result    = 0;
	const int threshold = 100;
	if(gradient_1 - gradient_2 > threshold) {
		// Edge in the -45deg direction
		result = -(1.0 / 16.0) * dst.at(x - 3, y, channel) +
		         (9.0 / 16.0) * dst.at(x - 1, y, channel) +
		         (9.0 / 16.0) * dst.at(x + 1, y, channel) -
		         (1.0 / 16.0) * dst.at(x + 3, y, channel);
	} else if(gradient_2 - gradient_1 > threshold) {
		// Edge in the +45deg direction
		result = -(1.0 / 16.0) * dst.at(x, y + 3, channel) +
		         (9.0 / 16.0) * dst.at(x, y + 1, channel) +
		         (9.0 / 16.0) * dst.at(x, y - 1, channel) -
		         (1.0 / 16.0) * dst.at(x, y - 3, channel);
	} else {
		// Non-strong edge
		int gradient_left  = L(dst, x, y, channel);
		int gradient_right = R(dst, x, y, channel);
		int gradient_up    = U(dst, x, y, channel);
		int gradient_down  = D(dst, x, y, channel);

		double       weight_1   = 0.25 / (gradient_left + 1);
		double       weight_2   = 0.25 / (gradient_right + 1);
		double       weight_3   = 0.25 / (gradient_up + 1);
		double       weight_4   = 0.25 / (gradient_down + 1);
		const double weight_sum = weight_1 + weight_2 + weight_3 + weight_4;
		result = (weight_1 / weight_sum) * dst.at(x - 1, y, channel) +
		         (weight_2 / weight_sum) * dst.at(x + 1, y, channel) +
		         (weight_3 / weight_sum) * dst.at(x, y - 1, channel) +
		         (weight_4 / weight_sum) * dst.at(x, y + 1, channel);
	}
	if(result > 255) {
		return 255;
	} else if(result < 0) {
		return 0;
	} else {
		return result;
	}
}

// Public Interfaces
// -----------------

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
				dst.set(x, y, src.at(x / 2, y / 2, channel), channel);
			}
		}
	}

	// Fill the interior pixels
	for(index x = 3; x < width - 3; x += 2) {
		for(index y = 3; y < height - 3; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				// Find the value with solve_interior
				uint8_t value = solve_interior_stage1(dst, x, y, channel);
				dst.set(x, y, value, channel);
			}
		}
	}

	// Fill the aligned pixels
	// NOTE: The paper only says to flip the interpolation window 45deg
	//       for stage 2 and is otherwise completely ambiguous. I made
	//       a guess as to how this should work, but it might not be what
	//       the authors intended.
	for(index x = 4; x < width - 3; x += 2) {
		for(index y = 3; y < height - 3; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				uint8_t value = solve_interior_stage2(dst, x, y, channel);
				dst.set(x, y, value, channel);
			}
		}
	}
	for(index x = 3; x < width - 3; x += 2) {
		for(index y = 4; y < height - 3; y += 2) {
			for(index channel = 0; channel < dst.channels(); ++channel) {
				uint8_t value = solve_interior_stage2(dst, x, y, channel);
				dst.set(x, y, value, channel);
			}
		}
	}

	// TODO: Edge completion

	return dst;
}

// Unit Tests
// ----------

TEST_CASE("Gradients are computed correctly") {
	Image src({7, 7}, 1);
	for(index x = 0; x < 7; ++x) {
		for(index y = 0; y < 7; ++y) { src.set(x, y, x * 2 + y * 10, 0); }
	}

	SUBCASE("G1 (Stage 1)") { CHECK(G1_stage1(src, 3, 3, 0) == 112); }
	SUBCASE("G2 (Stage 1)") { CHECK(G2_stage1(src, 3, 3, 0) == 168); }
	SUBCASE("G1 (Stage 2)") { CHECK(G1_stage2(src, 3, 3, 0) == 28); }
	SUBCASE("G2 (Stage 2)") { CHECK(G2_stage2(src, 3, 3, 0) == 140); }
	SUBCASE("R") { CHECK(R(src, 3, 3, 0) == 20); }
	SUBCASE("L") { CHECK(L(src, 3, 3, 0) == 20); }
	SUBCASE("U") { CHECK(U(src, 3, 3, 0) == 100); }
	SUBCASE("D") { CHECK(D(src, 3, 3, 0) == 100); }
}
