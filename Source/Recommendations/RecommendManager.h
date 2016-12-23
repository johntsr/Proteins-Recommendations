#ifndef __RECOMMEND_MANAGER__
#define  __RECOMMEND_MANAGER__

#include "../General/Point.h"
#include "../Clusters/Clusters.h"

#include <iostream>
#include <string>
#include <fstream>

typedef int (*rGenerator)(int N);

struct Pair{
	int Item;
	int Rating;

	Pair(int item, int rating){
		Item = item;
		Rating = rating;
	}
};

class RecommendManager{													// abstract class used to communicate with the user
	protected:
		Point** PointTable;					// the table of points to be clustered
		int 	NumUsers;					// thr number of points
		int 	NumItems;
		bool Complete;						// whether to print all the clusters or just the centers

		List<List<Pair> > Ratings;
		HashTable<PointIndex, Point*>* PointMap;
		double* MeanRatings;
		double** ResultRatings;
		bool** RealRatings;

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void finalise		(void);

		void getFileInfo	(std::string dataPath);
		Point* 	getNextPoint(std::ifstream& queryFile);// get the next point from a file
		void 	estimateRating	(int user, List<Point, Point*>* Neighbors);
		void 	evaluate		(std::ofstream& outfFile, std::string Message);


		// the above methods vary between the different Points
		virtual void fillTable			(std::string dataPath);					// fills "PointTable" form a file
		virtual void 	openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		virtual void 	runTests		(std::ofstream& outfFile)=0;

	public:
		RecommendManager(bool complete);

		void run(std::string& dataPath, std::string& outPath);		// initiate the procedure of "train & test"
		void runCUTests(void);

		virtual ~RecommendManager();
};


class NNRecommendManager: public RecommendManager{
	private:
		LocalHashTable<Point, Point*>* LSH;							// the Locality Sensitive Hash Table
		int L_hash;													// the number of Hash Tables the above table uses
		int K_hash;													// the number of "h" hash functions we use
		hash_function** hashFunctions;
		int barrier;
		int P;

		Quantity* getRadius		(void);
		void 	fillTable		(std::string dataPath);
		void 	runTests		(std::ofstream& outfFile);

		List<Point, Point*>* 	findNeighbours	(Point* point);

	public:
		NNRecommendManager(bool complete);
		~NNRecommendManager();
};




class ClusterRecommendManager: public RecommendManager{
	private:

		int K_clusters;						// the number of clusters
		TriangularMatrix* d;				// the table of precomputed distances
		ClusterAlgorithm* Algorithm;		// the algorithms to run

		void 	fillTable		(std::string dataPath);
		void 	runTests		(std::ofstream& outfFile);

		List<Point, Point*>* 	findNeighbours	(List<AssignPair>* cluster);


	public:
		ClusterRecommendManager(bool complete);
		~ClusterRecommendManager();
};



#endif
