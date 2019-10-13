#pragma once

#include "../required.h"
#include "../spinlock/spinlock.h"
#include "tl_storage/tl_storage.h"
#include "threadwatch/tw.h"

struct returnvec {
private:
	spinlock lock;
	std::vector<std::unique_ptr<std::wstring>> vector;
public:
	void add(std::wstring& input);
	std::wstring* get(unsigned int index);
	std::wstring* get_weak(unsigned int index);
	bool empty();
	size_t size();
	void clear();
};

class search {
private:
	struct xconnect {
		tl_storage files;
		tl_storage dirs;
	};
	
	xconnect store;
	
	bool is_file(const WIN32_FIND_DATA& FindFileData);
	returnvec* results_vec = nullptr;
	void add_to_results(std::wstring& input);
	
	threadwatch consumers;
	threadwatch producers;

	std::vector<std::wstring> folders_from_string(std::wstring& string);
	std::wstring filename_from_string(std::wstring& string);

public:
	enum mode { list, find_combined, find_file, find_folder };
	static std::wstring singletowide(const std::string& input);
	static std::string widetosingle(const std::wstring& input);
	void start_search(mode mode_in, std::wstring& dir_input, returnvec& results, std::wstring search_string = L"", bool exclude_windir = true);
	
private:
	void search_producer(unsigned int tandem_id, bool exclude_windir);
	void search_consumer(unsigned int tandem_id, mode mode_in, std::wstring search_string);
};