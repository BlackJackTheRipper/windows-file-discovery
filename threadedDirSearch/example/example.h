#pragma once
#include "../windows_file_search/include.h"

class example_use {
private:
	returnvec output;
	std::wstring dir;
	std::wstring search_str;
	search search_handle;
	std::atomic<bool> quit_output = false;

	static bool isdir(std::wstring& dir);
	
	void search_handler();
	void output_handler();

public:
	example_use(std::wstring dir_in, std::wstring search_str_in);
	void search();
};