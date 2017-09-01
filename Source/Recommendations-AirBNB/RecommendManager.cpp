#include "FoldValidation.h"
#include "RecommendManager.h"
#include "../General/Timing.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/Hamming.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Clusters/Assigner.h"			// need for Hash Table (point index)!

#include <float.h>
#include <map>

#define HASH_SIZE 1000

using namespace std;

PrintReset coutR;
PrintCondReset coutCR;


double abs(double x){
	if( x < 0.0 ){
		return -x;
	}
	return x;
}

string metricName(){
	return "Cosine";
}

/*****************************************************************
***************** RecommendManager class methods *************************
******************************************************************/

RecommendManager::RecommendManager() {
	NumItems = 0;

	L_hash = 5;
	K_hash = 5;
	barrier = 3;
	P = 50;

	// PointMap = NULL;
	LSH = NULL;
	hashFunctions = NULL;
}

void RecommendManager::init(int numItems){
	finalise();
	NumItems = numItems;

	// PointMap = new HashTable<PointIndex, Point*> (HASH_SIZE);

	int TableSize = 1 << K_hash;								// 2^K slots in the Hash Tables
	hashFunctions = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[i] = new Cosine_g( K_hash, NumItems );	// and initialise it
	}
	LSH = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)
}

void RecommendManager::addPoint(List<Pair>* ratingList){
	int count = MeanRatings.size();
	stringstream name;
	name << count;

	// calculate mean value of ratings
	double mean = 0.0;
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		mean += node->data()->Rating;
	}
	mean /= ratingList->count();
	MeanRatings.push_back(mean);									// store the mean value of the user

	ResultRatings.push_back(vector<double>());
	RealRatings.push_back(vector<bool>());
	for(int i = 0; i < NumItems; i++){							// initialise every item's rating
		ResultRatings[count].push_back(DBL_MAX);
		RealRatings[count].push_back(false);
	}

	// for the actual ratings
	// store the normalised rating
	// and remember that they are "real" ratings
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		ResultRatings[count][node->data()->Item] = node->data()->Rating - mean;
		RealRatings[count][node->data()->Item] = true;
	}

	int i = count;
	PointTable.push_back(new CosinePointSparse( name.str(), ratingList));
	// PointMap->insert( new PointIndex(i, PointTable[i]), true );
	PointMap[PointTable[i]] = i;
	LSH->insert( PointTable[i] );					// store all the points
}

void RecommendManager::estimateRating(int user){

	List<Point, Point*>* Neighbors = findNeighbours(user);
	int neighborNum = Neighbors->count();		// get the number of neighbors this user has

	if( neighborNum == 0 ){						// if no neighbors are found

		// set every unrated item to mean value of user ans return
		for(int item = 0; item < NumItems; item++){

			if( RealRatings[user][item] ){
				continue;
			}

			ResultRatings[user][item] = MeanRatings[user];
		}
		return;
	}

	// now, procceed normally
	int* neighborIndexes = new int[neighborNum];	// stores the indexes of the neighbors in the global table
	double* neighborSim = new double[neighborNum];	// stores the similarity with the above neighbor

	// fill the above tables with the appropriate values
	int index = 0;
	for (Node<Point>* node = Neighbors->start() ; node != NULL; node = node->next(), index++ ) {
		// int neighbor = (*PointMap)[node->data()]->i;
		int neighbor = PointMap[node->data()];
		neighborIndexes[index] = neighbor;
		neighborSim[index] = PointTable[user]->similarity( PointTable[neighbor] );
	}

	// now, predict the rating of the unrated items
	for(int item = 0; item < NumItems; item++){

		if( RealRatings[user][item] ){
			continue;
		}

		int count = 0;
		double sumRatings = 0.0, sumWeights = 0.0;
		for (index = 0 ; index < neighborNum; index++ ) {

			// skip the neighbor if he is me (of course!)
			// or if he hasn't rated the item I am interested in
			if( neighborIndexes[index] == user || !RealRatings[neighborIndexes[index]][item] ){
				continue;
			}

			count++;								// another "usefull" neighbor was found for this item
			// follow the e-class definition
			sumRatings += neighborSim[index] * ResultRatings[ neighborIndexes[index] ][item];
			sumWeights += abs(neighborSim[index]);
		}

		// if no "usefull" neighbors were found
		// of if our similarity is 0 (can't divide!)
		// then set to mean rating
		if( count == 0 || sumWeights == 0.0 ){
			ResultRatings[user][item] = MeanRatings[user];
		}
		else{
			ResultRatings[user][item] = MeanRatings[user] + (sumRatings / sumWeights);
		}
	}

	delete[] neighborIndexes;
	delete[] neighborSim;
}

