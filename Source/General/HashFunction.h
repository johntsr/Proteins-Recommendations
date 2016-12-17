#ifndef __HASH_FUNCTION__
#define __HASH_FUNCTION__

#include "Point.h"
#include <iostream>
#include <string.h>


// abstract class that describes the functionality of a hash function
class hash_function{
	public:

	// a hash function simply hashes a Point object into an integer value
	virtual uint64_t hash(Point*)=0;
	virtual ~hash_function(){}
};

// general class for defining "combining" hash functions
// i.e. a hash function that concatentates the binary outputs of other hash functions
class BitString_g: public hash_function{
	protected:
		hash_function** Table_h;			// table of "h" functions, the results of which will be combined
		int K;								// the size of the above table, i.e. the number of "h" I use
	public:

		BitString_g(int k);

		uint64_t hash(Point* p);			// @override: map a multi-dimensional point into an integer

		~BitString_g();
};


#endif
