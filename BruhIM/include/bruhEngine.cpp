#include <iostream>
#include <type_traits>
#include <sstream>
#include <iterator>
#include <regex>
#include <libconfig.h++>
#include <boost/algorithm/string.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include "bruhEngine.h"
#include "IMAgent/IMServer.h"
#include "IMThreads/IMObjectPool.h"
#include "IMException.h"
#include "Utility.h"

namespace po = boost::program_options;
namespace logging = boost::log;

using namespace libconfig;

// Extern global definitions.
IMServer* bruhEngine::server;
IMClient* bruhEngine::client;
std::vector<struct sockaddr_in> bruhEngine::pendingConnAck;

// TODO: Move Cli and PO into main/config package instead of engine.

enum class CliCommandEnum {
	HELP,
	CONNECT,
	SEND,
	ROOM,
	LEAVE,
	STATUS,
	EXIT,
};

std::map<std::string, CliCommandEnum> CliCommands = {
	{"!connect", CliCommandEnum::CONNECT},
	{"!send", CliCommandEnum::SEND},
	{"!room", CliCommandEnum::ROOM},
	{"!leave", CliCommandEnum::LEAVE},
	{"!help", CliCommandEnum::HELP},
	{"!stat", CliCommandEnum::STATUS},
	{"!exit", CliCommandEnum::EXIT},
};

std::vector<std::string> tokenize(const std::string& input) {
	typedef boost::escaped_list_separator<char> separator_type;
	separator_type separator(
		"\\",			// The escape characters.
		"= ",			// The separator characters.
		"\"\'"			// The quote characters.
	);

	// Tokenize the input.
	boost::tokenizer<separator_type> tokens(input, separator);

	// Copy non-empty tokens from the tokenizer into the result.
	std::vector<std::string> result;
	copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
		!boost::bind(&std::string::empty, _1));
	return result;
}

void printHelp(std::string usageString) {
	std::cout << std::endl << "Usage: " << usageString << std::endl;
	std::cout << "Available commands:" << std::endl;

	for (auto availIter = CliCommands.begin(); availIter != CliCommands.end(); ++availIter) {
		std::cout << std::setw(Constants::TAB) << availIter->first << std::endl;
	}
}

void printSubcommandHelp(std::string usageString, po::positional_options_description description) {
	std::cout << std::endl << "Usage: " << usageString << std::endl;
	std::cout << "Available commands:" << std::endl;

	for (decltype(description.max_total_count()) idx = 0; idx < description.max_total_count(); ++idx) {
		const std::string& optionString = description.name_for_position(idx);

		std::cout << std::setw(Constants::TAB) << optionString << std::endl;
	}
}

