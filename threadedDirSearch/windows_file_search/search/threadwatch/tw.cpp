#include "../../required.h"
#include "../search.h"

void threadwatch::add_thread(unsigned int id) {
	t_lock.acquire_strong();
	threads.push_back(std::make_unique<thread_entry>(id));
	t_lock.release();
	thread_amount += 1;
}

void threadwatch::sleeping(unsigned int id) {
	for (size_t i = 0; i < thread_amount; i++) {
		if (threads[i]->id == id) {
			bool expected = ALIVE;
			threads[i]->status.compare_exchange_strong(expected, DEAD);
			return;
		}
	}
}

void threadwatch::wake(unsigned int id) {
	for (size_t i = 0; i < thread_amount; i++) {
		if (threads[i]->id == id) {
			bool expected = DEAD;
			threads[i]->status.compare_exchange_strong(expected, ALIVE);
			return;
		}
	}
}

unsigned int threadwatch::alives() {
	unsigned int alives = 0;
	for (size_t i = 0; i < thread_amount; i++) {
		if (threads[i]->status == ALIVE)
			alives++;
	}
	return alives;
}

bool threadwatch::is_alive(unsigned int id) {
	for (size_t i = 0; i < thread_amount; i++) {
		if (threads[i]->id == id)
			if (threads[i]->status == ALIVE)
				return ALIVE;
			else
				return DEAD;
	}
	return DEAD;
}