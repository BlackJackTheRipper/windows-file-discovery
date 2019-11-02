#pragma once
#include "../../required.h"

template<typename T>
class slist {
private:
	struct element {
		std::shared_ptr<element> next = nullptr;
		T data;
		element(T in) {
			data = in;
		}
	};
	std::shared_ptr<element> head = nullptr;

public:
	void push(T& in);
	T pop();
	T* get(unsigned int index);
	[[nodiscard]] bool empty() const;
};