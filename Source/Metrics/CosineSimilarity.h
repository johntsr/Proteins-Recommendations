#ifndef __COSINE__
#define __COSINE__


#include "../General/HashFunction.h"
#include "../General/Vector.h"

// CosinePoint : just a VectorPoint with a unique definition of distance
class CosinePoint: public VectorPoint {
	public:

		CosinePoint(std::string Name, std::string& stringLine, int length):
				VectorPoint(Name, stringLine, length){}

		CosinePoint(std::string Name, double* coordinates, int length):
				VectorPoint(Name, coordinates, length){}

		CosinePoint(int dimension): VectorPoint(dimension){}

		Quantity* 	distance	(Point* p);				// @override
		Quantity* 	similarity	(Point* p);				// @override
		PointType	type		(void);					// @override
};



// a cosine hash function
// represents the "h" hash function of the e-class slides
// i.e. it just returns 1 or 0, whether aninnder product with the point given is > 0 or < 0
class Cosine_h: public hash_function{
	private:
		Point* p1;

	public:

		Cosine_h(int dimension);

		uint64_t hash(Point* p );				// @override: map a multi-dimensional point into an integer

		~Cosine_h();
};


class Cosine_g: public BitString_g{
	public:

		Cosine_g(int k, int dimension): BitString_g(k){
			for(int i = 0; i < K; i++ ){						// initialise every function
				Table_h[i] = new Cosine_h(dimension);			// i.e. specify which bit to extract
			}
		}

		~Cosine_g(){
			for(int i = 0; i < K; i++ ){
				delete Table_h[i];
			}
		}

};

#endif
