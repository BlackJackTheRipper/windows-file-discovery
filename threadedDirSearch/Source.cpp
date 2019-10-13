#include "example/example.h"

int main() {
	std::string dir;
	std::string search;
	
	std::cout << "[+] Welcome to rapid windows file search (c) Constantin Fuerst" << std::endl;
	std::cout << "[!] Please enter a directory the program should search in: ";
	std::getline(std::cin, dir);
	std::cout << "[!] Please enter a name the program should search for: ";
	std::getline(std::cin, search);
	
	example_use file_querry (search::singletowide(dir), search::singletowide(search));
	file_querry.search();

	std::cout << "[+] quit by pressing any key";
	std::cin.get();
	return 1;
}