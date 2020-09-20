#pragma once

#ifndef IMEXCEPTION_H
#define IMEXCEPTION_H

#include <exception>

class NoFreePortException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	NoFreePortException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class SocketNotCreatedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	SocketNotCreatedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class BindFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	BindFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class ListenFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	ListenFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class AcceptFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	AcceptFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class SetupFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	SetupFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class RunFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	RunFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class ConnectionFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	ConnectionFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class SendMessageFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	SendMessageFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class DaemonAttachFailedException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	DaemonAttachFailedException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class ServerInitException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	ServerInitException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};

class ClientInitException : public std::exception {
	const char* file;
	int line;
	const char* func;
	const char* info;

public:
	ClientInitException(const char* msg, const char* file_, int line_, const char* func_, const char* info_ = "") : std::exception(msg),
		file(file_),
		line(line_),
		func(func_),
		info(info_)
	{
	}

	const char* get_file() const { return file; }
	int get_line() const { return line; }
	const char* get_func() const { return func; }
	const char* get_info() const { return info; }
};
#endif