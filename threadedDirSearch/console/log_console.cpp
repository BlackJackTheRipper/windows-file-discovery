#include "pch.h"
#include "log_console.h"

logger::logger() {
	loglevel = extended;
}

logger& logger::get() {
	static logger singleton;
	return singleton;
}

void logger::log_threaded_strong(log_type lt, std::string message) {
	if (!check_ll(lt))
		return;
	console.acquire_strong();
	console_message msg = message_transform(lt, message);
	SetConsoleTextAttribute(hConsole, msg.text_color);
	std::cout << msg.message << std::endl;
	SetConsoleTextAttribute(hConsole, 7);
	console.release();
}

void logger::log_threaded_weak(log_type lt, std::string message) {
	if (!check_ll(lt))
		return;
	if (console.acquire_weak()) {
		console_message msg = message_transform(lt, message);
		SetConsoleTextAttribute(hConsole, msg.text_color);
		std::cout << msg.message << std::endl;
		SetConsoleTextAttribute(hConsole, 7);
		console.release();
	}
}

void logger::log(log_type lt, std::string message) {
	if (!check_ll(lt))
		return;
	console_message msg = message_transform(lt, message);
	SetConsoleTextAttribute(hConsole, msg.text_color);
	std::cout << msg.message << std::endl;
	SetConsoleTextAttribute(hConsole, 7);
}

console_message logger::message_transform(log_type& lt, std::string& message) {
	switch (lt) {
	case success:
		return { 10, "[+] " + message };
	case error:
		return { 12, "[x] " + message };
	case important:
		return { 14, "[!] " + message };
	case msg:
		return { 15, "[-] " + message };
	case diagnostic:
		return { 9, message };
	case imp_diagnostic:
		return { 14, "[!] " + message };
	default:
		return { 7, ""};
	}
}

bool logger::check_ll(log_type& lt) const {
	if (loglevel == basic && lt < diagnostic)
		return true;
	else if (loglevel == extended)
		return true;
	return false;
}
