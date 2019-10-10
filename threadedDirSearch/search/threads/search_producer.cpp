#include "pch.h"
#include "../search.h"

void search::search_producer(bool exclude_windir) {
	WIN32_FIND_DATA FindFileData;
	//creating thread local handlers
	HANDLE thread_local find_handle;
	const wchar_t file_querry[2] = L"*";
	const wchar_t sub_folder[2] = L"\\";
	bool quit = false;

	while (TRUE) {
		if (dir_queue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (dir_queue.empty()) {
				dead_producers++;
				while (TRUE) {
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					if (!dir_queue.empty()) {
						dead_producers--;
						break;
					}
					if (dead_producers >= prod_threadlimit) {
						quit = true;
						break;
					}
				}
			}
		}
		
		if (quit == true)
			break;
		
		//define where and what the thread is searching
		std::wstring search = dir_queue.get_first() + sub_folder;
		std::wstring search_querry = search + file_querry;
		
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
			if (search == L"\\" || result_only == L"." || result_only == L".." || (exclude_windir == true && result_only == L"Windows")) {
				continue;
			}

			//logger::get().log_threaded_weak(logger::diagnostic, "Found a file: " + widetosingle(result_only));
			
			//check whether the result is a file or not
			if (is_file(FindFileData)) {
				if (files_queue.fill_first(result_full) == false)
					files_buffer.add(result_full);
			}
			//if it is not a file it has to be a directory
			else {
				if (dir_queue.fill_first(result_full) == false)
					dir_buffer.add(result_full);
			}
			//repeat these steps while new files are found
		} while (FindNextFile(find_handle, &FindFileData));
		FindClose(find_handle);
	}
}