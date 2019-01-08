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
	const int prealloc = 250000;
	//queues and vectors to hold pub_files and directories
	std::queue <std::wstring> priv_dirs;
	//preset variables
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

	void set_variables(const int speed_mode_input, const std::string mode_input, const std::wstring dir_to_search_input, void(*output_function_input)(std::vector <std::wstring> &output), const std::wstring &search_filename_input) {
		pub_speed_mode = speed_mode_input;
		pub_mode = mode_input;
		pub_search_filename = search_filename_input;
		pub_dir_to_search = dir_to_search_input;
		pub_output_handler = output_function_input;
	}

	void initiate_search() {

		//this area requires synchronization

		InitializeCriticalSection(&priv_files_sync);
		InitializeCriticalSection(&priv_dirs_sync);
	
		priv_dirs.push(pub_dir_to_search);
		pub_files.reserve(prealloc);
		pub_found_files.reserve(1);

		//threadpool
		std::vector <std::thread> threadpool;
		for (int i = 0; i < priv_mthreads; i++) {
			threadpool.push_back(std::thread(&search_request::search_worker, this));
		}
		std::for_each(threadpool.begin(), threadpool.end(), std::mem_fn(&std::thread::join));

		DeleteCriticalSection(&priv_files_sync);
		DeleteCriticalSection(&priv_dirs_sync);
		//synchronization area ends here

		if (pub_mode == "index") {
			filecount = filecount + pub_files.size();
			output(pub_files);
			pub_files.clear();
		}
		else {
			filecount = filecount + pub_files.size();
			output(pub_found_files);
			pub_files.clear();
		}
	}
};

//uses a user-defined function and gives that function a vector to work with
inline void search_request::output(std::vector <std::wstring> &output) const {
	pub_output_handler(output);
}

inline bool search_request::file_exists(const std::wstring &file_name) {
	const DWORD dw_attrib = GetFileAttributes(file_name.c_str());
	if (!(dw_attrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return TRUE;
	}
	return FALSE;
}

inline void search_request::search_worker() {
	WIN32_FIND_DATA FindFileData;
	HANDLE thread_local find_handle;
	std::wstring home_dir;
	int thread_local sleepcount = 0;
	const wchar_t file_querry[2] = L"*";
	std::wstring test = L"*";
	const wchar_t sub_folder[2] = L"\\";

	while (TRUE) {
		EnterCriticalSection(&priv_dirs_sync);
		if (priv_dirs.empty()) {
			LeaveCriticalSection(&priv_dirs_sync);
			LOG_SPAM("IDLE THREAD");
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			EnterCriticalSection(&priv_dirs_sync);
			if (priv_dirs.empty()) {
				LOG_SPAM("THREAD DIED");
				LeaveCriticalSection(&priv_dirs_sync);
				break;
			}
		}
		home_dir = priv_dirs.front();
		priv_dirs.pop();
		LeaveCriticalSection(&priv_dirs_sync);

		if (!home_dir.empty()) {
			std::wstring search = home_dir + sub_folder;
			std::wstring search_querry = search + file_querry;
			LOG_SPAM(L"searching " + search_querry);

			find_handle = FindFirstFile(search_querry.c_str(), &FindFileData);
			if (find_handle == INVALID_HANDLE_VALUE) {}
			else do {
				std::wstring result_only = FindFileData.cFileName;
				std::wstring result_full = search + result_only;

				if (result_only == L"." || result_only == L"..") {
					continue;
				}
				if (file_exists(result_full)) {
					LOG_SPAM(result_only + L" found in: " + result_full);
					if (pub_mode == "search") {
						if (result_only.find(pub_search_filename) != std::wstring::npos) {
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
						EnterCriticalSection(&priv_files_sync);
						pub_files.push_back(result_full);
						LeaveCriticalSection(&priv_files_sync);
					}
				}
				else {
					LOG_SPAM(L"directory discovered: " + result_full);
					EnterCriticalSection(&priv_dirs_sync);
					priv_dirs.push(result_full);
					LeaveCriticalSection(&priv_dirs_sync);
				}
				if (pub_speed_mode == 3) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			} while (FindNextFile(find_handle, &FindFileData));
			FindClose(find_handle);

			if (pub_speed_mode == 1 || pub_speed_mode == 2) {
				if (sleepcount < 5) {
					sleepcount++;
				}
				else {
					sleepcount = 0;
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}

		EnterCriticalSection(&priv_files_sync);
		const size_t size = pub_files.size();
		if (size > prealloc - priv_mthreads && pub_speed_mode > FALSE && pub_mode == "index") {
			filecount = filecount + size;
			output(pub_files);
			pub_files.clear();
			pub_files.reserve(prealloc);
		}
		LeaveCriticalSection(&priv_files_sync);
	}
}