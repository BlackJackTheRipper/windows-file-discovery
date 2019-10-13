#include "../required.h"
#include "search.h"

void returnvec::add(std::wstring& input) {
	lock.acquire_strong();
	vector.push_back(std::make_unique<std::wstring>(input));
	lock.release();
}


std::wstring* returnvec::get(unsigned int index) {
	std::wstring* ptr = nullptr;
	lock.acquire_strong();
	if (!vector.empty() && index < vector.size()) {
		if (!vector.empty() && vector[index] != nullptr) {
			ptr = vector[index].get();
		}
	}
	lock.release();
	return ptr;
}

std::wstring* returnvec::get_weak(unsigned int index) {
	std::wstring* ptr = nullptr;
	if (lock.acquire_weak()) {
		if (!vector.empty() && index < vector.size()) {
			if (vector[index] != nullptr) {
				ptr = vector[index].get();
			}
		}
		lock.release();
	}
	return ptr;
}

void returnvec::clear() {
	lock.acquire_strong();
	vector.clear();
	lock.release();
}

bool returnvec::empty() {
	bool empty = true;
	lock.acquire_strong();
	empty = vector.empty();
	lock.release();
	return empty;
}

size_t returnvec::size() {
	size_t size_return = 0;
	lock.acquire_strong();
	size_return = vector.size();
	lock.release();
	return size_return;
}

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

void search::start_search(mode mode_in, std::wstring& dir_input, returnvec& results, std::wstring search_string, bool exclude_windir) {
	results_vec = &results;
	const unsigned int pairs = std::thread::hardware_concurrency() / 2;
	store.dirs.push_back(dir_input);
	std::vector <std::thread> threadpool;
	
	for (unsigned int i = 0; i < pairs; i++) {
		consumers.add_thread(i);
		threadpool.push_back(std::thread(&search::search_producer, this, i, exclude_windir));
	}
	
	for (unsigned int i = 0; i < pairs; i++) {
		producers.add_thread(i);
		threadpool.push_back(std::thread(&search::search_consumer, this, i, mode_in, search_string));
	}

	for (auto& thread : threadpool) {
		thread.join();
	}
}