#pragma once
#include "../windows_file_search/include.h"

class example_use {
private:
	slist<std::wstring> output;
	std::wstring dir;
	search search_handle;
	std::atomic<bool> quit_output = false;

	static bool isdir(std::wstring& dir);
	
	void search_handler();
	void output_handler(bool muted);

public:
	example_use(std::wstring dir_in);
	void search(bool muted = false);
};