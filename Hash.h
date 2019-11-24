#pragma once

// Overloads function application operator such that a
// char pointer is hashed based on the string it points to
struct StringHash {
	inline size_t operator()(const char * string) const {
		// djb2 hash, based on: http://www.cse.yorku.ca/~oz/hash.html
		size_t hash = 5381;
		int c;

		while (c = *string++) {
			hash = ((hash << 5) + hash) + c;
		}

		return hash;
	}
};

// Overloads function application operator such that 
// different char pointers pointing to different memory
// are considered equal if the strings they point to are equal
struct StringCompare {
	inline bool operator()(const char * a, const char * b) const {
		return strcmp(a, b) == 0;
	}
};
