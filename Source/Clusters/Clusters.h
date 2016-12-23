#ifndef __CLUSTERS__
#define __CLUSTERS__

#include <string>
#include <fstream>

#include "ClusterStructures.h"
#include "Initializer.h"
#include "Assigner.h"
#include "Updater.h"

#include "../DataStructures/DoubleList.h"
#include "../General/Point.h"
#include "../General/Quantity.h"


// abstract class that helps define a clustering algorithm
class Clustering{
	protected:
		std::string message;					// an info message
		double execTime;						// execution time of algorithm
	public:
		void printInfo	(std::ofstream& outfFile);

		virtual void run(void)=0;
		virtual double evaluate(std::ofstream& outfFile, bool complete, bool print)=0;
		virtual ~Clustering(){}
};

// a clustering algorithm composed of 3 steps: initialization, assignment and update
class ClusterAlgorithm: public Clustering{
	protected:
		TriangularMatrix* d;				// the matrix of distances between points
		Point** PointTable;					// the "global" table of points
		int 	N;							// the total number of points
		int* Centers;						// the newly computed centers
		int* OldCenters;					// the previously computed centers, used to check if changes occur
		int 	K;							// the number of centers
		double J;							// the value of the objective function
		List<AssignPair> * AssignedPoints;	// "K" lists that represent the clusters, 1 for each center
		Initializer* Init;					// Initializer object
		Assigner* Assign;					// Assigner object
		Updater* Update;					// Updater object

		double SumSilhouette;
		double* ClusterSilhouette;

		bool 	changes(void);					// check if changes occures after the update step
		double 	avgDistFromCluster( int i, int center );	// used in Silhouette computation
		void 	computeSilhouette(void);


	public:
		ClusterAlgorithm(Point** pointTable, TriangularMatrix* dPtr, int n, int k, int type, int k_hash, int l, int q, int s);

		void run(void);						// run the algorithm
		double getJ(void){ return J; }		// get value of objective function

		void printCenters(std::ofstream& outfFile);		// print the centroids
		void printClusters(std::ofstream& outfFile);	// print the whole cluster
		List<AssignPair>* getCluster(int k);

		virtual double evaluate	(std::ofstream& outfFile, bool complete, bool print);	// evaluate after run()

		virtual ~ClusterAlgorithm();

		static int  runCUTests		(void);
		static void dummyClustering	(void);
};

class ProteinsCluster: public ClusterAlgorithm{
	public:
		ProteinsCluster(Point** pointTable, TriangularMatrix* dPtr, int n, int k, int type, int k_hash, int l, int q, int s);
		double evaluate	(std::ofstream& outfFile, bool complete, bool print);
};


// CLARA algorithm
class CLARA: public Clustering{
	private:
		int N_;							// the N' ( <<< N ) of the e-class slides
		ClusterAlgorithm** Algorithms;	// "S" identical algorithms will be run
		int S;							// "S" parameter of the e-class slides
		int K;							// number of clusters
		int minIndex;					// index of best algorithm

		// lists that keep references to memory to be deleted
		List<Point*> DeleteListPoint;
		List<TriangularMatrix> DeleteListMatrix;

	public:
		CLARA(Point** PointTable, int N, int s, int n_, int k);

		void run		(void);
		double evaluate(std::ofstream& outfFile, bool complete, bool print);
		~CLARA();
};



#endif
