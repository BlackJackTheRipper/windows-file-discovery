#include "pch.h"
#include "search/search.h"

int main() {
	std::wstring search_dir = L"C:\\Projects";
	std::wstring search_str = L"README.md";
	search querry;
	querry.init_search(search::find_file, search_dir, search_str);
}