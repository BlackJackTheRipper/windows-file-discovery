#pragma once
#include "pch.h"

class spinlock {
private:
	const bool UNLOCKED = false;
	const bool LOCKED = true;
	std::atomic<bool> lock = false;
public:
	void acquire_strong();
	bool acquire_weak();
	void release();
};
