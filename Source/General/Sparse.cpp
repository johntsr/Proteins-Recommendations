#include "Sparse.h"
#include "Math.h"
#include <float.h>
#include <sstream>
#include <iostream>


/*****************************************************************
***************** SparsePoint class methods **********************
******************************************************************/

SparsePoint::SparsePoint(std::string name, List<Pair>* ratingList){
	Point::Name = name;										// store the name
	Length = ratingList->count();

	SparseArray = new Quantity[Length];
	Indexes = new int[Length];

	int i = 0;
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next(), i++ ) {
		SparseArray[i].setDouble(node->data()->Rating);
		Indexes[i] = node->data()->Item;
	}
}

SparsePoint::SparsePoint(int dimension){
	Point::Name = "Random Sparse aussian Vector";

	Length = dimension;

	SparseArray = new Quantity[Length];
	Indexes = new int[Length];

	for(int i = 0; i < Length; i++){				// for every co-ordinate
		Indexes[i] = i;
		SparseArray[i] = Quantity( Math::getGaussian() );	// get a random Gaussian variable
	}
}

// @override
std::string SparsePoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int SparsePoint::dimension(void) {				// the dimensionality of the point
	return Length;
}

// @override
Quantity* SparsePoint::value(void){						// maps a point into a non-negative integer value
	return SparseArray;
}

// @override
Quantity* SparsePoint::multiply (Point* p){				// defines the multiplication between two points

	Quantity* P_Array = p->value();
	int pLength = p->dimension();

	SparsePoint* sp = (SparsePoint*)p;
	int* pIndexes = sp->Indexes;

	// std::cout << this << '\n';
	// // std::cout << "Length = " << Length << " " << name() << std::endl;
	// for(int i = 0; i < Length; i++){
	// 	std::cout << "item = " << Indexes[i] << " - rating = " << SparseArray[i].getDouble() << std::endl;
	// }


	// std::cout << sp << '\n';
	// std::cout << "pLength = " << pLength << " " << name() << std::endl;
	// for(int i = 0; i < pLength; i++){
	// 	std::cout << "item = " << pIndexes[i] << " - rating = " << P_Array[i].getDouble() << std::endl;
	// }


	double innerProduct = 0;
	for(int i = 0, j = 0; i < Length && j < pLength; ){
		if( Indexes[i] == pIndexes[j] ){
			innerProduct +=  SparseArray[i].getDouble() * P_Array[j].getDouble();
			i++;
			j++;
		}
		else if( Indexes[i] < pIndexes[j] ){
			i++;
		}
		else{
			j++;
		}
	}
	return new Quantity(innerProduct);
}

// @override
bool SparsePoint::operator == (Point* p){				// defines the equality of 2 points

	if( dimension() != p->dimension() ){
		return false;
	}

	Quantity* P_Array = p->value();

	SparsePoint* sp = (SparsePoint*)p;
	int* pIndexes = sp->Indexes;

	for(int i = 0; i < Length; i++){
		if( Indexes[i] != pIndexes[i] ||
			SparseArray[i].getDouble() != P_Array[i].getDouble() ){
			return false;
		}
	}

	return true;										// all equal if I reach here
}

// @override
void SparsePoint::print(void){
	// Quantity* Array   = value();

	// for(int i = 0; i < dimension(); i++){
	// 	std::cout << Array[i].getString() << " ";
	// }
	std::cout << name() << std::endl;
}

SparsePoint::~SparsePoint(){
	delete[] SparseArray;
	delete[] Indexes;
}
