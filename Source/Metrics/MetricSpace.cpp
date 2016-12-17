#include "MetricSpace.h"
#include "../General/Math.h"

#include <float.h>
#include <limits.h>
#include <sstream>

/*****************************************************************
***************** MetricSpacePoint class methods ******************
******************************************************************/

MetricSpacePoint::MetricSpacePoint(std::string name, int length, int n, int position, EuclideanPoint** configuration){

	Point::Name = name;
	Position = position;

	N  = n;
	Configuration = configuration;

	Length = length;
	DistanceMatrix = new Quantity[Length];
	for(int i = 0; i < Length; i++){
		DistanceMatrix[i] = -1;
	}
}

// @override
std::string MetricSpacePoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int MetricSpacePoint::dimension(void) {				// the dimensionality of the point
	return Length;
}

// @override
Quantity* MetricSpacePoint::value(void){			// maps a point into a non-negative integer value
	return new Quantity( new Bitset(Position), true );
}

// @override
Quantity* MetricSpacePoint::multiply (Point* p){	// defines the multiplication between two points
	return NULL;									// NOTHING!
}

// @override
Quantity* MetricSpacePoint::distance(Point* p){
	Quantity* temp = p->value();
	uint64_t position = temp->getBits()->size();

	if( DistanceMatrix[position].castAsDouble() == -1.0 ){
		// TODO...
	}

	delete temp;
	return new Quantity( DistanceMatrix[ position ].getDouble() );

}

// @override
bool MetricSpacePoint::operator == (Point* p){
	if( dimension() != p->dimension() ){
		return false;
	}

	return Point::Name == p->name();
}

// @override
void MetricSpacePoint::print(void){
	std::cout 	<< "\t(" << name() << ")"  << std::endl;
	// for(int i = 0; i < Length; i++){
	// 	std::cout << DistanceMatrix[i].getString() << " ";
	// }
	// std::cout << std::endl << std::endl;
}

// @override
bool MetricSpacePoint::inRange(Point* p, Quantity* R){
	Quantity* temp = distance( p );
	if( temp->getDouble() == 0.0 ){
		delete temp;
		return false;
	}

	bool result = temp->getDouble() < Point::C * R->getDouble();
	delete temp;
	return result;
}

// @override
Quantity* MetricSpacePoint::maxDistance(void){
	return new Quantity(DBL_MAX);
}

MetricSpacePoint::~MetricSpacePoint(){
	delete[] DistanceMatrix;
}

// @override
PointType MetricSpacePoint::type(void){
	return METRIC_SPACE;
}

/*****************************************************************
***************** MetricSpace_h class methods ************************
******************************************************************/

double MetricSpace_h::compute(Point* x){
	Quantity* temp = x->distance(x1);
	double d1 = temp->getDouble();
	d1 *= d1;
	delete temp;

	temp = x->distance(x2);
	double d2 = temp->getDouble();
	d2 *= d2;
	delete temp;

	return ( d1 + d2 - dd ) / ( _2d );
}

MetricSpace_h::MetricSpace_h(Point** Points, int n){

	// "Points": table of ala points of the data set
	// randomply pick "x1" and "x2" from there
	x1 = Points[ (int)Math::dRand(0, n) ];
	x2 = Points[ (int)Math::dRand(0, n) ];

	Quantity* temp = x1->distance(x2);

	double tempd = temp->getDouble();
	delete temp;

	_2d = tempd * 2;
	dd = tempd * tempd;

	// t1 is median of ...


	// double sum = 0.0;
	// for( int i = 0; i < n; i++){
	// 	sum += compute( Points[i] );
	// }
	// t1 = sum / n;

	double* Results = new double[n];
	for( int i = 0; i < n; i++){
		Results[i] = compute( Points[i] );
	}
	t1 = Math::selection(Results, n, n/2 + 1);
	delete[] Results;
}

uint64_t MetricSpace_h::hash(Point* p ){					// map a multi-dimensional hamming point into an integer
	if (  compute(p) < t1 ){								// if the result in 0, then the bit was 0
		return 0;
	}
	else{												// non negative result means the bit was 1
		return 1;
	}
}
