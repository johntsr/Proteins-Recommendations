#include "../General/Point.h"
#include "../General/HashFunction.h"

// an actual point of the hamming space
class MetricSpacePoint: public Point {
	private:
		Quantity* 	DistanceMatrix;			// the "row" of distances from other points
		int 		Length;					// the size of the above table
		int 		Position;				// the index of this point in the distance table
	public:

		MetricSpacePoint(std::string name, int length, std::string lineOfDistances, int position);

		std::string name		(void);		// @override: the human-readable representation of the point
		int 		dimension	(void);		// @override: the dimensionality of the point
		Quantity* 	value		(void);		// @override: maps a point into a non-negative integer value
		Quantity* 	multiply 	(Point* p);	// @override: defines the multiplication between two points
		Quantity* 	distance	(Point* p);	// @override
		bool 		operator ==	(Point*); 	// @override
		void 		print  		(void);		// @override
		bool 		inRange		(Point*, Quantity*);// @override
		Quantity* 	maxDistance	(void);		// @override
		PointType	type		(void);		// @override

		~MetricSpacePoint();
};



// a hamming hash function
// represents the "h" hash function of the e-class slides
// i.e. it just "extracts" a bit of a specific position
class MetricSpace_h: public hash_function{
	private:
			// parameter of e-class slides
			Point* x1;
			Point* x2;
			double t1;

			// since the sitances between "x1" and "x2" is repeatedly needed
			// we compute them once for speed's sake
			double _2d;			// "2*distance(x1,x2)"
			double dd;			// "distance(x1,x2) * distance(x1,x2)"

			double compute(Point* x);			// helper function
	public:

		MetricSpace_h(Point** Points, int n);

		uint64_t hash(Point* p );				// @override: map a multi-dimensional hamming point into an integer

};


class MetricSpace_g: public BitString_g{
	public:

		MetricSpace_g(int k, Point** Points, int n): BitString_g(k) {
			for(int i = 0; i < K; i++ ){						// initialise every function
				Table_h[i] = new MetricSpace_h(Points, n);		// i.e. specify which bit to extract
			}
		}

		~MetricSpace_g() {
			for(int i = 0; i < K; i++ ){
				delete Table_h[i];
			}
		}


};
