#include "Application.hpp"

int main(int argc, char* argv[]) {
	try {
		Application app(argc, argv);
		app.run();
	} catch(const std::exception& e) { std::cerr << e.what() << std::endl; }
}

Application::Application(int argc, char** argv)
  : m_varMap(), m_optionsDesc("Options") {
	m_optionsDesc.add_options()("help,h", "produce help message")(
	  "input,i", po::value<std::string>(), "input file")(
	  "output,o", po::value<std::string>(), "output file")(
	  "method,m",
	  po::value<std::string>(),
	  "interpolation method (bilinear, IMDDT)")(
	  "scale,s", po::value<float>(), "scale factor");

	po::positional_options_description inputOption;
	inputOption.add("input", -1);

	po::command_line_parser parser(argc, argv);
	parser.options(m_optionsDesc);
	parser.positional(inputOption);

	po::store(parser.run(), m_varMap);
	po::notify(m_varMap);
}

void Application::run() {
	if(m_varMap.empty() || m_varMap.count("help")) {
		std::cout << m_optionsDesc << "\n";
	} else if(!m_varMap.count("input")) {
		std::cerr << "input file needed\n";
	} else if(!m_varMap.count("method")) {
		std::cerr << "interpolation method needed\n";
	} else {
		const std::string inFile = m_varMap["input"].as<std::string>();
		const Image       src(inFile);
		const std::string outFile = m_varMap.count("output") ?
		                              m_varMap["output"].as<std::string>() :
		                              "_" + inFile;
		const float scale =
		  m_varMap.count("scale") ? m_varMap["scale"].as<float>() : 2.0f;

		if(m_varMap["method"].as<std::string>().compare("IMDDT") == 0) {
			const Image dst =
			  IMDDT(src,
			        {static_cast<unsigned int>(src.dimensions().width * scale),
			         static_cast<unsigned int>(src.dimensions().height * scale)});
			dst.save(outFile);
		} else if(m_varMap["method"].as<std::string>().compare("bilinear") == 0) {
			const Image dst =
			  bilinear(src,
			           {static_cast<unsigned int>(src.dimensions().width * scale),
			            static_cast<unsigned int>(src.dimensions().height * scale)});
			dst.save(outFile);
		}
	}
}
