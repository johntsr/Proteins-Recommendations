#ifndef __RECOMMEND_MANAGER__
#define  __RECOMMEND_MANAGER__

#include "../General/Point.h"
#include "../Clusters/Clusters.h"

#include <iostream>
#include <fstream>
#include <string>

typedef int (*rGenerator)(int N);

// abstract class used to communicate with the user
class RecommendManager{
	protected:
		Point** PointTable[3];			// the table of points to be clustered (1 per metric)
		int 	NumUsers;				// the number of users
		int 	NumItems;				// the number of items

		// hash table between user number and point in "PointTable"
		HashTable<PointIndex, Point*>* PointMap[3];

		List<List<Pair> > Ratings;		// the actual ratings of each user
		double* MeanRatings;			// mean rating of each user
		bool** RealRatings;				// true for items the user rated
		bool* DataPoint;				// true for users to skip in predictions (used in validation process)

		double** ResultRatings[3];		// results of predictions (1 per metric)
		double MAE[3];					// Mean Absolute Error (1 per metric)
		std::string Messages[3];		// Evaluation message  (1 per metric)

		bool Validate;					// whether to perform F-fold cross validation

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void finalise		(void);

		void getFileInfo	(std::string dataPath);
		Point** 	getNextPoint(std::ifstream& queryFile);// get the next point from a file
		void 	estimateRating	(int metric, int user, List<Point, Point*>* Neighbors);
		void 	evaluate		(int metric, std::ofstream& outfFile, std::string Message);
		void 	validate 		(std::ofstream& outfFile);


		// the above methods vary between the different Points
		virtual void fillTable		(std::string dataPath);					// fills "PointTable" form a file
		virtual void openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		virtual void runTests		(int metric, std::ofstream& outfFile)=0;

	public:
		RecommendManager(bool validate);

		void run(std::string& dataPath, std::string& outPath);		// initiate the procedure of "train & test"
		void runCUTests(void);

		virtual ~RecommendManager();
};


class NNRecommendManager: public RecommendManager{
	private:
		LocalHashTable<Point, Point*>* LSH[3];						// the Locality Sensitive Hash Table
		hash_function** hashFunctions[3];
		int L_hash;													// the number of Hash Tables the above table uses
		int K_hash;													// the number of "h" hash functions we use
		int barrier;
		int P;

		double  getRadius		(int metric);
		void 	fillTable		(std::string dataPath);
		void 	runTests		(int metric, std::ofstream& outfFile);
		void 	validate 		(std::ofstream& outfFile);

		List<Point, Point*>* 	findNeighbours	(int metric, int i);

	public:
		NNRecommendManager(bool validate);
		~NNRecommendManager();
};




class ClusterRecommendManager: public RecommendManager{
	private:

		TriangularMatrix* d[3];				// the table of precomputed distances
		int K_clusters[3];					// the number of clusters
		ClusterAlgorithm* Algorithm[3];		// the algorithms to run

		void 	fillTable		(std::string dataPath);
		void 	runTests		(int metric, std::ofstream& outfFile);

		void 	findAlgorithm	(int metric);

		List<Point, Point*>* 	findNeighbours	(int metric, List<AssignPair>* cluster);

	public:
		ClusterRecommendManager(bool validate);
		~ClusterRecommendManager();
};



#endif
