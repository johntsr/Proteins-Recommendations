#include "RecommendManager.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Clusters/Assigner.h"			// need for Hash Table (point index)!

#include <sstream>
#include <float.h>

using namespace std;

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

RecommendManager::RecommendManager(bool complete) {
	NumUsers = 0;
	NumItems = 0;
	PointTable = NULL;
	Complete = complete;
	// BestTime = 0.0;
	// K_clusters = 0;
}

void RecommendManager::getFileInfo(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	NumUsers = -1;
	int tempUsers = 0, tempItem = 0, rating;
	int maxItem = -1;

	while( file >> tempUsers ){

		if( tempUsers != NumUsers ){
			Ratings.insertAtEnd( new List<Pair>(), true );
		}

		NumUsers = tempUsers;

		file >> tempItem;
		if(tempItem > maxItem){
			maxItem = tempItem;
		}

		file >> rating;

		Ratings.end()->data()->insertAtEnd( new Pair(tempItem, rating), true );
	}

	NumItems = maxItem;

	ResultRatings = new double*[NumUsers];
	RealRatings = new bool*[NumUsers];
	MeanRatings = new double[NumUsers];
	for(int i = 0; i < NumUsers; i++){
		ResultRatings[i] = new double[NumItems];
		RealRatings[i] = new bool[NumItems];
	}
}

void RecommendManager::run(std::string& dataPath, std::string& outPath){
	fillTable(dataPath);												// create the LSHT (as needed for every Point type)
	getPath( outPath,  "Please, enter the path for the output file" );	// promt the user for output file
	ofstream outfFile;
	openFileWrite(outPath, outfFile);									// open the above file
	runTests(outfFile);
	finalise();
}

void RecommendManager::runCUTests(void){
	ClusterAlgorithm::runCUTests();
}

void RecommendManager::fillTable(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	getFileInfo(dataPath);

	PointTable = new Point*[NumUsers];
	for(int i = 0; i < NumUsers; i++){			// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
	}
}

void RecommendManager::finalise(void){
	if( PointTable != NULL ){
		for(int i = 0; i < NumUsers; i++){
			delete PointTable[i];
		}
		delete[] PointTable;

		// if( Algorithm != NULL ){
		// 	delete Algorithm;
		// }

		// delete d;
		PointTable = NULL;

		for(int i = 0; i < NumUsers; i++){
			delete[] ResultRatings[i];
			delete[] RealRatings[i];
		}
		delete[] ResultRatings;
		delete[] RealRatings;

		delete[] MeanRatings;
	}
}

RecommendManager::~RecommendManager(){
	finalise();
}

/*****************************************************************
***************** NNRecommendManager class methods *********************
******************************************************************/

NNRecommendManager::NNRecommendManager(bool complete)
: RecommendManager(complete) {
	L_hash = 5;
	K_hash = 5;
	barrier = 10;
	P = 30;

	PointIndex::PointTable = PointTable;
}

void NNRecommendManager::fillTable(std::string dataPath){
	RecommendManager::fillTable(dataPath);

	int TableSize = 1 << K_hash;								// 2^K slots in the Hash Tables

	PointMap = new HashTable<PointIndex, Point*> (TableSize);

	hashFunctions = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[i] = new Cosine_g( K_hash, NumItems );	// and initialise it
	}

	// create the LSHT (false: not Euclidean!)
	LSH = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)

	for(int i = 0; i < NumUsers; i++){
		LSH->insert( PointTable[i] );					// store all the points
		PointMap->insert( new PointIndex(i), true );
	}
}

Point* NNRecommendManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	static int count = -1;

	stringstream name;
	count++;
	name << count + 1;

	List<Pair>* ratingList = Ratings.removeAtStart();

	double mean = 0.0;
	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		mean += node->data()->Rating;
	}
	mean /= ratingList->count();
	MeanRatings[count] = mean;

	double* coordinates = new double[NumItems];
	for(int i = 0; i < NumItems; i++){
		coordinates[i] = 0;
		ResultRatings[count][i] = DBL_MAX;
		RealRatings[count][i] = false;
	}

	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		coordinates[node->data()->Item] = node->data()->Rating - mean;
		ResultRatings[count][node->data()->Item] = coordinates[node->data()->Item];
		RealRatings[count][node->data()->Item] = true;
	}

	Point* temp = new CosinePoint( name.str(), coordinates, NumItems);

	delete[] coordinates;
	delete ratingList;

	return temp;
}