vector<int> RecommendManager::evaluate(int user, int top){
	stringstream s;

	estimateRating(user);

	s << "Evaluating the top 5 items for a user... (metric = " << metricName() << ")"; coutR << s;

	// prints the top 5 predicted items
	int* itemIndexes = new int[NumItems];

	int i = user;
	// keep the indexes of items before sorting
	for( int j = 0; j < NumItems; j++ ){
		itemIndexes[j] = j;
	}

	Math::sort(&(ResultRatings[i][0]), itemIndexes, NumItems);

	vector<int> top5Items;
	for(int j = 0, count = 0; j < NumItems && count < top; j++ ){
		// print only predicted items
		if( !RealRatings[i][ itemIndexes[j] ] ){
			count++;
			top5Items.push_back(itemIndexes[j]);
		}
	}

	s << "Evaluation completion: " << i * 100.0 / PointTable.size() << "%"; coutCR << s;

	delete[] itemIndexes;
	return top5Items;
}

void RecommendManager::finalise(void){
	for(unsigned int i = 0; i < PointTable.size(); i++){
		delete PointTable[i];
	}

	PointTable.clear();

	// if( PointMap != NULL){
	// 	delete PointMap;
	// 	PointMap = NULL;
	// }

	if( LSH != NULL){
		delete LSH;
		LSH = NULL;
	}

	if( hashFunctions != NULL ){
		for(int i = 0; i < L_hash; i++){
			delete hashFunctions[i];
		}
		delete[] hashFunctions;
		hashFunctions = NULL;
	}
}

double RecommendManager::getRadius(){
	return 0.5;
}

List<Point, Point*>* RecommendManager::findNeighbours(int user){

	return new List<Point, Point*>();

	Point* point = PointTable[user];					// get the point of global table

	int times = 0;												// number of times I performed NN query
	int BarrierTimes = 3;										// max nubmer ot times I permit
	int Tolerance = P / 4;										// don't expect to get exactly P neigbors!

	// big and small radius for the queries
	double RSmall = getRadius(), RBig = getRadius();

	// lists that hold neighbors for big and small radius
	List<Point, Point*> *ResultPointsSmall = new List<Point, Point*>(),
						*ResultPointsBig = new List<Point, Point*>();

	LSH->inRange( point, RSmall, *ResultPointsSmall );	// perform first query

	if( abs(ResultPointsSmall->count() - P) < Tolerance ){		// if the total number is OK
		delete ResultPointsBig;									// retun immediately
		return ResultPointsSmall;
	}
	else if( ResultPointsSmall->count() < P ){					// else, if too few neigbors were found
		// I have to look for the initial value of RBig
		do{
			ResultPointsBig->flush();
			RBig = PointTable[0]->multiplyDouble(RBig, 2.0);	// double its value
			LSH->inRange( point, RBig, *ResultPointsBig );		// NN query
			times++;
		} while( ResultPointsBig->count() < P && times < BarrierTimes );// until I am satisfied

		if( abs(ResultPointsBig->count() - P) < Tolerance || times == BarrierTimes ){	// if OK, return
			delete ResultPointsSmall;
			return ResultPointsBig;
		}
		else{ 	// else, more queries will follow
			RSmall = PointTable[0]->multiplyDouble(RBig, 0.5);		// small = big / 2
		}
	}
	else{														// else, if too many neigbors were found
		// I have to look for the initial value of RSmall
		do{
			ResultPointsSmall->flush();
			RSmall = PointTable[0]->multiplyDouble(RSmall, 0.5);	// halve its value
			LSH->inRange( point, RSmall, *ResultPointsSmall );		// NN query
			times++;
		} while( ResultPointsSmall->count() > P && times < BarrierTimes );	// until I am satisfied

		if(  abs(ResultPointsSmall->count() - P) < Tolerance || times == BarrierTimes ){ // if OK, return
			delete ResultPointsBig;
			return ResultPointsSmall;
		}
		else{	// else, more queries will follow
			RBig = PointTable[0]->multiplyDouble(RSmall, 2.0);		// big = small * 2
		}
	}

	// now, perform queries a la binary search
	do {
		double RMean = (RSmall + RBig) / 2.0;
		List<Point, Point*> *ResultPointsMean = new List<Point, Point*>();
		LSH->inRange( point, RMean, *ResultPointsMean );

		if( abs(ResultPointsMean->count() - P) < Tolerance || times == BarrierTimes ){
			delete ResultPointsBig;
			delete ResultPointsSmall;
			return ResultPointsMean;
		}
		else if( ResultPointsMean->count() < P ){
			RSmall = RMean;
			delete ResultPointsSmall;
			ResultPointsSmall = ResultPointsMean;
		}
		else{
			RBig = RMean;
			delete ResultPointsBig;
			ResultPointsBig = ResultPointsMean;
		}

		times++;

	} while(true);
}

RecommendManager::~RecommendManager(){
	finalise();
}
