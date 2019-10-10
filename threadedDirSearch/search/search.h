#pragma once
#include "pch.h"
#include "spinlock/spinlock.h"
#include "console/log_console.h"

template <int T>
struct atomic_ws_array {
private:
	struct data { std::wstring string; spinlock lock; };
	std::array<data, T> array = { L"" };
	static const unsigned int repeats_before_false = 3;
public:
	bool fill_first(std::wstring& input);
	std::wstring get_first();
	bool empty();
};

struct ws_vector_buffer {
private:
	spinlock readlock;
	spinlock writelock;
	std::vector<std::shared_ptr<std::wstring>> read_vector;
	std::vector<std::shared_ptr<std::wstring>> write_vector;
public:
	void add(std::wstring input);
	std::wstring get();
	void transfer_buffer();
	bool empty();
};

class search {
private:
	atomic_ws_array<512> dir_queue;
	atomic_ws_array<256> files_queue;
	ws_vector_buffer dir_buffer;
	ws_vector_buffer files_buffer;
	unsigned int cons_threadlimit = std::thread::hardware_concurrency() / 2;
	const unsigned int prod_threadlimit = std::thread::hardware_concurrency() - cons_threadlimit;
	
	static const unsigned int write_buffer_size = 50;

	bool is_file(const WIN32_FIND_DATA& FindFileData);
	spinlock filewrite;
	void write_to_file(std::wstring& ofilename, std::array<std::wstring, write_buffer_size>& write_buffer);
	spinlock results;
	void add_to_results(std::wstring& input);
	std::vector<std::wstring> results_vec;
	void transfer_buffers();

	std::atomic<unsigned int> dead_producers = 0;
	std::atomic<unsigned int> dead_consumers = 0;

	std::vector<std::wstring> folders_from_string(std::wstring& string);
	std::wstring filename_from_string(std::wstring& string);
public:
	enum mode { list, find_combined, find_file, find_folder };
	static std::wstring singletowide(const std::string& input);
	static std::string widetosingle(const std::wstring& input);
	void init_search(mode mode_in, std::wstring& dir_input, std::wstring search_string = L"", bool exclude_windir = true);

private:
	void search_producer(bool exclude_windir);
	void search_consumer(mode mode_in, std::wstring search_string);
};

template <int T>
bool atomic_ws_array<T>::fill_first(std::wstring& input) {
	int i = 0;
	while (TRUE) {
		i++;
		for (int i = 0; i < T; i++) {
			if (array[i].lock.acquire_weak()) {
				if (array[i].string.empty()) {
					array[i].string = input;
					array[i].lock.release();
					return true;
				}
				array[i].lock.release();
			}
		}
		if (i >= repeats_before_false)
			return false;
	}
}

template <int T>
std::wstring atomic_ws_array<T>::get_first() {
	std::wstring string = L"";
	int i = 0;
	while (TRUE) {
		i++;
		for (int i = 0; i < T; i++) {
			if (array[i].lock.acquire_weak()) {
				if (!array[i].string.empty()) {
					string = array[i].string;
					array[i].string.clear();
					array[i].lock.release();
					return string;
				}
				array[i].lock.release();
			}
		}
		if (i >= repeats_before_false)
			return string;
	}
}

template <int T>
bool atomic_ws_array<T>::empty() {
	for (int i = 0; i < T; i++) {
		if (array[i].string != L"") {
			return false;
		}
	}
	return true;
}