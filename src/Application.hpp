#ifndef APPLICATION_H
#define APPLICATION_H

#include "IMDDT.hpp"
#include "Image.hpp"
#include "bilinear.hpp"
#include <OpenImageIO/imageio.h>
#include <argagg/argagg.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <utility>

namespace po = boost::program_options;

class Application final {
	private:
	argagg::parser_results m_args;
	argagg::parser         m_argParser;

	public:
	Application(int argc, char* argv[]);
	void run();
};

#endif
