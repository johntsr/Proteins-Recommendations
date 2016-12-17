#ifndef __EUCLIDEAN__
#define __EUCLIDEAN__

#include "../General/HashFunction.h"
#include "../General/Vector.h"

// EuclideanPoint : just a VectorPoint with a unique definition of distance
class EuclideanPoint: public VectorPoint {
	public:

		EuclideanPoint(std::string Name, std::string& stringLine, int length):
			VectorPoint(Name, stringLine, length){}

		EuclideanPoint( int dimension ): VectorPoint(dimension){}

		Quantity* 	distance	(Point* p);				// @override
		PointType	type		(void);					// @override
};



// a hamming hash function
// represents the "h" hash function of the e-class slides
// i.e. it just "extracts" a bit of a specific position
class Euclidean_h: public hash_function{
	private:
		double t;
		int w;
		Point* p1;

	public:

		Euclidean_h(int dimension, int wRand );

		uint64_t hash(Point* p );				// @override: map a multi-dimensional point into an integer

		~Euclidean_h();
};

// a hamming hash function
// represents the "h" hash function of the e-class slides
// i.e. it combines the results of "h" functions above to produce the final hash-result
class Euclidean_g: public hash_function{
	private:
		const static uint64_t M;
		Euclidean_h** Table_h;				// table of "h" functions, the results of which will be combined
		int* r;								// random variables, used to lineary combined the outputs of the functions above
		int K;								// the size of the above table, i.e. the number of "h" I use
		int Dimension;						// dimensionality of the points I hash
	public:

		Euclidean_g(int tableSize, int k, int dimension, int w);

		uint64_t hash(Point* p);					// @override: map a multi-dimensional point into an integer

		~Euclidean_g();
};

#endif
