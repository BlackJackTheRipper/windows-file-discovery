#include "../../required.h"
#include "../search.h"

//will return the first available element of the vector
std::wstring tl_storage::get_first() {
	std::wstring return_string;
	const unsigned int vecsize_local = vecsize;
	for (int i = 0; i < vecsize_local; i++) {
		if (vec[i]->lock.acquire_weak()) {
			return_string = vec[i]->string;
			vec[i]->lock.release();
			return return_string;
		}
	}
	return return_string;
}

//will return the first available element of the vector and erase this element (vector lock, inefficient)
std::wstring tl_storage::get_first_del() {
	std::wstring return_string;
	const unsigned int vecsize_local = vecsize;
	for (int i = 0; i < vecsize_local; i++) {
		if (vec[i]->lock.acquire_weak()) {
			return_string = vec[i]->string;
			delete vec[i];
			vec.erase(vec.begin() + i);
			elements -= 1;
			break;
		}
	}
	return return_string;
}

//will return the first available element of the vector and clear this element (enabeling rewrite, efficient)
std::wstring tl_storage::get_first_clr() {
	std::wstring return_string;
	const unsigned int vecsize_local = vecsize;
	for (int i = 0; i < vecsize_local; i++) {
		if (vec[i] != nullptr) {
			if (vec[i]->lock.acquire_weak()) {
				if (vec[i]->used == true) {
					return_string = vec[i]->string;
					vec[i]->string = L"";
					vec[i]->used.store(false);
					vec[i]->lock.release();
					unused_space.queue_lock.acquire_strong();
					unused_space.queue.push(i);
					unused_space.queue_lock.release();
					elements -= 1;
					break;
				}
				vec[i]->lock.release();
			}
		}
	}
	return return_string;
}

//will add an element to the vector
void tl_storage::push_back(std::wstring& string) {
	elements += 1;
	unused_space.queue_lock.acquire_strong();
	if (!unused_space.queue.empty()) {
		const unsigned int free = unused_space.queue.front();
		unused_space.queue.pop();
		unused_space.queue_lock.release();
		if (vec[free] != nullptr) {
			if (vec[free]->used == false) {
				vec[free]->lock.acquire_strong();
				vec[free]->string = string;
				vec[free]->used.store(true);
				vec[free]->lock.release();
				return;
			}
		}
	}
	unused_space.queue_lock.release();
	vec_lock.acquire_strong();
	vecsize += 1;
	vec.push_back(new data_element(string));
	vec_lock.release();
}

bool tl_storage::empty() {
	if (elements == 0)
		return true;
	else
		return false;
}