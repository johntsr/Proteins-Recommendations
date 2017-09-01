#ifndef __RECOMMEND_MANAGER_O__
#define  __RECOMMEND_MANAGER_O__

#include "../General/Point.h"
#include "../Clusters/Clusters.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

typedef int (*rGenerator)(int N);

// abstract class used to communicate with the user
class RecommendManager{
	protected:

		std::vector<Point*> PointTable;// the table of points to be clustered (1 per metric)
		int 	NumItems;				// the number of items

		int count;

		// hash table between user number and point in "PointTable"
		// HashTable<PointIndex, Point*>* PointMap;
		std::map<Point*,int> PointMap;

		List<List<Pair> > Ratings;		// the actual ratings of each user
		std::vector<double> MeanRatings;			// mean rating of each user
		std::vector<std::vector<bool> > RealRatings;				// true for items the user rated
		std::vector<std::vector<double> > ResultRatings;		// results of predictions (1 per metric)

		double MAE[3];					// Mean Absolute Error (1 per metric)
		std::string Messages[3];		// Evaluation message  (1 per metric)


		LocalHashTable<Point, Point*>* LSH;						// the Locality Sensitive Hash Table
		hash_function** hashFunctions;
		int L_hash;													// the number of Hash Tables the above table uses
		int K_hash;													// the number of "h" hash functions we use
		int barrier;
		int P;


		void 					finalise		(void);
		void 					estimateRating	(int user);
		double  				getRadius		(void);
		List<Point, Point*>* 	findNeighbours	(int i);
		void 					normaliseRatings(int index, List<Pair>* ratingList);

	public:
		RecommendManager();

		void 		init			(int numItems);
		void 		addPoint		(List<Pair>* ratingList);
		void 		updatePoint		(int index, List<Pair>* ratingList);
		std::vector<int> 	evaluate	(int user, int top);

		void print(void){
			LSH->print();
		}

		virtual ~RecommendManager();
};
#endif
