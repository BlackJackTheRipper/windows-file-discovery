/*
PROGRAMM METADATA:

	name:           windows file discovery
	description:    c++ rapid file search and index
	copyright:      (c) 2019 Constantin Fürst constantin@fuersten.info

LICENSE:

   This work is created by Constantin Fürst.
   If you are unsure if your use of this project is permitted or you'd like to use it in a way prohibited by the following license, please message "constantin@fuersten.info".

   This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
   To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-nd/4.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

DISCLAIMER:

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */

#include "pch.h"

#ifndef INCLUDES
#define INCLUDES

#include "logger.h"
INITIATE_LOGGER;
#include "search_request.h"

#endif

#define CONSOLE_INDENT std::cout << "	"

search_request new_search;
std::string output_file_name;

/*
 *
 *	GENERAL FUNCTIONS (string converters, input handler and so on)
 *
 */

// convert widestring to sinlge byte
std::string widetosingle(const std::wstring &input) {
	std::wstring_convert < std::codecvt_utf8_utf16 <wchar_t> > converter;
	const std::string output = converter.to_bytes(input);
	return output;
}

// convert single byte to widestring
std::wstring singletowide(const std::string &input) {
	std::wstring_convert < std::codecvt_utf8_utf16 <wchar_t> > converter;
	const std::wstring output = converter.from_bytes(input);
	return output;
}

// checks if a directory specified exists, duh
bool isdir(const std::wstring &dirName_in) {
	const DWORD ftype = GetFileAttributesW(dirName_in.c_str());
	if (ftype == INVALID_FILE_ATTRIBUTES) {
		return FALSE;
	}
	if (ftype & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	}
	return FALSE;
}

void input_handler() {
	std::string argument;
	{
		#ifdef TDS_DEBUG
				//set level of verbose when running in debug mode
				LOG_IMPT_NOTICE("The Programm is running in debug mode");
				LOG_IMPT_NOTICE("What level of logging do you want? Choose between 'release', 'metrics' and 'detailed'");
				LOG_IMPT_NOTICE("Be aware that 'detailed' will cause low performance when searching large directories");
				while (TRUE) {
					CONSOLE_INDENT;
					getline(std::cin, argument);
					if (argument == "release") {
						log_instance.pub_verbose_level = logger::standard;
						break;
					}
					else if (argument == "metrics") {
						log_instance.pub_verbose_level = logger::metrics;
						break;
					}
					else if (argument == "detailed") {
						log_instance.pub_verbose_level = logger::detailed;
						break;
					}
					else {
						LOG_WARNING(argument + " is not a valid verbose level");
					}
				}
		#endif
	}
	{
		//getting the directory to search
		LOG_IMPT_NOTICE("Please enter a directory that the programm should search in (remember to put it in \"\" if it contains whitespaces)");
		CONSOLE_INDENT;
		getline(std::cin, argument);
		std::wstring argument_wide = singletowide(argument);
		//if the entered argument is not a directory ask again
		while (!isdir(argument_wide)) {
			LOG_WARNING("The directory you entered does not seem to exist, please try that again");
			CONSOLE_INDENT;
			getline(std::cin, argument);
			argument_wide = singletowide(argument);
		}
		new_search.pub_dir_to_search = argument_wide;
	}
	{
		//setting programm mode (search or index)
		LOG_IMPT_NOTICE("Please choose what you would like the programm to do ('search' for a file or 'index' a directory)");
		while (TRUE) {
			CONSOLE_INDENT;
			getline(std::cin, argument);
			//when index was entered
			if (argument == "index") {
				//request the name of an output file
				new_search.pub_mode = search_request::index;
				LOG_IMPT_NOTICE("Please enter an output file (relative paths are supported)");
				std::wofstream woutput_stream;
				while (TRUE) {
					CONSOLE_INDENT;
					getline(std::cin, output_file_name);
					woutput_stream.open(output_file_name, std::fstream::app);
					//check if filename is valid and programm has access rights for the location by performing an open or create of the file
					if (woutput_stream.is_open()) {
						woutput_stream.close();
						break;
					}
					else {
						LOG_WARNING(argument + " is not a valid filename or this application lacks the access rights to the folder specified, please try that again");
					}
				}
				break;
			}
			//when search was entered
			else if (argument == "search") {
				//get a string to search for
				new_search.pub_mode = search_request::file_search;
				LOG_IMPT_NOTICE("Please enter a filename or string you would like to search for");
				LOG_WARNING("Using a string that is too general will result in a large amount of results and long runtimes");
				CONSOLE_INDENT;
				getline(std::cin, argument);
				new_search.pub_search_filename = singletowide(argument);
				break;
			}
			else {
				LOG_WARNING(argument + " is not a valid mode setting, please try that again");
			}
		}
	}
	{
		//setting programm priority (ultra, high, normal and medium)
		LOG_IMPT_NOTICE("Please choose a priority setting from 'high' or 'normal' ('ultra' and 'low' are not recommended)");
		while (TRUE) {
			CONSOLE_INDENT;
			getline(std::cin, argument);
			if (argument == "ultra") {
				new_search.pub_speed_mode = search_request::ultra;
				break;
			}
			else if (argument == "high") {
				new_search.pub_speed_mode = search_request::high;
				break;
			}
			else if (argument == "normal") {
				new_search.pub_speed_mode = search_request::normal;
				break;
			}
			else if (argument == "low") {
				new_search.pub_speed_mode = search_request::low;
				break;
			}
			else {
				LOG_WARNING(argument + " is not a valid speed setting");
			}
		}
	}
}

//function that is called by the search thread and processes the output
void search_output_handler(std::vector<std::wstring> &output) {
	//when searching just print the output to console
	if (new_search.pub_mode == search_request::file_search && !output.empty()) {
		LOG_IMPT_NOTICE(L"	" + output[0]);
	}
	//when indexing:
	else if (new_search.pub_mode == search_request::index) {
		//open output stream to file
		const size_t size = output.size();
		std::wofstream woutput_stream;
		woutput_stream.open(output_file_name, std::fstream::app);
		if (woutput_stream.is_open()) {
			//for every member of the entered vector: write to output stream
			for (int i = 0; i < size; i++) {
				woutput_stream << output[i] << std::endl;
			}
			woutput_stream.close();
		}
		//throw error if output write operation failed
		else {
			LOG_ERROR("WRITING TO THE OUTPUT FILE FAILED");
		}
	}
}

//main function
int main () {
	{
		//print newline (makes the output look nicer)
		std::cout << std::endl;
		LOG_IMPT_NOTICE(L"(c) Constantin Fuerst 2019");
		//get input from the user
		input_handler();
		LOG_SUCCESS(L"STARTING TO SEARCH IN " + new_search.pub_dir_to_search);
	}

	//set the output handler function
	new_search.pub_output_handler = &search_output_handler;
	//start the search with the defined parameters
	new_search.initiate_search();

	{
		//summarize the operation
		if (new_search.pub_mode == search_request::file_search) {
			LOG_NOTICE("ended search");
		}
		else {
			//output the indexed file amount
			if (new_search.pub_filecount != 0) {
				LOG_SUCCESS(std::string("INDEXED ") + std::to_string(new_search.pub_filecount) + std::string(" FILES TO ") + output_file_name);
			}
			else {
				//warn if no file was found
				LOG_WARNING("DID NOT DISCOVER ANY FILE");
			}
		}
	}

	std::cin.get();
	return TRUE;
}