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

namespace po = boost::program_options;
namespace logging = boost::log;

using namespace libconfig;

IMServer* bruhEngine::server = nullptr;
IMClient* bruhEngine::client = nullptr;

enum CliCommandEnum {
	HELP,
	CONNECT,
	SEND,
	ROOM,
	LEAVE,
	EXIT
};

std::map< std::string, CliCommandEnum> CliCommands = {
	{"!help", CliCommandEnum::HELP},
	{"!connect", CliCommandEnum::CONNECT},
	{"!send", CliCommandEnum::SEND},
	{"!room", CliCommandEnum::ROOM},
	{"!leave", CliCommandEnum::LEAVE},
	{"!exit", CliCommandEnum::EXIT},
};

std::vector<std::string> tokenize(const std::string& input) {
	typedef boost::escaped_list_separator<char> separator_type;
	separator_type separator(
		"\\",			// The escape characters.
		"= ",			// The separator characters.
		"\"\'"			// The quote characters.
	);

	// Tokenize the intput.
	boost::tokenizer<separator_type> tokens(input, separator);

	// Copy non-empty tokens from the tokenizer into the result.
	std::vector<std::string> result;
	copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
		!boost::bind(&std::string::empty, _1));
	return result;
}

void printHelp(std::string usageString) {
	std::cout << "\nUsage: " << usageString << std::endl;
	std::cout << "Available commands:" << std::endl;

	for (auto availIter = CliCommands.begin(); availIter != CliCommands.end(); availIter++) {
		std::cout << " " << availIter->first << std::endl;
	}
	std::cout << std::endl;
}

void printSubcommandHelp(std::string usageString, po::positional_options_description description) {
	std::cout << "\nUsage: " << usageString << std::endl;
	std::cout << "Available commands:" << std::endl;

	for (auto idx = 0; idx < description.max_total_count(); idx++) {
		const std::string& optionString = description.name_for_position(idx);

		std::cout << " " << optionString << std::endl;
	}
	std::cout << std::endl;
}

int bruhEngine::init(libconfig::Config* config) {
	try
	{
		config->readFile("config/engine.cfg");
	}
	catch (const FileIOException& fioex)
	{
		std::cerr << "I/O error while reading file." << fioex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (const ParseException& pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}

int bruhEngine::runServer(libconfig::Config* config) {
	int port = lookupConfig<int>("port", config, 3000);

	try {
		server = new IMServer(IMObjectPool::addNewWorker());
		(server->getWorker())->promote();
		server->setup(port);

		server->run();
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(info) << "Server error captured." << e.what();
	}

	try {
		client = new IMClient();
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(info) << "Client error captured." << e.what();
	}

	return 0;
}

int bruhEngine::attachDaemon(int threadObjectId) {
	try {}
	catch (std::exception& e) {
		std::throw_with_nested(DaemonAttachFailedException("Process couldn't be daemonized due to an internal error.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

int bruhEngine::runDaemon(libconfig::Config* config) {
	int threadObjectId = runServer(config);

	try {
		attachDaemon(threadObjectId);
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << "Daemon error captured." << e.what();
	}

	return 0;
}

int bruhEngine::streamInput() {
	while (true) {

		// Legacy Solution using Regex
		//std::regex_search(
		//	input,
		//	command,
		//	std::regex("(?:^|\\n|\\r)(?:\\!)(connect|send|room|leave|exit)($|[\n\r\s]+|[^\!]+)")
		//);

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
			commandString = "!help";
			command = CliCommandEnum::HELP;
		}

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
			}
			catch (std::exception& e) {
				std::cout << e.what();
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
				("message", po::value<std::string>(), "Message string to send.");

			std::vector<std::string> subOpts = po::collect_unrecognized(parsedOpts.options, po::include_positional);
			subOpts.erase(subOpts.begin());

			po::positional_options_description pos;
			pos.add("message", -1);

			// Reparse leftover args...
			po::store(po::command_line_parser(subOpts).options(subDesc).positional(pos).run(), varMap);

			try {
				message = varMap["message"].as<std::string>();

				BOOST_LOG_TRIVIAL(info) << "Sending message \"" << message << "\"";

				client->sendMessage(message);
			}
			catch (std::exception& e) {
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
		case CliCommandEnum::EXIT:
			BOOST_LOG_TRIVIAL(info) << "EXIT";
			wrapUp();
			break;
		}
	}
}

int bruhEngine::wrapUp() {
	bruhEngine::client->closeSocket();
	bruhEngine::server->closeSocket();
	exit(0);
}