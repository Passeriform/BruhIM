#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <iostream>
#include <libconfig.h++>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <process.h>

#include "bruhEngine.h"
#include "Utility.h"

namespace logging = boost::log;

void init_logging(log::trivial::severity_level logLevel) {
	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logLevel
	);
}

using namespace std;

int main() {
	libconfig::Config config;
	bruhEngine::init(&config);

	init_logging(lookupLoggingSeverity(&config, log::trivial::info));

	BOOST_LOG_TRIVIAL(info) << "BruhIM initialized";
	BOOST_LOG_TRIVIAL(debug) << "PID: " << _getpid();

	if (lookupConfig<bool>("daemonize", &config, false)) {
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