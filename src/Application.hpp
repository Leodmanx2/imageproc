#ifndef APPLICATION_H
#define APPLICATION_H

#include <argagg/argagg.hpp>

class Application final {
	private:
	argagg::parser_results m_args;
	argagg::parser         m_argParser;

	public:
	Application(int argc, char* argv[]);
	void run();
};

#endif
