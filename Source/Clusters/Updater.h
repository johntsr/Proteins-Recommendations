#ifndef __UPDATER__
#define __UPDATER__

#include "ClusterStructures.h"
#include "../DataStructures/DoubleList.h"

class Updater{
	protected:
		TriangularMatrix* d;				// the table of distances
		double* J;							// the current value fo the objectiev function
		int K;								// the number of clusters
		int* Centers;						// the new centroids
		int* OldCenters;					// the old centroids
		List<AssignPair>* AssignedPoints;	// the clusters

		// variables that avoid endless loops of the custering algorithm
		int timesSwapped;					// number of swaps so far
		double minJ;						// minimum value of J so far
		bool endOfDay;						// if false, no swapping occurs

		void   keepOldCenters(void);
		double PAM_swap(int m, int t);
		double computeDJ(int cluster, int m, int t, AssignPair* iPair );
		void  swap(int m, int t, double J2);
	public:
		Updater(TriangularMatrix* dPtr, double* j, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints);
		virtual void update(void)=0;

		virtual ~Updater(){}
};

class LloydsUpdate: public Updater{
	private:
		AssignPair*  findt(int m);
	public:
		LloydsUpdate(TriangularMatrix* dPtr, double* j, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints);
		void update(void);
};

class CLARANS: public Updater{
	private:
		int N;					// nubmer of points
		int Q;					// number of pairs to check
		int S;					// nubmer of iterations to do
		int* Q_k;				// the center of the pair
		int* Q_n;				// the other point of the pair

		bool centroid(int t);	// true if t is a centroid
		void selectPair(int i);	// select a pair, store it in index "i" in the above tables
	public:
		CLARANS(TriangularMatrix* dPtr, double* j, int n, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints, int s, int q);
		void update(void);
		~CLARANS();
};

#endif
