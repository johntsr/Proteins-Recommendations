#include "Euclidean.h"
#include "../General/Math.h"

#include <sstream>
#include <cstdlib>
#include <limits.h>


// @override
double EuclideanPoint::distance(Point* p){
	// just follow the definition of the e-class slides
	Quantity* Array1  = value() ;					// get array of co-ordinates for this point
	Quantity* Array2  = p->value();					// as well as for p
	double distance = 0.0;							// the distance is gradually builh here
	for( int i = 0; i < dimension() ; i++){
		double diff = ( Array1[i].getDouble() - Array2[i].getDouble() );
		distance += diff * diff;	// euclidean norm
	}

	return distance;
}

// @override
double EuclideanPoint::similarity(Point* p){
	return 1.0 / ( distance(p) + 1.0  );			// compute the distance (in [0,2] )
}

// @override
PointType EuclideanPoint::type(void){
	return EUCLIDEAN;
}





// @override
double EuclideanPointSparse::distance(Point* p){
	Quantity* P_Array = p->value();
	int pLength = p->dimension();

	SparsePoint* sp = (SparsePoint*)p;
	int* pIndexes = sp->indexes();

	double dist = 0.0;
	double diff = 0.0;
	int i = 0, j = 0;
	for( ; i < Length && j < pLength; ){
		if( Indexes[i] == pIndexes[j] ){
			diff = SparseArray[i].getDouble() - P_Array[j].getDouble();
			i++;
			j++;
		}
		else if( Indexes[i] < pIndexes[j] ){
			diff = SparseArray[i].getDouble();
			i++;
		}
		else{
			diff = P_Array[j].getDouble();
			j++;
		}
		dist += diff * diff;
	}

	if( i < Length ){
		for( ; i < Length; i++ ){
			diff = SparseArray[i].getDouble();
			dist += diff * diff;
		}
	}
	else if( j < pLength){
		for( ; j < pLength; j++ ){
			diff = P_Array[j].getDouble();
			dist += diff * diff;
		}
	}

	return dist;
}

// @override
double EuclideanPointSparse::similarity(Point* p){
	return 1.0 / ( distance(p) + 1.0  );
}

// @override
PointType EuclideanPointSparse::type(void){
	return EUCLIDEAN;
}

/*****************************************************************
***************** Euclidean_h class methods **********************
******************************************************************/

Euclidean_h::Euclidean_h(int dimension, int wRand ){
	w = wRand;													// the "W" parameter, as defined by the user
	t = Math::dRand(0,w);										// the "t" parameter, randomply picked in [0,w)
	p1 = new EuclideanPointSparse( dimension );						// create a Gaussian random point-vector
}

uint64_t Euclidean_h::hash(Point* p ){
	// just follow the definition of the e-class slides
	Quantity* temp = p->multiply(p1);
	uint64_t result = (uint64_t)( ( temp->getDouble() + t) / w );
	delete temp;
	return result;
}

Euclidean_h::~Euclidean_h(){
	delete p1;
}


/*****************************************************************
***************** Euclidean_g class methods **********************
******************************************************************/

const uint64_t Euclidean_g::M = UINT32_MAX - 5;							// 2^32 - 5

Euclidean_g::Euclidean_g(int tableSize, int k, int dimension, int w){
	K = k;												// keep both the number of "h" functions
	Dimension = dimension;								// and the dimensionality of the points
	Table_h = new Euclidean_h*[K];						// allocate a table big enough for our hash functions
	r = new int[K];										// allocate a table big enough for our random parameters
	for(int i = 0; i < K; i++ ){						// initialise every function and parameter
		Table_h[i] = new Euclidean_h(dimension, w);
		r[i] = rand();
	}
}

uint64_t Euclidean_g::hash(Point* p){					// map a multi-dimensional point into an integer
	uint64_t hashValue = 0;								// variable on which the result is gradually built
	for(int i = 0; i < K; i++ ){
		hashValue += Table_h[i]->hash(p) * r[i];		// just follow the definition of the e-class slides
	}

	if( hashValue < 0.0){								// "%" is not exaclty "modulo"
		hashValue += M;									// so if negative, add "M"
	}

	return hashValue % M;								// now we return the actual "modulo"
}

Euclidean_g::~Euclidean_g(){
	for(int i = 0; i < K; i++ ){
		delete Table_h[i];
	}

	delete[] Table_h;
	delete[] r;
}