Quantity* NNRecommendManager::getRadius(void){
	return new Quantity(0.5);
}

List<Point, Point*>* NNRecommendManager::findNeighbours(Point* point){

	int times = 0;
	int BarrierTimes = 5;

	List<Point, Point*> *ResultPointsSmall, *ResultPointsBig;
	Quantity *RSmall, *RBig;

	ResultPointsSmall = new List<Point, Point*>();
	ResultPointsBig = new List<Point, Point*>();
	RSmall = getRadius();
	RBig = getRadius();
	LSH->inRange( point, RSmall, *ResultPointsSmall );

	if( ResultPointsSmall->count() == P ){
		delete RSmall;
		delete RBig;
		delete ResultPointsBig;
		return ResultPointsSmall;
	}
	else if( ResultPointsSmall->count() < P ){

		do{
			ResultPointsBig->flush();
			RBig->multiply(2.0);
			LSH->inRange( point, RBig, *ResultPointsBig );
			times++;
		} while( ResultPointsBig->count() < P && times < BarrierTimes );

		if( ResultPointsBig->count() == P || times == BarrierTimes ){
			delete RBig;
			delete RSmall;
			delete ResultPointsSmall;
			return ResultPointsBig;
		}

		RSmall->copy(RBig);
		RSmall->multiply(0.5);
	}
	else{

		do{
			ResultPointsSmall->flush();
			RSmall->multiply(0.5);
			LSH->inRange( point, RSmall, *ResultPointsSmall );
			times++;
		} while( ResultPointsSmall->count() > P && times < BarrierTimes );

		if( ResultPointsSmall->count() == P || times == BarrierTimes ){
			delete RSmall;
			delete RBig;
			delete ResultPointsBig;
			return ResultPointsSmall;
		}

		RBig->copy(RSmall);
		RBig->multiply(2.0);
	}


	do {
		Quantity* RMean = RSmall->mean(RBig);
		List<Point, Point*> *ResultPointsMean = new List<Point, Point*>();
		LSH->inRange( point, RMean, *ResultPointsMean );

		if( ResultPointsMean->count() == P || times == BarrierTimes ){
			delete RSmall;
			delete RMean;
			delete RBig;
			delete ResultPointsBig;
			delete ResultPointsSmall;
			return ResultPointsMean;
		}
		else if( ResultPointsMean->count() < P ){
			delete RSmall;
			RSmall = RMean;
			delete ResultPointsSmall;
			ResultPointsSmall = ResultPointsMean;
		}
		else{
			delete RBig;
			RBig = RMean;
			delete ResultPointsBig;
			ResultPointsBig = ResultPointsMean;
		}

		times++;
		delete ResultPointsMean;

	} while(true);
}

double NNRecommendManager::estimateRating(int user, List<Point, Point*>* Neighbors, int item){
	double sumRatings = 0.0, sumWeights = 0.0;
	for (Node<Point>* node = Neighbors->start() ; node != NULL; node = node->next() ) {
		int neighbor = (*PointMap)[node->data()]->i;

		Quantity* sim = PointTable[user]->similarity( PointTable[neighbor] );
		sumWeights += sim->getDouble();
		if( RealRatings[neighbor][item] ){
			sumRatings += sim->getDouble() * ResultRatings[neighbor][item];
		}

		delete sim;
	}

	return MeanRatings[user] + (sumRatings / sumWeights);
}

void NNRecommendManager::runTests(std::ofstream& outFile){

	for(int i = 0; i < NumUsers; i++){
		List<Point, Point*>* Neighbors = findNeighbours(PointTable[i]);
		for(int j = 0; j < NumItems; j++){
			if( ResultRatings[i][j] < DBL_MAX ){
				continue;
			}

			ResultRatings[i][j] = estimateRating(i, Neighbors, j);
		}
		delete Neighbors;
	}
	evaluate(outFile);

}

void NNRecommendManager::evaluate(std::ofstream& outFile){
	outFile << "Cosine LSH" << endl;
	int* itemIndexes = new int[NumItems];
	for(int i = 0; i < NumUsers; i++){
		for( int j = 0; j < NumItems; j++ ){
			itemIndexes[j] = j;
		}

		Math::sort(ResultRatings[i], itemIndexes, NumItems);
		outFile << i << " ";
		for(int j = 0, count = 0; j < NumItems && count < 5; j++ ){
			if( !RealRatings[i][ itemIndexes[j] ] ){
				count++;
				outFile << itemIndexes[j] << " ";
			}
		}
		outFile << endl;
	}
	delete[] itemIndexes;
}

