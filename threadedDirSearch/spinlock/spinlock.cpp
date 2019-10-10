#include "pch.h"
#include "spinlock.h"

void spinlock::acquire_strong() {
	while (true) {
		bool expected = UNLOCKED;
		if (lock.compare_exchange_strong(expected, LOCKED))
			return;
	}
}

bool spinlock::acquire_weak() {
	bool expected = UNLOCKED;
	return lock.compare_exchange_strong(expected, LOCKED);
}

void spinlock::release() {
	lock.store(UNLOCKED);
}