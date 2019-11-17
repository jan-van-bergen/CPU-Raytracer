#pragma once

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

struct StringCompare {
	inline bool operator()(const char * a, const char * b) const {
		return strcmp(a, b) == 0;
	}
};
