#include "pch.h"
#include "../search.h"

void search::write_to_file(std::wstring& ofilename, std::array<std::wstring, write_buffer_size>& write_buffer) {
	filewrite.acquire_strong();
	std::wofstream woutput_stream;
	woutput_stream.open(ofilename, std::fstream::app);
	if (woutput_stream.is_open()) {
		//for every member of the write buffer: write to output stream
		for (int i = 0; i < write_buffer_size; i++) {
			woutput_stream << write_buffer[i] << std::endl;
		}
		woutput_stream.close();
	}
	filewrite.release();
}

void search::add_to_results(std::wstring& input) {
	results.acquire_strong();
	results_vec.push_back(input);
	results.release();
}

void search::search_consumer(mode mode_in, std::wstring search_string) {
	bool quit = false;
	while (TRUE) {
		for (int i = 0; i < 11; i++) {
			if (mode_in != list) {
				std::wstring current = files_queue.get_first();
				if (current.empty())
					break;
				if (mode_in == find_combined) {
					if (current.find(search_string) != std::wstring::npos) {
						add_to_results(current);
					}
				}
				else if (mode_in == find_file) {
					std::wstring filename = filename_from_string(current);
					if (filename.find(search_string) != std::wstring::npos) {
						add_to_results(current);
						logger::get().log_threaded_strong(logger::msg, "FOUND: " + widetosingle(current));
					}
				}
				else if (mode_in == find_folder) {
					for (std::wstring folder : folders_from_string(current)) {
						if (folder.find(search_string) != std::wstring::npos) {
							add_to_results(current);
						}
					}
				}
				else return;
			}
			else {
				std::array<std::wstring, write_buffer_size> write_buffer;
				for (int i = 0; i < write_buffer_size; i++) {
					std::wstring current = files_queue.get_first();
					if (current.empty())
						break;
					write_buffer[i] = current;
				}
			}
		}
		if (files_queue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (files_queue.empty()) {
				dead_consumers++;
				while (TRUE) {
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					if (!files_queue.empty()) {
						dead_consumers--;
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

std::vector<std::wstring> search::folders_from_string(std::wstring& string) {
	std::vector<std::wstring> returnvec;
	unsigned int lastpos = 0;
	for (int i = string.size(); i > 0; i--) {
		if (string.at(i) == L'\\') {
			unsigned int posfromback = string.size() - 1 - i;
			std::wstring filename = string.substr(string.size() - posfromback, string.size() - 1 - lastpos);
			returnvec.push_back(filename);
			lastpos = posfromback;
		}
	}
	returnvec.erase(returnvec.begin());
	return returnvec;
}

std::wstring search::filename_from_string(std::wstring& string) {
	unsigned int posfromback = 0;
	for (int i = string.size() - 1; i > 0; i--) {
		const wchar_t current_char = string.at(i);
		if (current_char == L'\\') {
			posfromback = string.size() - 1 - i;
			break;
		}
	}
	std::wstring filename = string.substr(string.size() - posfromback, string.size() - 1);
	return filename;
}
