#include "Hamming.h"
#include "../General/Math.h"

#include <bitset>
#include <limits.h>


/*****************************************************************
***************** HammingPoint class methods *********************
******************************************************************/

HammingPoint::HammingPoint(std::string name, uint64_t value, int dimension ) {
	Point::Name = name;

	Value = new Quantity(value);
	Dimension = dimension;
}

// @override
std::string HammingPoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int HammingPoint::dimension(void) {				// the dimensionality of the point
	return Dimension;							// the number of bits a hamming point has
}

// @override
Quantity* HammingPoint::value(void){			// maps a point into a non-negative integer value
	return Value;								// simply the bitstring itself
}

// @override
Quantity* HammingPoint::multiply (Point* p){		// defines the multiplication between two points
	// simply a bitwise AND
	return new QuantityBit( value()->getBits() & p->value()->getBits() );
}

// @override
Quantity* HammingPoint::distance(Point* p){
	uint64_t mask = 1;							// variable used to construct the Mask as a point
	uint64_t distance = 0;						// variable on which the result is gradually built
	uint64_t value1 = value()->getBits();		// the bits representing this hamming point
	uint64_t value2 = p->value()->getBits();	// the bits representing p hamming point

	value1 = value1 ^ value2;					// xor their values
	for( int i = 0; i < dimension(); i++) {		// for every bit of the xor result
		if( (value1 & mask) == 1){				// if they are different ( i.e. xor[bit_i] == 1 )
			distance++;							// another different bit is found, increment the distance
		}
		value1 >>= 1;							// shift the mask in order to compare the next bit
	}

	return new QuantityBit(distance);
}

// @override
bool HammingPoint::operator == (Point* p){
	if( dimension() != p->dimension() ){
		return false;
	}

	uint64_t zero = 0;
	return (value()->getBits() ^ p->value()->getBits()) == zero;
}

// @override
void HammingPoint::print(void){
	std::cout 	<<  std::bitset<64>( value()->getBits() )
				<< "\t(" << name() << ")"  << std::endl;
}

// @override
bool HammingPoint::inRange(Point* p, Quantity* R){
	Quantity* temp = distance( p );
	bool result = temp->getBits() < ( (uint64_t)(Point::C) * R->getBits());
	delete temp;			// p is in range, if our distance in smaller than R
	return result;
}

// @override
Quantity* HammingPoint::maxDistance(void){
	return new QuantityBit( (uint64_t)UINT64_MAX);
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
	uint64_t base = 1;									// mask of 64 bits
	base <<= position;									// calibrate the mask to extract the specific bit
	Mask = new HammingPoint( "mask", base, length );	// create a point out of this mask
}

uint64_t Hamming_h::hash(Point* p ){					// map a multi-dimensional hamming point into an integer

	Quantity* temp = p->multiply(Mask);
	uint64_t mul = temp->getBits();						// bitwise AND between point and mask => bit exrtaction
	delete temp;
	if (  mul == 0 ){									// if the result in 0, then the bit was 0
		return 0;
	}
	else{												// non negative result means the bit was 1
		return 1;
	}
}

Hamming_h::~Hamming_h(){
	delete Mask;
}
