#ifndef APPLICATION_H
#define APPLICATION_H

#include "IMDDT.hpp"
#include "Image.hpp"
#include "bilinear.hpp"
#include <OpenImageIO/imageio.h>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace po = boost::program_options;

class Application final {
	private:
	po::variables_map       m_varMap;
	po::options_description m_optionsDesc;

	public:
	Application(int argc, char* argv[]);
	void run();
};

#endif
