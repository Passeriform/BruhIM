#pragma once

#ifndef IPADDRESSTYPE_H
#define IPADDRESSTYPE_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct class ip_t {
	std::string value;

	static constexpr const char* option_name() { return "address"; }

	static constexpr const char* description() { return "IPv4 address of a machine. Value must be in the form <_>.<_>.<_>.<_>. For example, 127.0.0.1"; }

	void check_value() const
		try {
		if (value != "foo" and value != "bar") {
			std::ostringstream ss;
			ss << "value must be foo or bar, you supplied " << std::quoted(value);
			throw std::invalid_argument(ss.str());
		}
	}
	catch (...) {
		std::throw_with_nested(po::validation_error(po::validation_error::invalid_option_value, option_name()));
	}

	// overload operators
	friend std::istream& operator>>(std::istream& is, foo_or_bar& arg) {
		is >> arg.value;
		arg.check_value();
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, foo_or_bar const& arg) {
		return os << arg.value;
	}

};

// test
void test(int argc, const char** argv) {
	foo_or_bar my_foo;

	po::options_description desc("test options");
	desc.add_options()
		(foo_or_bar::option_name(), po::value(&my_foo), foo_or_bar::description());

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);


	std::cout << "foo is " << my_foo << std::endl;
}

void print_exception(const std::exception& e, int level = 0)
{
	std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
	try {
		std::rethrow_if_nested(e);
	}
	catch (const std::exception& e) {
		print_exception(e, level + 1);
	}
	catch (...) {}
}

int main() {
	{
		std::vector<const char*> test_args = {
				"executable_name",
				"--foo=bar"
		};
		test(test_args.size(), test_args.data());
	}

	try {
		std::vector<const char*> test_args = {
				"executable_name",
				"--foo=bob"
		};
		test(test_args.size(), test_args.data());
	}
	catch (std::exception const& e) {
		print_exception(e);
	}
}

#endif