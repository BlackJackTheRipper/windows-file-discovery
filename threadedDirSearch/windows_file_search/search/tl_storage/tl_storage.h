#pragma once
#include "../../required.h"

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