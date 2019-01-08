#pragma once

#ifndef LOGGER_INCLUDES
#define LOGGER_INCLUDES

#include <string>
#include <iostream>
#include <mutex>

#endif

#define INITIATE_LOGGER logger console_log

//by changing and undefining (empty define) these following blocks you may change what type of log will be printed to console, defining a preprocessor definition is required
#ifdef TDS_RELEASE

#define LOG_NOTICE(msg) console_log.log("notice", msg)
#define LOG_WARNING(msg) console_log.log("warning", msg)
#define LOG_ERROR(msg) console_log.log("error", msg)
#define LOG_SUCCESS(msg) console_log.log("success", msg)
#define LOG_IMPT_NOTICE(msg) console_log.log("important_notice", msg)
#define LOG_SPAM(msg)
#define LOG(msg)

#else

#define LOG_NOTICE(msg) console_log.log_level("notice"); console_log.log(msg)
#define LOG_WARNING(msg) console_log.log_level("warning"); console_log.log(msg)
#define LOG_ERROR(msg) console_log.log_level("error"); console_log.log(msg)
#define LOG_SUCCESS(msg) console_log.log_level("success"); console_log.log(msg)
#define LOG_IMPT_NOTICE(msg) console_log.log_level("important_notice"); console_log.log(msg)
#define LOG_SPAM(msg) console_log.log_level("spam"); console_log.log(msg)
#define LOG(msg) console_log.log_level(""); console_log.log(msg)

#endif

class logger {
private:
	const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::mutex console;
	int color = 15;
	std::string message_prefix_a;
	std::wstring message_prefix_w;
public:
	std::string level;
	void log(const std::string &level, const std::string &message);
	void log(const std::string &level, const std::wstring &message);
	void log_level(const std::string &level);
};

inline void logger::log(const std::string &level, const std::string &message) {
	console.lock();
	log_level(level);
	SetConsoleTextAttribute(hConsole, color);
	std::cout << message_prefix_a << message << std::endl;
	console.unlock();
	SetConsoleTextAttribute(hConsole, 7);
}

inline void logger::log(const std::string &level, const std::wstring &message) {
	console.lock();
	log_level(level);
	SetConsoleTextAttribute(hConsole, color);
	std::wcout << message_prefix_w << message << std::endl;
	console.unlock();
	SetConsoleTextAttribute(hConsole, 7);
}

inline void logger::log_level(const std::string &level) {
	if (level == "notice") {
		color = 8;
		message_prefix_a = "[-] ";
		message_prefix_w = L"[-] ";
	}
	else if (level == "warning") {
		color = 14;
		message_prefix_a = "[!] ";
		message_prefix_w = L"[!] ";
	}
	else if (level == "error") {
		color = 12;
		message_prefix_a = "[x] ";
		message_prefix_w = L"[x] ";
	}
	else if (level == "success") {
		color = 10;
		message_prefix_a = "[+] ";
		message_prefix_w = L"[+] ";
	}
	else if (level == "important_notice") {
		color = 15;
		message_prefix_a = "[-] ";
		message_prefix_w = L"[-] ";
	}
	else if (level == "spam") {
		color = 9;
		message_prefix_a = "";
		message_prefix_w = L"";
	}
	else {
		color = 7;
		message_prefix_a = "";
		message_prefix_w = L"";
	}
}