#include "IMDDT.hpp"
#include <catch.hpp>

TEST_CASE("IMDDT_single returns expected results", "[IMDDT]") {
	SECTION("Pixel #2 excluded") {
		REQUIRE(IMDDT_single(30, 0, 255, 60, 0.3, 0.7) == Approx(192));
	}
	SECTION("Pixel #3 excluded") {
		REQUIRE(IMDDT_single(30, 0, 255, 60, 0.7, 0.3) == Approx(13.5));
	}
	SECTION("Pixel #4 excluded") {
		REQUIRE(IMDDT_single(0, 30, 60, 255, 0.5, 0.3) == Approx(16.5));
	}
	SECTION("Pixel #1 excluded") {
		REQUIRE(IMDDT_single(0, 30, 60, 255, 0.9, 0.3) == Approx(166.5));
	}
}
