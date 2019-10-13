#pragma once
#include "../../required.h"
#include "../../spinlock/spinlock.h"

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