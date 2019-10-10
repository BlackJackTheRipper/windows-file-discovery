#include "pch.h"
#include "search.h"

// convert widestring to sinlge byte
std::string search::widetosingle(const std::wstring& input) {
	std::wstring_convert < std::codecvt_utf8_utf16 <wchar_t> > converter;
	const std::string output = converter.to_bytes(input);
	return output;
}

// convert single byte to widestring
std::wstring search::singletowide(const std::string& input) {
	std::wstring_convert < std::codecvt_utf8_utf16 <wchar_t> > converter;
	const std::wstring output = converter.from_bytes(input);
	return output;
}

bool search::is_file(const WIN32_FIND_DATA& FindFileData) {
	if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return TRUE;
	}
	return FALSE;
}

void search::init_search(mode mode_in, std::wstring& dir_input, std::wstring search_string, bool exclude_windir) {
	if (std::thread::hardware_concurrency() < 4) {
		return;
	}
	
	dir_queue.fill_first(dir_input);

	//creation of threadpool
	std::vector <std::thread> threadpool;
	std::vector <std::thread> producers;
	for (int i = 0; i < unsigned int (prod_threadlimit); i++) {
		//create a thread with the search_worker function
		producers.push_back(std::thread(&search::search_producer, this, exclude_windir));
	}
	for (int i = 0; i < unsigned int(cons_threadlimit); i++) {
		//create a thread with the search_worker function
		threadpool.push_back(std::thread(&search::search_consumer, this, mode_in, search_string));
	}
	threadpool.push_back(std::thread(&search::transfer_buffers, this));
	for (int i = 0; i < producers.size(); i++) {
		producers.at(i).join();
	}
	cons_threadlimit += prod_threadlimit;
	for (int i = 0; i < unsigned int(prod_threadlimit); i++) {
		//create a thread with the search_worker function
		threadpool.push_back(std::thread(&search::search_consumer, this, mode_in, search_string));
	}
	for (int i = 0; i < threadpool.size(); i++) {
			threadpool.at(i).join();
	}
}