#include "../../required.h"
#include "../search.h"

void search::search_producer(unsigned int tandem_id, bool exclude_windir) {
	WIN32_FIND_DATA FindFileData;
	//creating thread local handlers
	HANDLE thread_local find_handle;
	const wchar_t file_querry[2] = L"*";
	const wchar_t sub_folder[2] = L"\\";
	bool quit = false;

	while (TRUE) {
		if (dirs.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (dirs.empty()) {
				producers.sleeping(tandem_id);
				while (TRUE) {
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					if (!dirs.empty()) {
						producers.wake(tandem_id);
						break;
					}
					if (producers.alives() <= 1) {
						quit = true;
						break;
					}
				}
			}
		}
		
		if (quit == true)
			break;
		
		//define where and what the thread is searching
		std::wstring search = dirs.pop() + sub_folder;
		std::wstring search_querry = search + file_querry;

		if (search != L"\\") {
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
				if (result_only == L"." || result_only == L".." || (exclude_windir == true && result_only == L"Windows")) {
					continue;
				}

				//check whether the result is a file or not
				if (is_file(FindFileData)) {
					files.push(result_full);
				}
				//if it is not a file it has to be a directory
				else {
					dirs.push(result_full);
				}
				//repeat these steps while new files are found
			} while (FindNextFile(find_handle, &FindFileData));
			FindClose(find_handle);
		}
	}
}