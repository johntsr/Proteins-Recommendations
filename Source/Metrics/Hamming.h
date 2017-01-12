#ifndef __HAMMING__
#define __HAMMING__

#include "../DataStructures/DoubleList.h"
#include "../General/Point.h"
#include "../General/HashFunction.h"
#include "../General/Quantity.h"

// an actual point of the hamming space
class HammingPoint: public Point {

	private:
		Bitset* Value;			// the co-ordinates of a point can be stored in 64 bits

	public:
		HammingPoint(std::string name, std::string value);
		HammingPoint(std::string name, List<Pair>* ratingList, int dimension);
		HammingPoint(std::string name, Bitset* bits);

		std::string name		(void);		// @override: the human-readable representation of the point
		int 		dimension	(void);		// @override: the dimensionality of the point
		Quantity* 	multiply 	(Point* p);	// @override: defines the multiplication between two points
		Quantity* 	value		(void);		// @override: maps a point into a non-negative integer value
		double 		distance	(Point* p);	// @override
		double 		similarity	(Point* p);	// @override
		bool 		operator ==	(Point*); 	// @override
		void 		print  		(void);		// @override
		PointType	type		(void);		// @override
		~HammingPoint();
};



// a hamming hash function
// represents the "h" hash function of the e-class slides
// i.e. it just "extracts" a bit of a specific position

class Hamming_h: public hash_function{
	private:
		Point* Mask;							// mask used to extract the above specific bit

	public:

		Hamming_h(int length);

		uint64_t hash(Point* p );				// @override: map a multi-dimensional hamming point into an integer

		~Hamming_h();
};


class Hamming_g: public BitString_g{
	public:

		Hamming_g(int k, int bitLength): BitString_g(k) {
			for(int i = 0; i < K; i++ ){						// initialise every function
				Table_h[i] = new Hamming_h(bitLength);			// i.e. specify which bit to extract
			}
		}

		~Hamming_g(){
			for(int i = 0; i < K; i++ ){
				delete Table_h[i];
			}
		}

};

#endif
