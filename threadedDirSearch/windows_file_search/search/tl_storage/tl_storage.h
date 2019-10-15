#pragma once
#include "../../required.h"

class tl_storage {
private:
	struct tls_queue {
	private:
		spinlock queue_lock;
		std::queue<unsigned int> queue;
	public:
		int get();
		void add(unsigned int input);
	};
	
	struct data_element {
		std::wstring string;
		spinlock lock;
		data_element(std::wstring& string_in);
	};
	
	tls_queue unused_space;
	spinlock vec_lock;
	std::vector<data_element*> vec;
	std::atomic<size_t> vecsize = 0;
	std::atomic<unsigned int> elements = 0;
	
public:
	void push_back(std::wstring& string);
	std::wstring get_first();
	bool empty() const;
	~tl_storage() {
		for (data_element* element : vec) {
			delete element;
		}
	}
};