#ifndef __ASSIGNER__
#define __ASSIGNER__

#include "ClusterStructures.h"

#include "../DataStructures/LinearHash.h"
#include "../DataStructures/DoubleList.h"
#include "../DataStructures/LocalityHashing.h"

#include "../General/Point.h"
#include "../General/HashFunction.h"

// abstract class that assigns points to clusters
// basically, it matches a point to the center it is closest to it
class Assigner{
	protected:
		TriangularMatrix* d;				// the matrix of distances is already computed in the initializing step, just keep a reference
		int 	N;							// the number of points

		int* Centers;						// the indexes of the Centers (1,...,N-1)
		int 	K;							// the nubmer of the above centers

		List<AssignPair>* AssignedPoints;	// "K" lists that represent the clusters, 1 for each center

		void assignBrute(int i, int& center, int& center2);
		void flush(void);

	public:
		Assigner(TriangularMatrix* dPtr, int n, int* centers, int k, List<AssignPair>* assignedPoints );

		virtual double assign(void)=0;		// return the value of the objective function

		virtual ~Assigner(){}
};

// assign based on the simple PAM approach
// basically, it compares all distances
class PAM_Simple: public Assigner{
	public:

		PAM_Simple(TriangularMatrix* dPtr, int n, int* centers, int k, List<AssignPair>* assignedPoints );

		virtual double assign(void);
};

// class used to map a point to its index in the "global" table (index = 1,...,N-1)
class PointIndex{
	public:
		Point* point;
		int i;								// the index of the point

		PointIndex(int n, Point* p){
			i = n;
			point = p;
		}

		operator Point*(void){
			return point;
		}

		bool operator == (Point* p){
			return point == p;
		}

		static int hash(Point* p){
			// remember: hash("point i") == hash( (Point*)("index i") )
			return (long)(p);
		}
};

// assign based on the simple Raverse LSH approach
// basically, it inserts all the points into an LSH structure
// then, it approximately finds the nearest center
class Reverse_LSH: public Assigner{
	private:
		LocalHashTable<Point, Point*>* LSH;				// the Locality Sensitive Hash Table, used to perform queries
		Point** PointTable;								// the "global" table of points, used to fill the LSH
		Point* Barrier;									// "barrier" point, used to discriminate the matched points from the other ones
		int* NearestCenter;								// for every point, store its closest center
		int* NearestCenter2;							// for every point, also store its second closest center
		HashTable<PointIndex, Point*>* PointMap;		// Hash Table that maps indexes to points
														// "glue" between LSH and indexing of clusters

		hash_function** hashFunctions;
		int L;

		void lshArguments(int& TableSize, hash_function**& HashFunctions, int LSH_K, int L);	// initialize LSH parameters
		double minDistBetweenCenters(void);													// find the min distance between 2 centers
		void assignToCenters(List<Point,Point* >* ResultPoints, int k);							// assign points of an LSH query to clusters
		void assignRestToCenters(void);															// finally, assign the rest points to clusters


	public:
		Reverse_LSH(TriangularMatrix* dPtr, Point** pointTable, int n, int* centers, int k, List<AssignPair>* assignedPoints, int k_hash, int l);

		double assign(void);

		~Reverse_LSH();
};

#endif
