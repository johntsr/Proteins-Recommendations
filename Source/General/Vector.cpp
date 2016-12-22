#include "Vector.h"
#include "Math.h"
#include <float.h>
#include <sstream>
#include <iostream>


/*****************************************************************
***************** VectorPoint class methods **********************
******************************************************************/

VectorPoint::VectorPoint(std::string name, std::string& stringLine, int dimension) {
	Point::Name = name;										// store the name

	Dimension = dimension;									// store the dimensionality of the vector, i.e. the length of the array of the co-ordinates
	Array = new Quantity[Dimension];						// allocate the above array

	std::istringstream streamLine(stringLine);				// convert the string in an easily parsed form
	std::string temp;										// temporary variable for extracting data from the above "string"

	for(int i = 0; i < Dimension; i++){						// for every co-ordinate
		streamLine >> temp;									// get its value as string
		Array[i] = Quantity( strtod( temp.c_str(), NULL ) );// and convert it to double
	}
}

VectorPoint::VectorPoint(std::string name, double* coordinates, int dimension){
	Point::Name = name;										// store the name

	Dimension = dimension;									// store the dimensionality of the vector, i.e. the length of the array of the co-ordinates
	Array = new Quantity[Dimension];						// allocate the above array

	for(int i = 0; i < Dimension; i++){						// for every co-ordinate
		Array[i] = Quantity( coordinates[i] );// and convert it to double
	}
}

VectorPoint::VectorPoint(int dimension){
	Point::Name = "Random Gaussian Vector";
	Dimension 	= dimension;						// store the dimensionality of the vector, i.e. the length of the array of the co-ordinates
	Array = new Quantity[Dimension];				// allocate the above array

	for(int i = 0; i < Dimension; i++){				// for every co-ordinate
		Array[i] = Quantity( Math::getGaussian() );	// get a random Gaussian variable
	}
}

// @override
std::string VectorPoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int VectorPoint::dimension(void) {				// the dimensionality of the point
	return Dimension;
}

// @override
Quantity* VectorPoint::value(void){						// maps a point into a non-negative integer value
	return Array;
}

// @override
Quantity* VectorPoint::multiply (Point* p){				// defines the multiplication between two points
	Quantity* Array   = value();						// get the address of the table of co-ordinates
	Quantity* P_Array = p->value();						// for both Points

	double innerProduct = 0;
	for(int i = 0; i < dimension(); i++){				// iterate over the arrays
		// mathematical definition
		innerProduct += Array[i].getDouble() * P_Array[i].getDouble();
	}

	return new Quantity(innerProduct);					// "parse" the result to 64 bits
}

// @override
bool VectorPoint::operator == (Point* p){				// defines the equality of 2 points
	Quantity* Array   = value() ;						// get the address of the table of co-ordinates
	Quantity* P_Array = p->value();						// for both Points

	if( dimension() != p->dimension() ){
		return false;
	}

	for(int i = 0; i < dimension(); i++){
		if( Array[i].getDouble() != P_Array[i].getDouble() )// if just a co-ordinate is different
			return false;								// then the points are different
	}

	return true;										// all equal if I reach here
}

// @override
void VectorPoint::print(void){
	// Quantity* Array   = value();

	// for(int i = 0; i < dimension(); i++){
	// 	std::cout << Array[i].getString() << " ";
	// }
	std::cout << name() << std::endl;
}

// @override
Quantity* VectorPoint::maxDistance(void){			// maximum possible distanve between 2 points
	return new Quantity( (double)DBL_MAX);			// from <float.h>, the maximum value a double may have ("parsed" in 64 bits)
}

// @override
bool VectorPoint::inRange(Point* p, Quantity* R){
	Quantity* temp = distance( p );
	bool result = temp->getDouble() < Point::C * R->getDouble();	// p is in range, if our distance in smaller than R
	delete temp;
	return result;
}


VectorPoint::~VectorPoint(){
	delete[] Array;
}
