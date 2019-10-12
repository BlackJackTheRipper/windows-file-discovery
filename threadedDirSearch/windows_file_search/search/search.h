#pragma once
#include "../required.h"
#include "../spinlock/spinlock.h"

class threadwatch {
public:
	static const bool ALIVE = true;
	static const bool DEAD = false;
private:
	struct thread_entry {
		std::atomic<unsigned int> id;
		std::atomic<bool> status = true;
		thread_entry(unsigned int id_in) {
			id = id_in;
		}
	};
	std::vector<std::unique_ptr<thread_entry>> threads;
	spinlock t_lock;
	std::atomic<size_t> thread_amount = 0;;
public:
	void add_thread(unsigned int id);
	void sleeping(unsigned int id);
	void wake(unsigned int id);
	unsigned int alives();
	bool is_alive(unsigned int id);
};

class tl_storage {
private:
	struct tls_queue {
		spinlock queue_lock;
		std::queue<unsigned int> queue;
	};
	struct data_element {
		std::wstring string;
		std::atomic<bool> used;
		spinlock lock;
		data_element(std::wstring& string_in) {
			string = string_in;
			used = true;
		}
	};
	tls_queue unused_space;
	spinlock vec_lock;
	std::vector<data_element*> vec;
	std::atomic<size_t> vecsize = 0;
	std::atomic<unsigned int> elements = 0;
public:
	void push_back(std::wstring& string);
	std::wstring get_first();
	std::wstring get_first_del();
	std::wstring get_first_clr();
	bool empty();
	~tl_storage() {
		for (data_element* element : vec) {
			delete element;
		}
	}
};

class search {
private:
	struct xconnect {
		tl_storage files;
		tl_storage dirs;
	};
	
	xconnect store;
	
	bool is_file(const WIN32_FIND_DATA& FindFileData);
	void add_to_results(std::wstring& input);
	std::shared_ptr<std::vector<std::wstring>> results_vec;
	spinlock results;
	
	threadwatch consumers;
	threadwatch producers;

	std::vector<std::wstring> folders_from_string(std::wstring& string);
	std::wstring filename_from_string(std::wstring& string);

public:
	enum mode { list, find_combined, find_file, find_folder };
	static std::wstring singletowide(const std::string& input);
	static std::string widetosingle(const std::wstring& input);
	std::shared_ptr<std::vector<std::wstring>> start_search(mode mode_in, std::wstring& dir_input, std::wstring search_string = L"", bool exclude_windir = true);
	
private:
	void search_producer(unsigned int tandem_id, bool exclude_windir);
	void search_consumer(unsigned int tandem_id, mode mode_in, std::wstring search_string);
};