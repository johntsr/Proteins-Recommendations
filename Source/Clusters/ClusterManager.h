#ifndef __IO_NAMAGER__
#define  __IO_NAMAGER__

#include "../General/Point.h"
#include "Clusters.h"

#include <iostream>
#include <string>
#include <fstream>


class ClusterManager{													// abstract class used to communicate with the user
	protected:
		Point** PointTable;					// the table of points to be clustered
		TriangularMatrix* d;				// the table of precomputed distances
		int 	N;							// thr number of points
		int Dimension;						// the dimensionality of the points
		Clustering* Algorithm[9];			// the algorithms to run

		int K_clusters;						// the number of clusters
		int K_hash;							// the "K" parameter in the LSH
		int L;								// the "L" parameter in the LSH
		int Q;								// the "Q" parameter in CLARANS
		int S;								// the "S" parameter in CLARA and CLARANS
		bool Complete;						// whether to print all the clusters or just the centers

		int  countPoints		(std::string fileName); // count the number of lines of a file (equivalent to countin the points)
		// the above method also counts the dimensionality of the points, store it in "dimension"

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void fillTable		(std::string dataPath);					// fills "PointTable" form a file
		void runTests		(std::ofstream& outfFile);				// calls the above methods for every query point
		void finalise		(void);

		// the above methods vary between the different Points
		virtual void 	openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		virtual Point* 	getNextPoint	(std::ifstream& queryFile)=0;// get the next point from a file
		virtual int  	countDimension	(std::string fileName);		// count the dimensionality of the points

	public:
		ClusterManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete);

		void run(std::string& dataPath, std::string& outPath);		// initiate the procedure of "train & test"
		void runCUTests(void);

		virtual ~ClusterManager();
};


class HammingManager: public ClusterManager{
	private:
		Point* 		getNextPoint	(std::ifstream& queryFile);	// get the next point from a file
		int 	 	countDimension	(std::string fileName);		// count the dimensionality of the points

	public:
		HammingManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete);

};

class CosineManager: public ClusterManager{
	private:
		Point* 	getNextPoint(std::ifstream& queryFile);		// get the next point from a file
		void 	openFileRead(std::string& path, std::ifstream& file);
	public:
		CosineManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete);

};

class EuclideanManager: public ClusterManager{
	private:
		bool Explicit;

		Point* 	getNextPoint(std::ifstream& queryFile);		// get the next point from a file
		void 	openFileRead(std::string& path, std::ifstream& file);
	public:
		EuclideanManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete, bool flag);

};


class MetricSpaceManager: public ClusterManager{
	private:
		std::string* Names;
		int currentPointIndex;

		Point* 	getNextPoint(std::ifstream& queryFile);					// get the next point from a file
		void 	openFileRead(std::string& path, std::ifstream& file);
	public:
		MetricSpaceManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete);
		~MetricSpaceManager();
};


#endif
