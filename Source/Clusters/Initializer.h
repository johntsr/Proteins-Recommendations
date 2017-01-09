#ifndef __INITIALIZER__
#define __INITIALIZER__

#include "ClusterStructures.h"

// abstract class that initializes the clustering algorithm
// basically, it picks the initial centers based on some desired properties
class Initializer{

	protected:
		TriangularMatrix* d;	// the distances between all points

		int* Centers;			// the Centers that I will pick (0,...,N-1)
		int 	K;				// the numbers of the above centers
		int 	N;				// the number of points to be clustered

	public:
		Initializer( int* centers, TriangularMatrix* dPtr, int k, int n );
		virtual void initialise(void)=0;

		virtual ~Initializer(){}
};

// initialzer based on the Park-Jun paper
// basically, pick centers that are close to each other
class Park_Jun: public Initializer{
	public:
		Park_Jun( int* centers, TriangularMatrix* dPtr, int k, int n );
		void initialise(void);
};

// initialzer based on the K-medoids++ algorithm
// basically, pick centers that are remote to each another
class K_MedoidsPP: public Initializer{
	private:

		double minFromCluster(int i, int k);
		bool uniqueCenter(int k, int r);

	public:
		K_MedoidsPP( int* centers, TriangularMatrix* dPtr, int k, int n );
		void initialise(void);
};

class RandInit: public Initializer{
	public:
		RandInit( int* centers, TriangularMatrix* dPtr, int k, int n );
		void initialise(void);
};

#endif
