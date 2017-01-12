#include "Hamming.h"
#include "../General/Math.h"

#include <bitset>
#include <limits.h>

#define RATING_BOUND 1

/*****************************************************************
***************** HammingPoint class methods *********************
******************************************************************/

HammingPoint::HammingPoint(std::string name, std::string value) {
	Point::Name = name;
	Value = new Bitset(value);
}

HammingPoint::HammingPoint(std::string name, Bitset* bits){
	Point::Name = name;
	Value = bits;
}

HammingPoint::HammingPoint(std::string name, List<Pair>* ratingList, int dimension){
	Point::Name = name;
	Value = new Bitset(dimension);

	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		if( node->data()->Rating >= RATING_BOUND ){
			Value->set( node->data()->Item );
		}
	}
}

// @override
std::string HammingPoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int HammingPoint::dimension(void) {				// the dimensionality of the point
	return Value->dimension();						// the number of bits a hamming point has
}

// @override
Quantity* HammingPoint::value(void){			// maps a point into a non-negative integer value
	return new Quantity(Value, false);			// simply the bitstring itself
}

// @override
Quantity* HammingPoint::multiply (Point* p){		// defines the multiplication between two points
	// simply a bitwise AND
	Quantity* temp1 = value();
	Quantity* temp2 = p->value();
	Bitset* mul = (*temp1->getBits()) & (*temp2->getBits());

	delete temp1;
	delete temp2;

	return new Quantity(mul, true);
}

// @override
double HammingPoint::distance(Point* p){
	Quantity* temp1 = value();
	Quantity* temp2 = p->value();
	Bitset* dist = (*temp1->getBits()) ^ (*temp2->getBits());
	int count = dist->count();
	delete temp1;
	delete temp2;
	delete dist;

	return count * 1.0;
}

// @override
double HammingPoint::similarity(Point* p){
	return dimension() - distance(p);
}

// @override
bool HammingPoint::operator == (Point* p){
	if( dimension() != p->dimension() ){
		return false;
	}

	return !( distance(p) > 0.0 );
}

// @override
void HammingPoint::print(void){
	std::cout 	<<  value()->getBits()->getString()
				<< "\t(" << name() << ")"  << std::endl;
}

HammingPoint::~HammingPoint(){
	delete Value;
}

// @override
PointType HammingPoint::type(void){
	return HAMMING;
}

/*****************************************************************
***************** Hamming_h class methods ************************
******************************************************************/

#define MASKS 60

Hamming_h::Hamming_h(int length){						// length of the input bitstring
	Bitset* temp = new Bitset(length);
	for(int i = 0; i < MASKS; i++){
		int position = length * Math::dRand();			// randomply pick a bit to "peek"
		temp->set(position);
	}
	Mask = new HammingPoint( "mask", temp );			// create a point out of this mask
}

uint64_t Hamming_h::hash(Point* p ){					// map a multi-dimensional hamming point into an integer
	Quantity* temp = p->multiply(Mask);
	bool nonZero = temp->getBits()->count() > 0;		// bitwise AND between point and mask => bit exrtaction
	delete temp;

	if ( nonZero ){										// non negative result means the bit was 1
		return 1;
	}
	else{												// if the result in 0, then the bit was 0
		return 0;
	}
}

Hamming_h::~Hamming_h(){
	delete Mask;
}
