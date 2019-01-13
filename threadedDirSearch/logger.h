/*
PROGRAMM METADATA:

	name:           general logger class
	description:    c++ windows logger class using a mutex and different colors
	copyright:      (c) 2019 Constantin Fürst constantin@fuersten.info

LICENSE:

   This work is created by Constantin Fürst.
   If you are unsure if your use of this project is permitted or you'd like to use it in a way prohibited by the following license, please message "constantin@fuersten.info".

   This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
   To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-nd/4.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

DISCLAIMER:

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */

#pragma once

#ifndef LOGGER_INCLUDES
#define LOGGER_INCLUDES

#include <string>
#include <iostream>
#include <mutex>

#endif

#define INITIATE_LOGGER logger console_log

//by changing and undefining (empty define) these following blocks you may change what type of log will be printed to console, defining a preprocessor definition is required
#ifdef TDS_DEBUG

#define LOG_NOTICE(msg) console_log.log(logger::notice, msg)
#define LOG_WARNING(msg) console_log.log(logger::warning, msg)
#define LOG_ERROR(msg) console_log.log(logger::error, msg)
#define LOG_SUCCESS(msg) console_log.log(logger::success, msg)
#define LOG_IMPT_NOTICE(msg) console_log.log(logger::important_notice, msg)
#define LOG_SPAM(msg) console_log.log(logger::spam, msg)
#define LOG(msg) console_log.log(logger::undefined, msg)

#else

#define LOG_NOTICE(msg)
#define LOG_WARNING(msg) console_log.log(logger::warning, msg)
#define LOG_ERROR(msg) console_log.log(logger::error, msg)
#define LOG_SUCCESS(msg) console_log.log(logger::success, msg)
#define LOG_IMPT_NOTICE(msg) console_log.log(logger::important_notice, msg)
#define LOG_SPAM(msg)
#define LOG(msg)

#endif

class logger {
private:
	const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::mutex console;
	int color = 15;
	std::string message_prefix_a;
	std::wstring message_prefix_w;
public:
	enum error_level {
		success = 0, error = 1, important_notice = 2, warning = 3, notice = 4, spam = 5, undefined = 6
	};
	void log(const error_level &level, const std::string &message);
	void log(const error_level &level, const std::wstring &message);
	void log_level(const error_level &level);
};

//log function that accepts multi byte strings, using a mutex for threadsafe operation
inline void logger::log(const error_level &level, const std::string &message) {
	console.lock();
	log_level(level);
	SetConsoleTextAttribute(hConsole, color);
	std::cout << message_prefix_a << message << std::endl;
	console.unlock();
	SetConsoleTextAttribute(hConsole, 7);
}

//overwrite for the log function aboce that accepts single byte strings, using a mutex for threadsafe operation
inline void logger::log(const error_level &level, const std::wstring &message) {
	console.lock();
	log_level(level);
	SetConsoleTextAttribute(hConsole, color);
	std::wcout << message_prefix_w << message << std::endl;
	console.unlock();
	SetConsoleTextAttribute(hConsole, 7);
}

//function that sets the log color according to a string level input
inline void logger::log_level(const error_level &level) {
	if (level == success) {
		color = 10;
		message_prefix_a = "[+] ";
		message_prefix_w = L"[+] ";
	}
	else if (level == error) {
		color = 12;
		message_prefix_a = "[x] ";
		message_prefix_w = L"[x] ";
	}
	else if (level == important_notice) {
		color = 15;
		message_prefix_a = "[-] ";
		message_prefix_w = L"[-] ";
	}
	else if (level == warning) {
		color = 14;
		message_prefix_a = "[!] ";
		message_prefix_w = L"[!] ";
	}
	else if (level == notice) {
		color = 8;
		message_prefix_a = "[-] ";
		message_prefix_w = L"[-] ";
	}
	else if (level == spam) {
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