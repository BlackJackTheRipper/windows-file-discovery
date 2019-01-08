#pragma once

#include "pch.h"

#ifndef INCLUDES
#define INCLUDES

#include "logger.h"
INITIATE_LOGGER;
#include "search_request.h"

#endif

//synchronizers
CRITICAL_SECTION priv_files_sync;
CRITICAL_SECTION priv_dirs_sync;

class search_request {
private:
	//prealocate, add 1000 files as a buffer
	const int priv_prealloc = 250000;
	//queues and vectors to hold pub_files and directories
	std::queue <std::wstring> priv_dirs;
	//maxthreads is the CPU core count of the executing system
	int priv_mthreads = std::thread::hardware_concurrency();
	//deffinition of handler function input
	typedef void(*output_function)(std::vector <std::wstring> &output);

public:
	//user defined variables
	int pub_speed_mode = 1;
	std::wstring pub_search_filename;
	std::string pub_mode;
	std::wstring pub_dir_to_search;
	output_function pub_output_handler;

	//results of search
	std::vector <std::wstring> pub_files;
	std::vector <std::wstring> pub_found_files;
	size_t filecount = 0;

	void search_worker();
	void output(std::vector <std::wstring> &output) const;
	static bool file_exists(const std::wstring &file_name);

	//function to set all required variables for a search in one operation
	void set_variables(const int speed_mode_input, const std::string mode_input, const std::wstring dir_to_search_input, void(*output_function_input)(std::vector <std::wstring> &output), const std::wstring &search_filename_input) {
		pub_speed_mode = speed_mode_input;
		pub_mode = mode_input;
		pub_search_filename = search_filename_input;
		pub_dir_to_search = dir_to_search_input;
		pub_output_handler = output_function_input;
	}

	void initiate_search() {

		//this area requires synchronization -> initialize the critical sectors

		InitializeCriticalSection(&priv_files_sync);
		InitializeCriticalSection(&priv_dirs_sync);

		//push the maindir to the queue and preallocate space for the vectors (-> will require less memory management down the line)
		priv_dirs.push(pub_dir_to_search);
		pub_files.reserve(priv_prealloc);
		pub_found_files.reserve(5);

		//creation of threadpool
		std::vector <std::thread> threadpool;
		for (int i = 0; i < priv_mthreads; i++) {
			//create a thread with the search_worker function
			threadpool.push_back(std::thread(&search_request::search_worker, this));
		}
		//when the threads exit do a join operation on everyone
		std::for_each(threadpool.begin(), threadpool.end(), std::mem_fn(&std::thread::join));

		DeleteCriticalSection(&priv_files_sync);
		DeleteCriticalSection(&priv_dirs_sync);
		//synchronization area ends here

		//after the search is done dump the rest of the result to the output function
		if (pub_mode == "index") {
			filecount = filecount + pub_files.size();
			output(pub_files);
			pub_files.clear();
		}
	}
};

//uses a user-defined function and gives that function a vector to process
inline void search_request::output(std::vector <std::wstring> &output) const {
	pub_output_handler(output);
}

//cheks if a file specified exists
inline bool search_request::file_exists(const std::wstring &file_name) {
	const DWORD dw_attrib = GetFileAttributes(file_name.c_str());
	if (!(dw_attrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return TRUE;
	}
	return FALSE;
}

//a thread-ready directory search worker
inline void search_request::search_worker() {
	WIN32_FIND_DATA FindFileData;
	//creating thread local handlers
	HANDLE thread_local find_handle;
	//defining variables
	int sleepcount = 0;
	const wchar_t file_querry[2] = L"*";
	const wchar_t sub_folder[2] = L"\\";
	std::wstring home_dir;

	//loop for ever
	while (TRUE) {
		//check if priv_dirs queue has a member
		EnterCriticalSection(&priv_dirs_sync);
		if (priv_dirs.empty()) {
			//when the queue is empty sleep for 50ms and then check again (another thread may have found a directory in the meantime)
			LeaveCriticalSection(&priv_dirs_sync);
			LOG_SPAM("IDLE THREAD");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			EnterCriticalSection(&priv_dirs_sync);
			if (priv_dirs.empty()) {
				//if the queue is still empty break the loop and quit this thread
				LOG_SPAM("THREAD DIED");
				LeaveCriticalSection(&priv_dirs_sync);
				break;
			}
		}
		//take the first element of the queue and leave the critical section
		home_dir = priv_dirs.front();
		priv_dirs.pop();
		LeaveCriticalSection(&priv_dirs_sync);

		//start the search if the directory string is not just empty
		if (!home_dir.empty()) {
			//define where and what the thread is searching
			std::wstring search = home_dir + sub_folder;
			std::wstring search_querry = search + file_querry;
			LOG_SPAM(L"searching " + search_querry);

			//now use the handle to find the first file matching the parameters
			find_handle = FindFirstFile(search_querry.c_str(), &FindFileData);
			//do nothing if there is an error with the handle
			if (find_handle == INVALID_HANDLE_VALUE) {}
			//if a file was found
			else do {
				//define some strings with the result and the full path
				std::wstring result_only = FindFileData.cFileName;
				std::wstring result_full = search + result_only;

				//skip the . and .. (upper and this folder designations)
				if (result_only == L"." || result_only == L"..") {
					continue;
				}
				//check whether the result is a file or not
				if (file_exists(result_full)) {
					//if it is a file:
					LOG_SPAM(result_only + L" found in: " + result_full);
					//when searching
					if (pub_mode == "search") {
						//check if the found file matches the search
						if (result_only.find(pub_search_filename) != std::wstring::npos) {
							//send the result back to the user-defined output variable
							LOG_SPAM(result_only + L" matches the search");
							EnterCriticalSection(&priv_files_sync);
							pub_found_files.push_back(result_full);
							output(pub_found_files);
							pub_found_files.clear();
							pub_found_files.reserve(1);
							LeaveCriticalSection(&priv_files_sync);
						}
					}
					else {
						//add the result to the result vector
						EnterCriticalSection(&priv_files_sync);
						pub_files.push_back(result_full);
						LeaveCriticalSection(&priv_files_sync);
					}
				}
				//if it is not a file it has to be a directory
				else {
					//add the directory to the queue
					LOG_SPAM(L"directory discovered: " + result_full);
					EnterCriticalSection(&priv_dirs_sync);
					priv_dirs.push(result_full);
					LeaveCriticalSection(&priv_dirs_sync);
				}
				//when using "slow" priorities wait now
				if (pub_speed_mode == 3) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				//repeat these steps while new files are found
			} while (FindNextFile(find_handle, &FindFileData));
			FindClose(find_handle);

			//when using "high" or "normal" priorities
			if (pub_speed_mode == 1 || pub_speed_mode == 2) {
				//count up everytime we get here and if we hit 5 wait
				if (sleepcount < 5) {
					sleepcount++;
				}
				else {
					sleepcount = 0;
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
		//when using indey mode check if the files in the found vector are about to fill up the preallocated space and if so return and clear them
		EnterCriticalSection(&priv_files_sync);
		const size_t size = pub_files.size();
		if (size > (priv_prealloc - 500) && pub_speed_mode > FALSE && pub_mode == "index") {
			filecount = filecount + size;
			output(pub_files);
			pub_files.clear();
			pub_files.reserve(priv_prealloc);
		}
		LeaveCriticalSection(&priv_files_sync);
	}
}