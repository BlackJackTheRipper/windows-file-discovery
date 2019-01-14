/*
PROGRAMM METADATA:

	name:           windows directory search class
	description:    c++ windows multithreaded search class
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

#include "pch.h"

#ifndef INCLUDES
#define INCLUDES

#include "logger.h"
INITIATE_LOGGER;
#include "search_request.h"

#endif


class search_request {
private: //private variables and functions use the "m_" prefix
	//synchronizers
	CRITICAL_SECTION pub_files_sync;
	CRITICAL_SECTION pub_dirs_sync;

	//enum for result type
	enum m_result_type {
		file = 0, directory = 1
	};

	//used by the search function
	const wchar_t m_file_querry[2] = L"*";
	const wchar_t m_sub_folder[2] = L"\\";

	//prealocate size for vector
	const int m_prealloc = 250000;
	//maxthreads is the CPU core count of the system
	const int m_mthreads = std::thread::hardware_concurrency();
	//queues and vectors to hold m_files and directories
	std::queue <std::wstring> m_dirs;

	//results of search
	std::vector <std::wstring> m_files;
	std::vector <std::wstring> m_found_files;

	//timepointer for runtime meassurement
	std::chrono::high_resolution_clock::time_point m_runtime_start;

	void m_search_worker();
	static bool m_result_type(const WIN32_FIND_DATA &FindFileData);

public: //public variables use the "pub_" prefix
	//enum for application mode
	enum pub_app_mode {
		index = 0, file_search = 1
	};
	//enum for priority
	enum pub_app_priority_mode {
		ultra = 0, high = 1, normal = 2, low = 3
	};

	//deffinition of handler function input
	typedef void(*output_function)(std::vector <std::wstring> &output);

	//user defined variables
	pub_app_priority_mode pub_app_priority;
	pub_app_mode pub_mode;
	output_function pub_output_handler;
	std::wstring pub_search_filename;
	std::wstring pub_dir_to_search;

	//results of search
	size_t pub_filecount = 0;

	void initiate_search();

	//function to set all required variables for a search in one operation
	void set_variables(const pub_app_priority_mode speed_mode_input, const pub_app_mode mode_input, const std::wstring dir_to_search_input, const output_function output_function_input, const std::wstring search_filename_input) {
		pub_app_priority = speed_mode_input;
		pub_mode = mode_input;
		pub_search_filename = search_filename_input;
		pub_dir_to_search = dir_to_search_input;
		pub_output_handler = output_function_input;
	}

	search_request() {
		//set a timepoint here (will be overwritten in search_request::initiate_search)
		m_runtime_start = std::chrono::high_resolution_clock::now();
	}

	~search_request() {
		//use a new timepoint with the old one (set in search_request::initiate_search) to calculate runtime in ms
		const std::chrono::high_resolution_clock::time_point runtime_end = std::chrono::high_resolution_clock::now();
		const std::chrono::milliseconds runtime = std::chrono::duration_cast<std::chrono::milliseconds>(runtime_end - m_runtime_start);

		//if runtime is below 1 seconds warn the user
		if (runtime < std::chrono::milliseconds(1000)) {
			LOG_WARNING("EXECUTION TIME BELOW 1 SECONDS");
		}

		//log the execution time
		std::string runtime_str = std::to_string(runtime.count());
		runtime_str = runtime_str.substr(0, runtime_str.size() - 3) + "," + runtime_str.substr(runtime_str.size() - 3, runtime_str.size());
		LOG_NOTICE("execution time of " + runtime_str + " seconds");
	}
};

//prepares all the stuff for the search and then launches it
inline void search_request::initiate_search() {
	//create the runtime start timepoint
	m_runtime_start = std::chrono::high_resolution_clock::now();
	//this area requires synchronization -> initialize the critical sectors

	InitializeCriticalSection(&pub_files_sync);
	InitializeCriticalSection(&pub_dirs_sync);

	//push the maindir to the queue and preallocate space for the vectors (-> will require less memory management down the line)
	m_dirs.push(pub_dir_to_search);
	m_files.reserve(m_prealloc);
	m_found_files.reserve(5);

	//creation of threadpool
	std::vector <std::thread> threadpool;
	for (int i = 0; i < m_mthreads; i++) {
		//create a thread with the search_worker function
		threadpool.push_back(std::thread(&search_request::m_search_worker, this));
	}

	//when the threads exit do a join operation on everyone
	std::for_each(threadpool.begin(), threadpool.end(), std::mem_fn(&std::thread::join));

	//synchronization area ends here
	DeleteCriticalSection(&pub_files_sync);
	DeleteCriticalSection(&pub_dirs_sync);

	//after the search is done dump the rest of the result to the output function
	if (pub_mode == index) {
		pub_filecount += m_files.size();
		//use predefined function (outside of this class) to handle output
		pub_output_handler(m_files);
		m_files.clear();
	}
}

//cheks if a file specified exists
inline bool search_request::m_result_type(const WIN32_FIND_DATA &FindFileData) {
	if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
		return directory;
	}
	else {
		return file;
	}
}

//thread-ready directory search worker
inline void search_request::m_search_worker() {
	WIN32_FIND_DATA found_file;
	HANDLE thread_local find_handle;
	//defining variables
	int sleepcount = 0;

	//loop for ever
	while (TRUE) {
		//check if m_dirs queue has a member
		EnterCriticalSection(&pub_dirs_sync);
		if (m_dirs.empty()) {
			//when the queue is empty sleep for 50ms and then check again (another thread may have found a directory in the meantime)
			LeaveCriticalSection(&pub_dirs_sync);
			LOG_SPAM("IDLE THREAD");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			EnterCriticalSection(&pub_dirs_sync);
			if (m_dirs.empty()) {
				//if the queue is still empty break the loop and quit this thread
				LOG_SPAM("THREAD DIED");
				LeaveCriticalSection(&pub_dirs_sync);
				break;
			}
		}
		//take the first element of the queue and leave the critical section
		std::wstring home_dir = m_dirs.front();
		m_dirs.pop();
		LeaveCriticalSection(&pub_dirs_sync);

		//start the search if the directory string is not just empty
		if (!home_dir.empty()) {
			//define where and what the thread is searching
			std::wstring search = home_dir + m_sub_folder;
			std::wstring search_querry = search + m_file_querry;
			LOG_SPAM(L"searching " + search_querry);

			//now use the handle to find the first file matching the parameters
			find_handle = FindFirstFile(search_querry.c_str(), &found_file);
			//do nothing if there is an error with the handle
			if (find_handle == INVALID_HANDLE_VALUE) {}
			//if a file was found
			else do {
				//define some strings with the result and the full path
				std::wstring result_only = found_file.cFileName;
				std::wstring result_full = search + result_only;

				//skip the . and .. (upper and this folder designations)
				if (result_only == L"." || result_only == L"..") {
					continue;
				}
				//check whether the result is a file or not
				if (m_result_type(found_file) == file) {
					//if it is a file:
					LOG_SPAM(result_only + L" found in: " + result_full);
					//when searching
					if (pub_mode == file_search) {
						//check if the found file matches the search
						if (result_only.find(pub_search_filename) != std::wstring::npos) {
							//send the result back to the user-defined output variable
							LOG_SPAM(result_only + L" matches the search");
							EnterCriticalSection(&pub_files_sync);
							m_found_files.push_back(result_full);
							//use predefined function (outside of this class) to handle output
							pub_output_handler(m_found_files);
							m_found_files.clear();
							m_found_files.reserve(1);
							LeaveCriticalSection(&pub_files_sync);
						}
					}
					else {
						//add the result to the result vector
						EnterCriticalSection(&pub_files_sync);
						m_files.push_back(result_full);
						LeaveCriticalSection(&pub_files_sync);
					}
				}
				//if it is not a file it has to be a directory
				else {
					//add the directory to the queue
					LOG_SPAM(L"directory discovered: " + result_full);
					EnterCriticalSection(&pub_dirs_sync);
					m_dirs.push(result_full);
					LeaveCriticalSection(&pub_dirs_sync);
				}
				//when using "slow" priorities wait now
				if (pub_app_priority == low) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				//repeat these steps while new files are found
			} while (FindNextFile(find_handle, &found_file));
			FindClose(find_handle);

			//when using "high" or "normal" priorities
			if (pub_app_priority == high || pub_app_priority == normal) {
				//when using high priority wait every 5th time we get here
				if (sleepcount < 5 && pub_app_priority == high) {
					sleepcount++;
				}
				//when using normal priority
				else if (sleepcount < 2 && pub_app_priority == normal) {
					sleepcount++;
				}
				else {
					sleepcount = 0;
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
		//when using index mode check if the files in the found vector are about to fill up the preallocated space and if so return and clear them
		EnterCriticalSection(&pub_files_sync);
		const size_t size = m_files.size();
		if (size > (m_prealloc - 500) && pub_app_priority > FALSE && pub_mode == index) {
			if (size > m_prealloc) {
				LOG_NOTICE("CLEARING VECTOR LARGER THAN PREALLOC");
			}
			else {
				LOG_NOTICE("CLEARING VECTOR");
			}
			pub_filecount += size;
			//use predefined function (outside of this class) to handle output
			pub_output_handler(m_files);
			//clear the vector and preallocate again
			m_files.clear();
			m_files.reserve(m_prealloc);
		}
		LeaveCriticalSection(&pub_files_sync);
	}
}