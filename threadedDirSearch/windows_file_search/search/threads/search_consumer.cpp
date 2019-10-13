#include "../../required.h"
#include "../search.h"

void search::add_to_results(std::wstring& input) {
	if (results_vec != nullptr) {
		results_vec->add(input);
	}
}

void search::search_consumer(unsigned int tandem_id, mode mode_in, std::wstring search_string) {
	bool quit = false;
	while (TRUE) {
		for (int i = 0; i < 11; i++) {
			if (mode_in != list) {
				std::wstring current = store.files.get_first_clr();
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
					}
				}
				else if (mode_in == find_folder) {
					for (std::wstring folder : folders_from_string(current)) {
						if (folder.find(search_string) != std::wstring::npos) {
							add_to_results(current);
						}
					}
				}
				else {
					add_to_results(current);
				}
			}
		}
		if (store.files.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (store.files.empty()) {
				consumers.sleeping(tandem_id);
				while (TRUE) {
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					if (!store.files.empty()) {
						consumers.wake(tandem_id);
						break;
					}
					if (consumers.alives() == 0 && producers.alives() == 0) {
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
