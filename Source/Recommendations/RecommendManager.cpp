#include "FoldValidation.h"
#include "RecommendManager.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/Hamming.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Clusters/Assigner.h"			// need for Hash Table (point index)!

#include <sstream>
#include <float.h>

#define COS_INDEX 0
#define EUCL_INDEX 1
#define HAM_INDEX 2

using namespace std;

double abs(double x){
	if( x < 0.0 ){
		return -x;
	}
	return x;
}

string metricName(int metric){
	switch (metric) {
		case COS_INDEX: return "Cosine";
		case EUCL_INDEX: return "Euclidean";
		default: return "Hamming";
	}
}

/*****************************************************************
***************** RecommendManager class methods *************************
******************************************************************/

void RecommendManager::getPath(string& path, string message){
	if( path != "" ){											// if the path is already set
		return;													// just return
	}

	cout << message << endl;									// else, promt the user to give a path
	cin >> path;												// and actually store the path
}

void RecommendManager::openFileWrite(string& path, ofstream& file){
	file.open( path.c_str(), std::ios_base::app );								//open a file to write, create it if it doesn't exist
}

void RecommendManager::openFileRead(string& path, ifstream& file){
	file.open( path.c_str() );									// open a file to read
	while( !file.is_open() ){									// if somethind went wrong
		path = "";												// clear the path
		getPath(path, "Cannot open file : " + path + ". Please, enter a valid path." );	// promt the user to re-enter a path
		file.open( path.c_str() );								// and attempt to open the new file
	}
}

RecommendManager::RecommendManager(bool validate) {
	NumUsers = 0;
	NumItems = 0;
	Validate = validate;
}

Point** RecommendManager::getNextPoint(ifstream& queryFile){
	static int count = -1;										// the id of each user

	stringstream name;
	count++;
	name << count + 1;

	List<Pair>* ratingList = Ratings.removeAtStart();			// get the list of ratings of current user

	// calculate mean value of ratings
	double mean = 0.0;
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		mean += node->data()->Rating;
	}
	mean /= ratingList->count();


	MeanRatings[count] = mean;									// store the mean value of the user

	for(int i = 0; i < NumItems; i++){							// initialise every item's rating
		ResultRatings[COS_INDEX][count][i] = DBL_MAX;
		ResultRatings[EUCL_INDEX][count][i] = DBL_MAX;
		ResultRatings[HAM_INDEX][count][i] = DBL_MAX;
		RealRatings[count][i] = false;
	}

	// for the actual ratings
	// store the normalised rating
	// and remember that they are "real" ratings
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		ResultRatings[COS_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		ResultRatings[EUCL_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		ResultRatings[HAM_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		RealRatings[count][node->data()->Item] = true;
	}

	Point** temp = new Point*[3];
	temp[COS_INDEX] = new CosinePointSparse( name.str(), ratingList);
	temp[EUCL_INDEX] = new EuclideanPointSparse( name.str(), ratingList);
	temp[HAM_INDEX] = new HammingPoint( name.str(), ratingList, NumItems);

	delete ratingList;
	return temp;
}

void RecommendManager::getFileInfo(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);								// open the input file containing ratings of users

	NumUsers = -1;												// initialise with -1, no such user exists!
	NumItems = -1;												// initialise with -1, no such item exists!
	int tempUsers = 0, tempItem = 0, rating = 0;

	while( file >> tempUsers ){									// read the whole file

		if( tempUsers != NumUsers ){							// if a new user was found
			Ratings.insertAtEnd( new List<Pair>(), true );		// insert a new list in the rating list
		}

		NumUsers = tempUsers;									// # of users is the id of last user
		file >> tempItem;										// read the next item
		if(tempItem > NumItems){								// if its id is the biggest so far
			NumItems = tempItem;								// then set its id to be the # of items
		}

		file >> rating;											// read the rating of the previous item

		Ratings.end()->data()->insertAtEnd( new Pair(tempItem - 1, rating), true );
	}


	// allocate tables to store the results of each metric
	ResultRatings[COS_INDEX] = new double*[NumUsers];
	ResultRatings[EUCL_INDEX] = new double*[NumUsers];
	ResultRatings[HAM_INDEX] = new double*[NumUsers];

	// allocate tables to store info about each user
	RealRatings = new bool*[NumUsers];			// the ratings he actually did
	MeanRatings = new double[NumUsers];			// the mean rating
	DataPoint = new bool[NumUsers];				// and whether he is to be processed (used for validation)

	for(int i = 0; i < NumUsers; i++){
		DataPoint[i] = false;					// initially, every user will be processed
		ResultRatings[COS_INDEX][i] = new double[NumItems];
		ResultRatings[EUCL_INDEX][i] = new double[NumItems];
		ResultRatings[HAM_INDEX][i] = new double[NumItems];
		RealRatings[i] = new bool[NumItems];
	}

}

