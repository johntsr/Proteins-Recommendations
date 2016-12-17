#ifndef __VECTOR__
#define __VECTOR__

#include "Quantity.h"
#include "Point.h"


// a Point in the multi-dimensional space (used by Euclidean and Cosine Similarity representations)
class VectorPoint: public Point{
	protected:
		Quantity* 	Array;					// the co-ordinates of a point can be stored in an array
		int 		Dimension;				// the actual number of dimensions a point needs
	public:

		VectorPoint(int dimension);			// contruct a point from a given vector
		VectorPoint(std::string Name, std::string& stringLine, int length);	// constructs a point parsing a string with its co-ordinates

		std::string name		(void);		// @override: the human-readable representation of the point
		int 		dimension	(void);		// @override: the dimensionality of the point
		Quantity* 	value		(void);		// @override: maps a point into a non-negative integer value
		Quantity*	multiply 	(Point* p);	// @override: defines the multiplication between two points
		bool 		operator ==	(Point*); 	// @override: defines the equality of 2 points
		Quantity*	maxDistance	(void);		// @override: returns the maximum possible distance between 2 points
		void 		print  		(void);		// @override
		bool		inRange		(Point* p, Quantity* R);	// @override
		virtual ~VectorPoint();
};

#endif
