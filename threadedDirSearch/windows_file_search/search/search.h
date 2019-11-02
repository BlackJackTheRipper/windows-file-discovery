#pragma once

#include "../required.h"
#include "threadwatch/tw.h"
#include "slist/slist.h"

class search {
private:
	slist<std::wstring> files;
	slist<std::wstring> dirs;
	
	bool is_file(const WIN32_FIND_DATA& FindFileData);
	slist<std::wstring>* results_vec = nullptr;
	void add_to_results(std::wstring& input);
	
	threadwatch consumers;
	threadwatch producers;

	std::vector<std::wstring> folders_from_string(std::wstring& string);
	std::wstring filename_from_string(std::wstring& string);

public:
	enum mode { list, find_combined, find_file, find_folder };
	static std::wstring singletowide(const std::string& input);
	static std::string widetosingle(const std::wstring& input);
	void start_search(mode mode_in, std::wstring& dir_input, slist<std::wstring>& results, std::wstring search_string = L"", bool exclude_windir = true);
	
private:
	void search_producer(unsigned int tandem_id, bool exclude_windir);
	void search_consumer(unsigned int tandem_id, mode mode_in, std::wstring search_string);
};