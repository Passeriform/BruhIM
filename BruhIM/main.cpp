#include <iostream>
#include <libconfig.h++>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "bruhEngine.h"
// #include "bruhServer"
// #include "bruhClient"


namespace logging = boost::log;

void init_logging()
{
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);
}

using namespace std;

int main() {
	init_logging();

	libconfig::Config config;
	bruhEngine::init(&config);

	BOOST_LOG_TRIVIAL(info) << "BruhIM initialized";

	if (bruhEngine::lookupConfig<bool>("daemonize", &config, false)) {
		BOOST_LOG_TRIVIAL(info) << "Running daemon version. You can continue your work!";
		bruhEngine::runDaemon(&config);
		// BOOST_LOG_TRIVIAL(debug) << "Running at process id: " << bruhEngine::getDaemonPid();
	}
	else {
		BOOST_LOG_TRIVIAL(info) << "Running live version. Press ctrl^+R to attach to daemon. Press ctrl^+C to kill.";
		bruhEngine::runServer(&config);

		bruhEngine::streamInput();
	}

}