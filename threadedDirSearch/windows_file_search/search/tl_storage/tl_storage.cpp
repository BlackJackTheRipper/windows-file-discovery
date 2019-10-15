#include "../../required.h"
#include "../search.h"
#include "tl_storage.h"

int tl_storage::tls_queue::get() {
	int free = -1;
	queue_lock.acquire_strong();
	if (!queue.empty()) {
		free = queue.front();
		queue.pop();
	}
	queue_lock.release();
	return free;
}

void tl_storage::tls_queue::add(unsigned int input) {
	queue_lock.acquire_strong();
	queue.push(input);
	queue_lock.release();
}

//will return the first available element of the vector
std::wstring tl_storage::get_first() {
	std::wstring return_string;
	const unsigned int vecsize_local = vecsize;
	for (int i = 0; i < vecsize_local; i++) {
		if (vec[i] != nullptr) {
			if (vec[i]->lock.acquire_weak()) {
				return_string = vec[i]->string;
				vec[i]->lock.release();
				unused_space.add(i);
				elements -= 1;
				break;
			}
		}
	}
	return return_string;
}

//will add an element to the vector
void tl_storage::push_back(std::wstring& string) {
	elements += 1;
	const int free = unused_space.get();
	if (free != -1 && vec[free] != nullptr) {
		vec[free]->lock.acquire_strong();
		vec[free]->string = string;
		vec[free]->lock.release();
		return;
	}
	vec_lock.acquire_strong();
	vecsize += 1;
	vec.push_back(new data_element(string));
	vec_lock.release();
}

bool tl_storage::empty() const {
	if (elements == 0)
		return true;
	else
		return false;
}

tl_storage::data_element::data_element(std::wstring& string_in) {
	string = string_in;
}