void RecommendManager::estimateRating(int metric, int user, List<Point, Point*>* Neighbors){

	if( DataPoint[user] ){	// if the user participates in a train validation set
		return;				// skip him
	}

	int neighborNum = Neighbors->count();		// get the number of neighbors this user has

	if( neighborNum == 0 ){						// if no neighbors are found

		// set every unrated item to mean value of user ans return
		for(int item = 0; item < NumItems; item++){

			if( RealRatings[user][item] ){
				continue;
			}

			ResultRatings[metric][user][item] = MeanRatings[user];
		}
		return;
	}

	// now, procceed normally
	int* neighborIndexes = new int[neighborNum];	// stores the indexes of the neighbors in the global table
	double* neighborSim = new double[neighborNum];	// stores the similarity with the above neighbor

	// fill the above tables with the appropriate values
	int index = 0;
	for (Node<Point>* node = Neighbors->start() ; node != NULL; node = node->next(), index++ ) {
		int neighbor = (*PointMap[metric])[node->data()]->i;
		neighborIndexes[index] = neighbor;
		neighborSim[index] = PointTable[metric][user]->similarity( PointTable[metric][neighbor] );
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
			if( neighborIndexes[index] == user || !RealRatings[neighborIndexes[index]][item]){
				continue;
			}

			count++;								// another "usefull" neighbor was found for this item
			// follow the e-class definition
			sumRatings += neighborSim[index] * ResultRatings[metric][ neighborIndexes[index] ][item];
			sumWeights += abs(neighborSim[index]);
		}

		// if no "usefull" neighbors were found
		// of if our similarity is 0 (can't divide!)
		// then set to mean rating
		if( count == 0 || sumWeights == 0.0 ){
			ResultRatings[metric][user][item] = MeanRatings[user];
		}
		else{
			ResultRatings[metric][user][item] = MeanRatings[user] + (sumRatings / sumWeights);
		}
	}

	delete[] neighborIndexes;
	delete[] neighborSim;
}

void RecommendManager::run(std::string& dataPath, std::string& outPath){
	fillTable(dataPath);												// create the LSHT (as needed for every Point type)
	getPath( outPath,  "Please, enter the path for the output file" );	// promt the user for output file
	ofstream outFile;
	openFileWrite(outPath, outFile);									// open the above file

	// for(int metric = 0; metric < 3; metric++){
	// 	runTests(metric, outFile);
	// 	evaluate(metric, outFile, Messages[metric]);
	// }

	runTests(HAM_INDEX, outFile);
	evaluate(HAM_INDEX, outFile, Messages[HAM_INDEX]);


	// if( Validate ){
	// 	validate(outFile);
	// }

	finalise();
}

void RecommendManager::runCUTests(void){
	ClusterAlgorithm::runCUTests();
}

void RecommendManager::fillTable(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	getFileInfo(dataPath);

	PointTable[COS_INDEX] = new Point*[NumUsers];
	PointTable[EUCL_INDEX] = new Point*[NumUsers];
	PointTable[HAM_INDEX] = new Point*[NumUsers];

	PointMap[COS_INDEX] = new HashTable<PointIndex, Point*> (NumUsers / 16);
	PointMap[EUCL_INDEX] = new HashTable<PointIndex, Point*> (NumUsers / 16);
	PointMap[HAM_INDEX] = new HashTable<PointIndex, Point*> (NumUsers / 16);

	for(int i = 0; i < NumUsers; i++){			// read the whole file
		Point** temp = getNextPoint(file);		// store all the points
		PointTable[COS_INDEX][i] = temp[COS_INDEX];
		PointTable[EUCL_INDEX][i] = temp[EUCL_INDEX];
		PointTable[HAM_INDEX][i] = temp[HAM_INDEX];
		delete[] temp;

		PointMap[COS_INDEX]->insert( new PointIndex(i, PointTable[COS_INDEX][i]), true );
		PointMap[EUCL_INDEX]->insert( new PointIndex(i, PointTable[EUCL_INDEX][i]), true );
		PointMap[HAM_INDEX]->insert( new PointIndex(i, PointTable[HAM_INDEX][i]), true );
	}
}

