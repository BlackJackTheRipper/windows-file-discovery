#include "pch.h"
#include "search/search.h"

bool ws_vector_buffer::empty() {
	bool empty_write = false;
	bool empty_read = false;
	writelock.acquire_strong();
	empty_write = write_vector.empty();
	writelock.release();
	readlock.acquire_strong();
	empty_read = read_vector.empty();
	readlock.release();
	if (empty_read == empty_write == true)
		return true;
	else
		return false;
}
	
void ws_vector_buffer::add(std::wstring input) {
	const std::shared_ptr<std::wstring> string_uptr = std::make_shared<std::wstring>(input);
	writelock.acquire_strong();
	write_vector.push_back(string_uptr);
	writelock.release();
}

std::wstring ws_vector_buffer::get() {
	std::shared_ptr<std::wstring> string_uptr = nullptr;
	readlock.acquire_strong();
	if (!read_vector.empty()) {
		string_uptr = read_vector.back();
		read_vector.pop_back();
	}
	readlock.release();
	if (string_uptr == nullptr) {
		writelock.acquire_strong();
		if (!write_vector.empty()) {
			string_uptr = write_vector.back();
			write_vector.pop_back();
		}
		writelock.release();
	}
	if (string_uptr == nullptr)
		return L"";
	return *string_uptr;
}

void ws_vector_buffer::transfer_buffer() {
	std::shared_ptr<std::wstring> current = nullptr;
	{
		if (writelock.acquire_weak()) {
			if (!write_vector.empty()) {
				current = write_vector.back();
				write_vector.pop_back();
			}
			writelock.release();
		}
	}
	{
		if (current != nullptr) {
			if (readlock.acquire_weak()) {
				read_vector.push_back(current);
				readlock.release();
			}
		}
	}
}

void search::transfer_buffers() {
	std::wstring current;
	bool quit = false;
	while (TRUE) {
		for (int i = 0; i < 11; i++) {
			{
				files_buffer.transfer_buffer();
				current = files_buffer.get();
				if (!current.empty()) {
					if (files_queue.fill_first(current) == false) {
						files_buffer.add(current);
					}
				}
			}
			{
				dir_buffer.transfer_buffer();
				current = dir_buffer.get();
				if (!current.empty()) {
					if (dir_queue.fill_first(current) == false) {
						dir_buffer.add(current);
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			current = L"";
		}
		if (files_buffer.empty() && dir_buffer.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (files_buffer.empty() && dir_buffer.empty()) {
				while (TRUE) {
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					if (!files_buffer.empty() && !dir_buffer.empty()) {
						break;
					}
					if (dead_consumers >= cons_threadlimit && dead_producers >= prod_threadlimit) {
						quit = true;
						break;
					}
				}
			}
		}
		if (quit == true)
			break;
	}
}