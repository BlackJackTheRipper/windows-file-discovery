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
#define log_instance console_log
#define LOG_NOTICE(msg) console_log.log(logger::notice, msg)
#define LOG_WARNING(msg) console_log.log(logger::warning, msg)
#define LOG_ERROR(msg) console_log.log(logger::error, msg)
#define LOG_SUCCESS(msg) console_log.log(logger::success, msg)
#define LOG_IMPT_NOTICE(msg) console_log.log(logger::important_notice, msg)
#define LOG_SPAM(msg) console_log.log(logger::spam, msg)
#define LOG(msg) console_log.log(logger::undefined, msg)

class logger {
private:
	const HANDLE m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::mutex m_console;
	int m_color = 15;
	std::string m_message_prefix_a;
	std::wstring m_message_prefix_w;
public:
	enum verbose_level {
		standard = 3, metrics = 4, detailed = 6
	};
	enum error_level {
		success = 0, error = 1, important_notice = 2, warning = 3, notice = 4, spam = 5, undefined = 6
	};
	verbose_level pub_verbose_level = standard;
	void log(const error_level &level, const std::string &message);
	void log(const error_level &level, const std::wstring &message);
	void log_level(const error_level &level);
};

//log function that accepts multi byte strings, using a mutex for threadsafe operation
inline void logger::log(const error_level &level, const std::string &message) {
	if (level <= pub_verbose_level) {
		m_console.lock();
		log_level(level);
		SetConsoleTextAttribute(m_hConsole, m_color);
		std::cout << m_message_prefix_a << message << std::endl;
		m_console.unlock();
		SetConsoleTextAttribute(m_hConsole, 7);
	}
}

//overwrite for the log function aboce that accepts single byte strings, using a mutex for threadsafe operation
inline void logger::log(const error_level &level, const std::wstring &message) {
	if (level <= pub_verbose_level) {
		m_console.lock();
		log_level(level);
		SetConsoleTextAttribute(m_hConsole, m_color);
		std::wcout << m_message_prefix_w << message << std::endl;
		m_console.unlock();
		SetConsoleTextAttribute(m_hConsole, 7);
	}
}

//function that sets the log color according to a string level input
inline void logger::log_level(const error_level &level) {
	if (level == success) {
		m_color = 10;
		m_message_prefix_a = "[+] ";
		m_message_prefix_w = L"[+] ";
	}
	else if (level == error) {
		m_color = 12;
		m_message_prefix_a = "[x] ";
		m_message_prefix_w = L"[x] ";
	}
	else if (level == important_notice) {
		m_color = 15;
		m_message_prefix_a = "[-] ";
		m_message_prefix_w = L"[-] ";
	}
	else if (level == warning) {
		m_color = 14;
		m_message_prefix_a = "[!] ";
		m_message_prefix_w = L"[!] ";
	}
	else if (level == notice) {
		m_color = 8;
		m_message_prefix_a = "[-] ";
		m_message_prefix_w = L"[-] ";
	}
	else if (level == spam) {
		m_color = 9;
		m_message_prefix_a = "";
		m_message_prefix_w = L"";
	}
	else {
		m_color = 7;
		m_message_prefix_a = "";
		m_message_prefix_w = L"";
	}
}