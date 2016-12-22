#include "Hamming.h"
#include "../General/Math.h"

#include <bitset>
#include <limits.h>


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

// @override
std::string HammingPoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int HammingPoint::dimension(void) {				// the dimensionality of the point
	return Value->size();							// the number of bits a hamming point has
}

// @override
Quantity* HammingPoint::value(void){			// maps a point into a non-negative integer value
	return new Quantity(Value, false);					// simply the bitstring itself
}

// @override
Quantity* HammingPoint::multiply (Point* p){		// defines the multiplication between two points
	// simply a bitwise AND
	Quantity* temp1 = value();
	Quantity* temp2 = p->value();
	Bitset* mul = (*temp1->getBits()) & (*temp2->getBits());

	delete temp1;
	delete temp2;

	return new QuantityBit(mul, true);
}

// @override
Quantity* HammingPoint::distance(Point* p){
	Quantity* temp1 = value();
	Quantity* temp2 = p->value();
	Bitset* dist = (*temp1->getBits()) ^ (*temp2->getBits());

	delete temp1;
	delete temp2;

	return new QuantityBit(dist, true);
}

// @override
Quantity* HammingPoint::similarity(Point* p){
	Quantity* temp = distance(p);
	temp->setDouble( dimension() - temp->castAsDouble() );
	return temp;
}

// @override
bool HammingPoint::operator == (Point* p){
	if( dimension() != p->dimension() ){
		return false;
	}

	Quantity* dist = distance(p);
	bool ret = !( dist->castAsDouble() > 0.0 );
	delete dist;
	return ret;
}

// @override
void HammingPoint::print(void){
	std::cout 	<<  value()->getBits()->getString()
				<< "\t(" << name() << ")"  << std::endl;
}

// @override
bool HammingPoint::inRange(Point* p, Quantity* R){
	Quantity* dist = distance( p );
	bool result = dist->castAsDouble() < ( (Point::C) * R->castAsDouble());
	delete dist;			// p is in range, if our distance in smaller than R
	return result;
}

// @override
Quantity* HammingPoint::maxDistance(void){
	return new QuantityBit( Value->maxDistance(), true );
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

Hamming_h::Hamming_h(int length){						// length of the input bitstring
	int position = length * Math::dRand();				// randomply pick a bit to "peek"
	Bitset* temp = new Bitset(length);
	temp->set(position);
	Mask = new HammingPoint( "mask", temp );	// create a point out of this mask
}

uint64_t Hamming_h::hash(Point* p ){					// map a multi-dimensional hamming point into an integer

	Quantity* temp = p->multiply(Mask);
	bool nonZero = temp->getBits()->nonZero();			// bitwise AND between point and mask => bit exrtaction
	delete temp;
	if (  nonZero ){									// non negative result means the bit was 1
		return 1;
	}
	else{												// if the result in 0, then the bit was 0
		return 0;
	}
}

Hamming_h::~Hamming_h(){
	delete Mask;
}