int bruhEngine::init(libconfig::Config* config) {
	try {
		config->readFile("config/engine.cfg");
	}
	catch (const FileIOException& fioex) {
		BOOST_LOG_TRIVIAL(warning) << "I/O error while reading file." << fioex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (const ParseException& pex) {
		BOOST_LOG_TRIVIAL(warning) << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int bruhEngine::runServer(libconfig::Config* config) {
	int port = lookupConfig<int>("port", config, 3000);

	try {
		server = new IMServer();

		BOOST_LOG_TRIVIAL(debug) << "Preparing server configuration." << std::endl;
		server->prepare(&port);

		BOOST_LOG_TRIVIAL(info) << "Starting server at port " << port;
		server->run();
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(fatal) << "Server error captured." << e.what();
	}

	try {
		BOOST_LOG_TRIVIAL(debug) << "Intializing a client.";
		client = new IMClient();
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(fatal) << "Client error captured." << e.what();
	}

	// TODO: return threadObjectId.
	return EXIT_SUCCESS;
}

// TODO: Add daemonizing mechanism
int bruhEngine::attachDaemon(int threadObjectId) {
	try {
		BOOST_LOG_TRIVIAL(debug) << "Attaching client to daemonized server at " << threadObjectId;
		// (getThread(threadObjectId)->window).pipe(currentWindow);
	}
	catch (std::exception&) {
		std::throw_with_nested(DaemonAttachFailedException("Process couldn't be daemonized due to an internal error.", __FILE__, __LINE__, __func__));
	}

	return EXIT_SUCCESS;
}

int bruhEngine::runDaemon(libconfig::Config* config) {
	BOOST_LOG_TRIVIAL(debug) << "Starting server instance.";
	int threadObjectId = runServer(config);

	BOOST_LOG_TRIVIAL(debug) << "Started server at thread with id: " << threadObjectId;

	try {
		// TODO: Save threadObjectId on a temporary state fd.
		BOOST_LOG_TRIVIAL(info) << "Daemonizing server.";
		// daemonize(threadObjectId)

		// server->setupDaemonListeners(threadObjectId);
		BOOST_LOG_TRIVIAL(info) << "Started server daemon. Send further commands by calling this executable and passing the -p flag";
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << "Daemon error captured." << e.what();
	}

	return EXIT_SUCCESS;
}

int bruhEngine::streamInput() noexcept {
	// TODO: Simplify and create static utility methods.
	while (true) {
		std::string input;
		std::getline(std::cin, input);

		po::options_description global("Global options");
		global.add_options()
			("help", "Show this help")
			("command", po::value<std::string>(), "Select a subcommand")
			("subargs", po::value<std::vector<std::string>>(), "Arguments for command");

		po::positional_options_description pos;
		pos.add("command", 1)
			.add("subargs", -1);

		po::variables_map varMap;

		po::parsed_options parsedOpts = po::command_line_parser(tokenize(input))
			.options(global)
			.positional(pos)
			.allow_unregistered()
			.run();

		po::store(parsedOpts, varMap);

		po::notify(varMap);

		std::string commandString;
		CliCommandEnum command;

		try {
			commandString = varMap["command"].as<std::string>();
			command = CliCommands[commandString];
		}
		catch (std::exception e) {
			BOOST_LOG_TRIVIAL(error) << "Parsing command threw exception:" << std::endl << e.what();
			BOOST_LOG_TRIVIAL(debug) << "Defaulting to !help";
			commandString = "!help";
			command = CliCommandEnum::HELP;
		}

		// TODO: Add help option for subcommands as well.

		switch (command) {
		case CliCommandEnum::HELP:
			printHelp("!<subcommand> [subcommandOptions] ...");
			break;

		case CliCommandEnum::CONNECT:
		{
			po::options_description subDesc("Connect options");
			subDesc.add_options()
				("help", "Show connect help")
				("address", po::value<std::string>(), "Server address to connect to")
				("port", po::value<int>(), "Port to run client at");

			std::vector<std::string> subOpts = po::collect_unrecognized(parsedOpts.options, po::include_positional);
			subOpts.erase(subOpts.begin());

			po::positional_options_description pos;
			pos.add("address", 1)
				.add("port", 1);

			// Reparse leftover args...
			po::store(po::command_line_parser(subOpts).options(subDesc).positional(pos).run(), varMap);

			std::string address;
			int port;

			try {
				address = varMap["address"].as<std::string>();
				port = varMap["port"].as<int>();

				BOOST_LOG_TRIVIAL(info) << "Connecting to " << address << ":" << port;

				client->connect(address, port);

				BOOST_LOG_TRIVIAL(info) << "Connected to " << address << ":" << port;
			}
			catch (std::exception& e) {
				BOOST_LOG_TRIVIAL(fatal) << "Exception occurred while connecting to peer." << e.what();
				printSubcommandHelp(commandString + " [options] ...", pos);
			}

			break;
		}
		case CliCommandEnum::SEND:
		{
			std::string message;

			po::options_description subDesc("Send options");
			subDesc.add_options()
				("help", "Show connect help")
				("message", po::value<std::vector<std::string>>(), "Message string to send.");

			std::vector<std::string> subOpts = po::collect_unrecognized(parsedOpts.options, po::include_positional);
			subOpts.erase(subOpts.begin());

			po::positional_options_description pos;
			pos.add("message", -1);

			// Reparse leftover args...
			po::store(po::command_line_parser(subOpts).options(subDesc).positional(pos).run(), varMap);

			try {
				message = boost::algorithm::join(varMap["message"].as<std::vector<std::string>>(), " ");

				BOOST_LOG_TRIVIAL(info) << "Sending message \"" << message << "\"";

				client->sendMessage(message);
			}
			catch (std::exception& e) {
				BOOST_LOG_TRIVIAL(fatal) << "Exception occurred while sending message." << e.what();
				printSubcommandHelp(commandString + " [options] ...", pos);
			}

			break;
		}
		case CliCommandEnum::ROOM:
			BOOST_LOG_TRIVIAL(info) << "ROOM";
			// bruhEngine::client->joinRoom(args);
			break;
		case CliCommandEnum::LEAVE:
			BOOST_LOG_TRIVIAL(info) << "LEAVE";
			// bruhEngine::client->leaveRoom(args);
			break;
		case CliCommandEnum::STATUS:
			bruhEngine::client->printStatus();
			break;
		case CliCommandEnum::EXIT:
			BOOST_LOG_TRIVIAL(info) << "EXIT";
			wrapUp();
			break;
		}
	}

	return EXIT_SUCCESS;
}

int bruhEngine::wrapUp() {
	exit(0);
}