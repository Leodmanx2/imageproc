#include "Application.hpp"

#include "AIS_cubic.hpp"
#include "IMDDT.hpp"
#include "Image.hpp"
#include "bilinear.hpp"
#include <OpenImageIO/imageio.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest\doctest.h>

int main(int argc, char* argv[]) {
	try {
		doctest::Context context;
		context.applyCommandLine(argc, argv);
		int result = context.run();
		if(context.shouldExit()) { return result; }

		Application app(argc, argv);
		app.run();
		return result;
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

Application::Application(int argc, char** argv)
  : m_argParser{{{"help", {"-h", "--help"}, "produce help message", 0},
                 {"input", {"-i", "--input"}, "input file", 1},
                 {"output", {"-o", "--output"}, "output file", 1},
                 {"method",
                  {"-m", "--method"},
                  "interpolation method (bilinear, IMDDT, AIS)",
                  1},
                 {"scale", {"-s", "--scale"}, "scale factor", 1}}} {
	m_args = m_argParser.parse(argc, argv);
}

void Application::run() {
	if(m_args["help"]) {
		argagg::fmt_ostream fmt(std::cout);
		fmt << m_argParser;
		return;
	}

	// Determine input file
	std::string inFile;
	if(m_args["input"]) {
		inFile = m_args["input"].as<std::string>();
	} else if(m_args.pos.size() >= 1) {
		inFile = m_args.as<std::string>(0);
	} else {
		std::cerr << "input file needed\nUse -h for help.\n";
		return;
	}

	// Determine interpolation method
	if(!m_args["method"]) {
		std::cerr << "interpolation method needed\nUse -h for help.\n";
		return;
	}
	const std::string method = m_args["method"].as<std::string>();

	// Determine scale
	const float scale = m_args["scale"].as<float>(2.0f);

	// Determine output file
	std::stringstream ss;
	ss << m_args["method"].as<std::string>() << "-" << scale << "x_" << inFile;
	const std::string outFile = m_args["output"].as<std::string>(ss.str());

	// Process image
	const Image src(inFile);
	if(method.compare("IMDDT") == 0) {
		const Image dst =
		  IMDDT(src,
		        {static_cast<unsigned int>(src.dimensions().width * scale),
		         static_cast<unsigned int>(src.dimensions().height * scale)});
		dst.save(outFile);
	} else if(method.compare("bilinear") == 0) {
		const Image dst =
		  bilinear(src,
		           {static_cast<unsigned int>(src.dimensions().width * scale),
		            static_cast<unsigned int>(src.dimensions().height * scale)});
		dst.save(outFile);
	} else if(method.compare("AIS") == 0) {
		const Image dst = AIS_cubic(src);
		dst.save(outFile);
	} else {
		std::cerr << "method unrecognized\n";
	}
}
