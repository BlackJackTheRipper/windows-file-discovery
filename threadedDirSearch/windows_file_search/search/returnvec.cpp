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