#include "example.h"

example_use::example_use(std::wstring dir_in) {
	dir = dir_in;
}

void example_use::search(bool muted) {
	if (!isdir(dir)) {
		std::cout << "[x] The directory entered does not exist\n";
		return;
	}
	if (muted == false)
		std::cout << "[+] starting search\n\n";
	
	std::thread t1_search(&example_use::search_handler, this);
	std::thread t2_output(&example_use::output_handler, this, muted);

	t1_search.join();
	t2_output.join();

	if (muted == false)
		std::cout << "\n[+] search ended\n";
}


void example_use::search_handler() {
	search_handle.start_search(search::list, dir, output);
	quit_output.store(true);
}

void example_use::output_handler(bool muted) {
	if (muted == false) {
		static size_t counter = 0;
		std::wstring current;
		while (TRUE) {
			current = output.pop();
			if(!current.empty()) {
				counter++;
				std::wcout << L"[-] Found: " << current << std::endl;
			}
			if (quit_output == true)
				break;
		}
	}
}

// checks if a directory specified exists, duh
bool example_use::isdir(std::wstring& dir) {
	const DWORD ftype = GetFileAttributesW(dir.c_str());
	if (ftype == INVALID_FILE_ATTRIBUTES) {
		return FALSE;
	}
	if (ftype & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	}
	return FALSE;
}