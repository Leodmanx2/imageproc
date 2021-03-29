// AIS is an acronym for "Adaptive Interpolation Scheme". It is an
// edge-adaptive scale doubling method developed by Suheir M. Elbayoumi Harb,
// Nor Ashidi Mat Isa, and Samy Salamah. The method is described in their
// paper "New adaptive interpolation scheme for image upscaling"
// (DOI: 10.1007/s11042-015-2647-9)

#include <cstdint>

#include "Image.hpp"

int G1_stage1(const Image& src, int i, int j, int channel);
int G2_stage1(const Image& src, int i, int j, int channel);
int G1_stage2(const Image& src, int i, int j, int channel);
int G2_stage2(const Image& src, int i, int j, int channel);

int RU(const Image& src, int i, int j, int channel);
int RD(const Image& src, int i, int j, int channel);
int LU(const Image& src, int i, int j, int channel);
int LD(const Image& src, int i, int j, int channel);

int R(const Image& src, int i, int j, int channel);
int D(const Image& src, int i, int j, int channel);
int L(const Image& src, int i, int j, int channel);
int U(const Image& src, int i, int j, int channel);

uint8_t solve_interior_stage1(const Image& src, int x, int y, int channel);
uint8_t solve_interior_stage2(const Image& src, int x, int y, int channel);

Image AIS_cubic(const Image& src);
