#include "../../required.h"
#include "slist.h"

template <typename T>
T slist<T>::pop() {
	auto p = std::atomic_load(&head);
	while (p && !std::atomic_compare_exchange_weak(&head, &p, p->next)) {}
	if (p != nullptr)
		return p->string;
	return T();
}

template<typename T>
T* slist<T>::get(unsigned int index) {
	auto p = std::atomic_load(&head);
	for (unsigned int i = 0; i < index; i++) {
		if (p == nullptr)
			return nullptr;
		p = std::atomic_load(p->next);
	}
	return std::atomic_load(p->next);
}

template <typename T>
void slist<T>::push(T& in) {
	auto p = std::make_shared<element>(in);
	p->next = head;
	while (!std::atomic_compare_exchange_weak(&head, &p->next, p)){}
}

template <typename T>
bool slist<T>::empty() const {
	const auto p = std::atomic_load(&head);
	if (p == nullptr)
		return true;
	return false;
}