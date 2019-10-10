#pragma once
#include "pch.h"
#include "spinlock/spinlock.h"

struct console_message {
	short text_color;
	std::string message;
};

class logger {
public:
	enum log_type { success, error, important, msg, imp_diagnostic, diagnostic };
	enum log_level { basic, extended };
	static logger& get();
	logger();
private:
	log_level loglevel;
	spinlock console;
	static console_message message_transform(log_type& lt, std::string& message);
	const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	bool check_ll(log_type& lt) const;
public:
	void log_threaded_strong(log_type lt, std::string message);
	void log_threaded_weak(log_type lt, std::string message);
	void log(log_type lt, std::string message);
};