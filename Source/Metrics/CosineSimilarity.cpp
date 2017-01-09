#include "CosineSimilarity.h"
#include "../General/Math.h"

#include <sstream>
#include <cstdlib>
#include <limits.h>


// @override
double CosinePoint::distance(Point* p){
	// just follow the definition of the e-class slides
	Quantity* temp;

	temp = multiply(p);
	double product = temp->getDouble();						// inner product
	delete temp;

	temp = multiply(this);
	double magnitude1 = sqrt( temp->getDouble() );			// norm(x) = inner_product(x,x)
	delete temp;

	temp = p->multiply(p);
	double magnitude2 = sqrt( temp->getDouble() );			// same here
	delete temp;

	return 1.0 - ( product / ( magnitude1 * magnitude2 ) );	// compute the distance (in [0,2] )
}

// @override
double CosinePoint::similarity(Point* p){
	return 1 - distance(p);
}

// @override
PointType CosinePoint::type(void){
	return COSINE;
}




// @override
double CosinePointSparse::distance(Point* p){
	// just follow the definition of the e-class slides
	Quantity* temp;

	temp = multiply(p);
	double product = temp->getDouble();						// inner product
	delete temp;

	temp = multiply(this);
	double magnitude1 = sqrt( temp->getDouble() );			// norm(x) = inner_product(x,x)
	delete temp;

	temp = p->multiply(p);
	double magnitude2 = sqrt( temp->getDouble() );			// same here
	delete temp;

	return 1.0 - ( product / ( magnitude1 * magnitude2 ) );	// compute the distance (in [0,2] )
}

// @override
double CosinePointSparse::similarity(Point* p){
	return 1 - distance(p);
}

// @override
PointType CosinePointSparse::type(void){
	return COSINE;
}



/*****************************************************************
***************** Cosine_h class methods *************************
******************************************************************/

Cosine_h::Cosine_h(int dimension){
	// p1 = new CosinePoint( dimension );					// create a Gaussian random point-vector
	p1 = new CosinePointSparse( dimension );				// create a Gaussian random point-vector
}

uint64_t Cosine_h::hash(Point* p ){						// map a multi-dimensional hamming point into an integer
	// just follow the definition of the e-class slides
	Quantity* temp = p->multiply(p1);
	bool result = temp->getDouble() < 0.0;
	delete temp;
	if( result ){
		return 0;
	}
	else{
		return 1;
	}
}

Cosine_h::~Cosine_h(){
	delete p1;
}