void RecommendManager::evaluate(int metric, std::ofstream& outFile, string Message){

	// prints the top 5 predicted items
	int* itemIndexes = new int[NumItems];
	outFile << Message << endl;
	for(int i = 0; i < NumUsers; i++){

		// keep the indexes of items before sorting
		for( int j = 0; j < NumItems; j++ ){
			itemIndexes[j] = j;
		}

		Math::sort(ResultRatings[metric][i], itemIndexes, NumItems);
		outFile << i << " ";
		for(int j = 0, count = 0; j < NumItems && count < 5; j++ ){
			// print only predicted items
			if( !RealRatings[i][ itemIndexes[j] ] ){
				count++;
				outFile << itemIndexes[j] << " ";
			}
		}
		outFile << endl;
	}

	outFile << endl;
	outFile << endl;
	outFile << endl;
	delete[] itemIndexes;
}

void RecommendManager::validate(std::ofstream& outFile){
	int F = 10;										// 10- fold cross validation

	List<int> positions;
	for(int i = 0; i < NumUsers; i++){				// create a list that contains
		positions.insertRandom( new int(i), true);	// all users in random order
	}

	Partition* Partitions[F];						// create "F" partitions
	int validSize = NumUsers * (1.0) / F;			// the size of each partition
	for(int f = 0; f < F - 1; f++){					// the first "F-1" partitions
		Partitions[f] = new Partition(validSize, positions); //have fixed size
	}
	// the last one takes up the remaining
	Partitions[F-1] = new Partition(positions.count(), positions);

	// initialise "MAE" to 0
	for(int metric = 0; metric < 3; metric++){
		MAE[metric] = 0.0;
	}

	for(int f = 0; f < F; f++){ // for every validation fold

		// store initial results
		double** ResultRatingsCopy[3];					// table that will store the initial results of each metric
		for(int metric = 0; metric < 3; metric++){		// for every metric
			ResultRatingsCopy[metric] = new double*[Partitions[f]->size()];			// allocate a table big enough
			for(Index user(Partitions[f]); user < Partitions[f]->size(); user++){	// for every user
				ResultRatingsCopy[metric][user.index()] = new double[NumItems];
				for(int item = 0; item < NumItems; item++){							// store his ratings
					ResultRatingsCopy[metric][user.index()][item] = ResultRatings[metric][*user][item];
				}
			}
		}

		Partition DataSet(Partitions, F, f);			// create dataset out of the rest of the partitions
		// skip them in predictions
		for(Index user(&DataSet); user < DataSet.size(); user++){	// for every user in the data set
			DataPoint[*user] = true;								// remember not to process him
		}

		for(Index user(Partitions[f]); user < Partitions[f]->size(); user++){	// for every test-user
			for(int item = 0; item < NumItems; item++){							// flip his "real" ratings
				// so, predict the already rated for validation and skip the rest!
				RealRatings[*user][item] = !RealRatings[*user][item];
			}
		}

		double fMAE[3] = {0.0, 0.0, 0.0};
		for(int metric = 0; metric < 3; metric++){			// for every metric
			runTests(metric, outFile);						// perform validation tests

			for(Index user(Partitions[f]); user < Partitions[f]->size(); user++){	// for every test user
				// compute his share of MAE (of this partition)
				for(int item = 0; item < NumItems; item++){
					if( !RealRatings[*user][item] ){	// NOTE: NOT! skip the unpredicted!
						fMAE[metric] += abs( ResultRatingsCopy[metric][user.index()][item] - ResultRatings[metric][*user][item] );
					}
					ResultRatings[metric][*user][item] = ResultRatingsCopy[metric][user.index()][item];
				}
				delete[] ResultRatingsCopy[metric][user.index()];
			}
			delete[] ResultRatingsCopy[metric];
			fMAE[metric] /= Partitions[f]->size();
			MAE[metric] += fMAE[metric];
		}

		// reset the status of the dataset
		for(Index user(&DataSet); user < DataSet.size(); user++){
			DataPoint[*user] = false;
		}

		// and reset the real rating status of the test set
		for(Index user(Partitions[f]); user < Partitions[f]->size(); user++){
			for(int item = 0; item < NumItems; item++){
				RealRatings[*user][item] = !RealRatings[*user][item];
			}
		}
	}

	// compute the MAE of each metric
	for(int metric = 0; metric < 3; metric++){
		MAE[metric] /= F;
		outFile << Messages[metric] << " MAE: " << MAE[metric] << endl;
	}


	for(int f = 0; f < F; f++){
		delete Partitions[f];
	}
}

