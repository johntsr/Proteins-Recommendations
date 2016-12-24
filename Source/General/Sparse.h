#ifndef __SPARSE__
#define __SPARSE__

#include "Quantity.h"
#include "Point.h"
#include "../DataStructures/DoubleList.h"


// a Point in the multi-dimensional space (used by Euclidean and Cosine Similarity representations)
class SparsePoint: public Point{
	protected:
		Quantity* 	SparseArray;			// the co-ordinates of a point can be stored in an array
		int* 		Indexes;
		int 		Length;
	public:

		SparsePoint(int dimension);			// contruct a point from a given vector
		SparsePoint(std::string Name, List<Pair>* ratingList);

		std::string name		(void);		// @override: the human-readable representation of the point
		int 		dimension	(void);		// @override: the dimensionality of the point
		Quantity* 	value		(void);		// @override: maps a point into a non-negative integer value
		Quantity*	multiply 	(Point* p);	// @override: defines the multiplication between two points
		bool 		operator ==	(Point*); 	// @override: defines the equality of 2 points
		void 		print  		(void);		// @override
		virtual ~SparsePoint();

		int* indexes(){
			return Indexes;
		}
};

#endif
