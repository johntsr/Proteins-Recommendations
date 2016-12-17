#include "HashFunction.h"

/*****************************************************************
***************** Hamming_g class methods ************************
******************************************************************/

BitString_g::BitString_g(int k){
	K = k;												// keep the number of "h" functions
	Table_h = new hash_function*[K];					// allocate a table big enough for our hash functions
}

// @override
uint64_t BitString_g::hash(Point* p){					// map a multi-dimensional hamming point into an integer
	uint64_t hashValue = 0;
	for(int i = 0; i < K; i++ ){						// for every hash function
		hashValue <<= 1;								// make space for its output
		hashValue += Table_h[i]->hash(p);				// and then actually store it
	}

	return hashValue;
}

BitString_g::~BitString_g(){
	delete[] Table_h;
}