void RecommendManager::finalise(void){
	if( PointTable[0] != NULL ){
		for(int i = 0; i < NumUsers; i++){
			delete PointTable[COS_INDEX][i];
			delete PointTable[EUCL_INDEX][i];
			delete PointTable[HAM_INDEX][i];
		}
		delete[] PointTable[COS_INDEX];
		delete[] PointTable[EUCL_INDEX];
		delete[] PointTable[HAM_INDEX];

		PointTable[0] = NULL;

		for(int i = 0; i < NumUsers; i++){
			delete[] ResultRatings[COS_INDEX][i];
			delete[] ResultRatings[EUCL_INDEX][i];
			delete[] ResultRatings[HAM_INDEX][i];
			delete[] RealRatings[i];
		}
		delete[] ResultRatings[COS_INDEX];
		delete[] ResultRatings[EUCL_INDEX];
		delete[] ResultRatings[HAM_INDEX];

		delete[] RealRatings;
		delete[] MeanRatings;
		delete[] DataPoint;

		delete PointMap[COS_INDEX];
		delete PointMap[EUCL_INDEX];
		delete PointMap[HAM_INDEX];
	}
}

RecommendManager::~RecommendManager(){
	finalise();
}

/*****************************************************************
***************** NNRecommendManager class methods *********************
******************************************************************/

NNRecommendManager::NNRecommendManager(bool validate)
: RecommendManager(validate) {
	L_hash = 5;
	K_hash = 5;
	barrier = 3;
	P = 50;

	Messages[COS_INDEX] = "Cosine LSH";
	Messages[EUCL_INDEX] = "Euclidean LSH";
	Messages[HAM_INDEX] = "Hamming LSH";
}

void NNRecommendManager::fillTable(std::string dataPath){
	RecommendManager::fillTable(dataPath);

	int TableSize = 0;

	TableSize = 1 << K_hash;								// 2^K slots in the Hash Tables
	hashFunctions[COS_INDEX] = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[COS_INDEX][i] = new Cosine_g( K_hash, NumItems );	// and initialise it
	}
	LSH[COS_INDEX] = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions[COS_INDEX], false );	// create the LSHT (false: not Euclidean!)

	TableSize = NumUsers / 4;
	hashFunctions[EUCL_INDEX] = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[EUCL_INDEX][i] = new Euclidean_g(TableSize, K_hash, NumItems, 4);
	}
	LSH[EUCL_INDEX] = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions[EUCL_INDEX], true );	// create the LSHT (false: not Euclidean!)


	TableSize = 1 << K_hash;
	hashFunctions[HAM_INDEX] = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[HAM_INDEX][i] = new Hamming_g( K_hash, NumItems );	// and initialise it
	}
	LSH[HAM_INDEX] = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions[HAM_INDEX], false );	// create the LSHT (false: not Euclidean!)


	for(int i = 0; i < NumUsers; i++){
		LSH[COS_INDEX]->insert( PointTable[COS_INDEX][i] );					// store all the points
		LSH[EUCL_INDEX]->insert( PointTable[EUCL_INDEX][i] );				// store all the points
		LSH[HAM_INDEX]->insert( PointTable[HAM_INDEX][i] );					// store all the points
	}
}

