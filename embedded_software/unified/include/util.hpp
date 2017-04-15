/******************************************************************************
 * Hackerboat Beaglebone utilities
 * util.hpp
 * This is a set of generic utilites
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Apr 2017
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <string>

#define REMOVE(a) delete a; a = NULL;

class Args {
	public:
		int load(int argc, char **argv) {
			int i = 0;
			for (i = 0; i < argc; i++) {
				Args::_args->emplace_back(argv[i]);
			}
			return i;
		}
		std::list<std::string> *args() {return _args;};
		static Args* getargs() {return _instance;};
	private:
		Args () {
			_args = new std::list<std::string>;
		}								
		Args (Args const&) = delete;					/**< Hark, a singleton! */
		Args& operator=(Args const&) = delete;			/**< Hark, a singleton! */
		static Args *_instance;
		std::list<std::string> *_args;					/**< List of arguments */
};

#endif /* UTILS_H */
