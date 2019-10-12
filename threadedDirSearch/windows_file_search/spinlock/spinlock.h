#pragma once
#include "../required.h"

class spinlock {
private:
	static const bool UNLOCKED = false;
	static const bool LOCKED = true;
	std::atomic<bool> lock = false;
public:
	void acquire_strong();
	bool acquire_weak();
	void release();
};