double NNRecommendManager::getRadius(int metric){
	if( metric == COS_INDEX ){
		return 0.5;
	}
	else if( metric == HAM_INDEX ){
		// mean "true" NumItems is 10, so approximately 5 will differ
		return 5;
	}
	else{
		// mean diff is 3 (sqr -> 9)
		// mean "true" NumItems is 10, so approximately 20 when calculating distance
		return 180.0;
	}
}

void NNRecommendManager::runTests(int metric, std::ofstream& outFile){
	for(int i = 0; i < NumUsers; i++){
		List<Point, Point*>* Neighbors;
		Neighbors = findNeighbours(metric, i);
		estimateRating(metric, i, Neighbors);
		delete Neighbors;
	}
}

List<Point, Point*>* NNRecommendManager::findNeighbours(int metric, int user){
	if( DataPoint[user] ){
		return new List<Point, Point*>();
	}

	std::cout << "Examine Neighbors of user " << user << std::endl;


	Point* point = PointTable[metric][user];					// get the point of global table

	// // TODO
	// if( metric == HAM_INDEX ){
	// 	return new List<Point, Point*>();
	// }

	int times = 0;												// number of times I performed NN query
	int BarrierTimes = 3;										// max nubmer ot times I permit
	int Tolerance = P / 4;										// don't expect to get exactly P neigbors!

	// big and small radius for the queries
	double RSmall = getRadius(metric), RBig = getRadius(metric);

	// lists that hold neighbors for big and small radius
	List<Point, Point*> *ResultPointsSmall = new List<Point, Point*>(), *ResultPointsBig = new List<Point, Point*>();

	clock_t begin = clock();
	LSH[metric]->inRange( point, RSmall, *ResultPointsSmall );	// perform first query
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;


	// if( metric == HAM_INDEX ){	// TODO
	// 	std::cout << "time of inRange() = " << time_spent << " secs." << std::endl;
	// 	std::cout << "RSmall = " << RSmall << std::endl;
	// 	std::cout << "count = " << ResultPointsSmall->count() << std::endl;
	// 	exit(0);
	// }


	if( abs(ResultPointsSmall->count() - P) < Tolerance ){		// if the total number is OK
		delete ResultPointsBig;									// retun immediately
		return ResultPointsSmall;
	}
	else if( ResultPointsSmall->count() < P ){					// else, if too few neigbors were found
		// I have to look for the initial value of RBig
		do{
			ResultPointsBig->flush();
			RBig = PointTable[metric][0]->multiplyDouble(RBig, 2.0);	// double its value
			LSH[metric]->inRange( point, RBig, *ResultPointsBig );		// NN query
			times++;
		} while( ResultPointsBig->count() < P && times < BarrierTimes );// until I am satisfied

		if( abs(ResultPointsBig->count() - P) < Tolerance || times == BarrierTimes ){	// if OK, return
			delete ResultPointsSmall;
			return ResultPointsBig;
		}
		else{ 	// else, more queries will follow
			RSmall = PointTable[metric][0]->multiplyDouble(RBig, 0.5);		// small = big / 2
		}
	}
	else{														// else, if too many neigbors were found
		// I have to look for the initial value of RSmall
		do{
			ResultPointsSmall->flush();
			RSmall = PointTable[metric][0]->multiplyDouble(RSmall, 0.5);	// halve its value
			LSH[metric]->inRange( point, RSmall, *ResultPointsSmall );		// NN query
			times++;
		} while( ResultPointsSmall->count() > P && times < BarrierTimes );	// until I am satisfied

		if(  abs(ResultPointsSmall->count() - P) < Tolerance || times == BarrierTimes ){ // if OK, return
			delete ResultPointsBig;
			return ResultPointsSmall;
		}
		else{	// else, more queries will follow
			RBig = PointTable[metric][0]->multiplyDouble(RSmall, 2.0);		// big = small * 2
		}
	}

	// now, perform queries a la binary search
	do {
		double RMean = (RSmall + RBig) / 2.0;
		List<Point, Point*> *ResultPointsMean = new List<Point, Point*>();
		LSH[metric]->inRange( point, RMean, *ResultPointsMean );

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

NNRecommendManager::~NNRecommendManager(){
	delete LSH[COS_INDEX];
	delete LSH[EUCL_INDEX];
	delete LSH[HAM_INDEX];
	for(int i = 0; i < L_hash; i++){
		delete hashFunctions[COS_INDEX][i];
		delete hashFunctions[EUCL_INDEX][i];
		delete hashFunctions[HAM_INDEX][i];
	}
	delete[] hashFunctions[COS_INDEX];
	delete[] hashFunctions[EUCL_INDEX];
	delete[] hashFunctions[HAM_INDEX];
}


// /*****************************************************************
// ***************** dRMSDManager class methods *********************
// ******************************************************************/

ClusterRecommendManager::ClusterRecommendManager(bool validate)
: RecommendManager(validate) {
	Messages[COS_INDEX] = "Cosine Clustering";
	Messages[EUCL_INDEX] = "Euclidean Clustering";
	Messages[HAM_INDEX] = "Hamming Clustering";
}

void ClusterRecommendManager::fillTable(std::string dataPath){
	RecommendManager::fillTable(dataPath);

	// find the proper algorithm for every metric
	for(int metric = 0; metric < 3; metric++){
		findAlgorithm(metric);
	}
}

void ClusterRecommendManager::findAlgorithm(int metric){

	Algorithm[metric] = NULL;										// initially, no algorithm was picked
	d[metric] = new TriangularMatrix(NumUsers, PointTable[metric]);	// initialise the distance matrix

	ofstream dummyFile("");											// needed for function call
	double bestScore = -2.0;										// initialise "bestScore" to be small enough (Silhouette)
	int MaxK = log2(NumUsers) * 2;									// max # of clusters I will check

	std::cout << metricName(metric) << ": Ready to select optimal K in [2, " << MaxK - 1 << "] through Silhouette..." << '\n';
	for( int clusters = 2; clusters < MaxK; clusters++){			// for every possible # of clusters

		if( (clusters - 1) % 15 == 0 ){
			std::cout << "Done " << clusters - 1 << " iterations..." << '\n';
		}

		// pick an algorithm (Park-Jun, PAM-Swap, Lloyd's)
		ClusterAlgorithm* tempClustering = new ClusterAlgorithm(PointTable[metric], d[metric], NumUsers, clusters, 0, -1, -1, -1, -1);
		tempClustering->run();										// run the algorithm
		double tempScore = tempClustering->evaluate(dummyFile, false, true);	// get his score
		if( tempScore > bestScore ){								// if its good enough
			bestScore = tempScore;									// keep the score
			K_clusters[metric] = clusters;							// keep the clusters
			if( Algorithm[metric] != NULL ){
				delete Algorithm[metric];
			}
			Algorithm[metric] = tempClustering;						// and keep the algorithm
		}
		else{
			delete tempClustering;
		}
	}

	// std::cout << "clusters[" << metric << "] = " << K_clusters[metric] << '\n';
	// for(int i = 0; i < K_clusters[metric]; i++){
	// 	std::cout << "i = " << i << ", size = " << Algorithm[metric]->getCluster(i)->count() << '\n';
	// }
	// std::cout << '\n' << '\n' << '\n';
}

void ClusterRecommendManager::runTests(int metric, std::ofstream& outFile){
	for(int k = 0; k < K_clusters[metric]; k++){
		List<AssignPair>* cluster = Algorithm[metric]->getCluster(k);
		List<Point, Point*>* Neighbors = findNeighbours(metric, cluster);
		for (Node<AssignPair>* node = cluster->start() ; node != NULL; node = node->next() ) {
			estimateRating(metric, node->data()->assigned(), Neighbors);
		}
		delete Neighbors;
	}
}

List<Point, Point*>* ClusterRecommendManager::findNeighbours(int metric, List<AssignPair>* cluster){

	// the list of neighbors is essentially the cluster itself
	// the cluster holds more info, need to keep what I want
	List<Point, Point*>* Neighbors = new List<Point, Point*>();
	for (Node<AssignPair>* node = cluster->start() ; node != NULL; node = node->next() ) {
		Neighbors->insertAtEnd( PointTable[metric][node->data()->assigned()], false );
	}

	return Neighbors;
}

ClusterRecommendManager::~ClusterRecommendManager(){
	delete d[COS_INDEX];
	delete d[EUCL_INDEX];
	delete d[HAM_INDEX];

	delete Algorithm[COS_INDEX];
	delete Algorithm[EUCL_INDEX];
	delete Algorithm[HAM_INDEX];
}