NNRecommendManager::~NNRecommendManager(){
	delete LSH;
	for(int i = 0; i < L_hash; i++){
		delete hashFunctions[i];
	}
	delete[] hashFunctions;
	delete PointMap;
}


/*****************************************************************
***************** dRMSDManager class methods *********************
******************************************************************/

// int dRMSDManager::R = 0;
// bool dRMSDManager::firstTime = true;
// Pair* dRMSDManager::Indexes = NULL;
//
// double* dRMSDManager::Configuration = NULL;
//
// dRMSDManager::dRMSDManager(dOption t, rGenerator func, bool complete)
// : RecommendManager(complete) {
// 	Func = func;
// 	T = t;
// }
//
// Point* dRMSDManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
// 	static int count = -1;
//
// 	stringstream name;
// 	count++;
// 	name << count + 1;
// 	stringstream rCoordinates;
//
// 	if( Configuration == NULL ){
// 		Configuration = new double[numConform*N*3];
// 	}
//
// 	if( count < numConform ){
// 		for(int i = 0; i < 3*N; i++){
// 			queryFile >> Configuration[N*3*count + i];
// 		}
// 	}
//
//
// 	if( firstTime ){
// 		firstTime = false;
// 		R = Func(N);
// 		Indexes = new Pair[R];
//
//
// 		if( T == SMALLEST || T == LARGEST ){
// 			int DistLength = N * (N - 1) / 2;
// 			Pair* tempIndexes = new Pair[DistLength];
// 			double* Distances = new double[DistLength];
//
// 			for(int i = 0, d = 0; i < 3*N; i += 3){
// 				for(int j = 0; j < i; j += 3, d++){
// 					tempIndexes[d].i = i;
// 					tempIndexes[d].j = j;
//
// 					double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
// 					dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
// 					dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
// 					Distances[d] = dist;
// 				}
// 			}
//
// 			Math::sort(Distances, tempIndexes, DistLength);
//
// 			if( T == SMALLEST ){
// 				for(int r = 0; r < R; r++){
// 					Indexes[r] = tempIndexes[r];
// 					rCoordinates << Distances[r] << " ";
// 				}
// 			}
// 			else{
// 				for(int r = DistLength - R; r < DistLength; r++){
// 					Indexes[r - (DistLength - R) ] = tempIndexes[r];
// 					rCoordinates << Distances[r] << " ";
// 				}
// 			}
//
// 			delete[] tempIndexes;
// 			delete[] Distances;
// 		}
// 		else{
// 			for( int r = 0; r < R; r++ ){
// 				int i = ( (int)Math::dRand(0,N) ) * 3;
// 				int j = ( (int)Math::dRand(0,N) ) * 3;
// 				Indexes[r].i = i;
// 				Indexes[r].j = j;
//
// 				double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
// 				dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
// 				dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
// 				rCoordinates << dist << " ";
// 			}
// 		}
// 	}
// 	else{
// 		for( int r = 0; r < R; r++ ){
// 			int i = N*3*(count % numConform) + Indexes[r].i;
// 			int j = N*3*(count % numConform) + Indexes[r].j;
//
// 			double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
// 			dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
// 			dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
// 			rCoordinates << dist << " ";
// 		}
// 	}
//
// 	string buffer = rCoordinates.str();
// 	return new EuclideanPoint(name.str(), buffer, R);
// }
//
// void dRMSDManager::evaluate(std::ofstream& outfFile){
// 	double Silhouette = Algorithm->evaluate(outfFile, Complete, false);
// 	outfFile 	<< "r: " << Func(N) << " , T = " << T << " , k = " << K_clusters
// 				<< ", silhouette = " << Silhouette << " execution time = " << BestTime << " secs." << endl;
// }
//
// dRMSDManager::~dRMSDManager(){
// 	if( Indexes != NULL ){
// 		delete[] Indexes;
// 		Indexes = NULL;
// 	}
//
// 	if( Configuration != NULL ){
// 		delete[] Configuration;
// 		Configuration = NULL;
// 	}
// }
