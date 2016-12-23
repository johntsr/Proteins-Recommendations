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

Point* RecommendManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
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


	// for (Node<List<Pair> >* list = Ratings.start() ; list != NULL; list = list->next() ) {
	// 	std::cout << "A user is:" << std::endl << std::endl;
	// 	for (Node<Pair>* node = list->data()->start() ; node != NULL; node = node->next() ) {
	// 		std::cout << node->data()->Item << " - " << node->data()->Rating << std::endl;
	// 	}
	// }
	// std::cout << "NumUsers = " << NumUsers << std::endl;
	// std::cout << "NumItems = " << NumItems << std::endl;
	// exit(0);

}

void RecommendManager::estimateRating(int user, List<Point, Point*>* Neighbors){

	int neighborNum = Neighbors->count();

	if( neighborNum == 0){
		for(int item = 0; item < NumItems; item++){
			ResultRatings[user][item] = MeanRatings[user];
		}
		return;
	}

	int* neighborIndexes = new int[neighborNum];
	double* neighborSim = new double[neighborNum];
	int index = 0;
	for (Node<Point>* node = Neighbors->start() ; node != NULL; node = node->next(), index++ ) {
		int neighbor = (*PointMap)[node->data()]->i;
		neighborIndexes[index] = neighbor;
		Quantity* temp = PointTable[user]->similarity( PointTable[neighbor] );
		neighborSim[index] = temp->getDouble();
		delete temp;
	}

	for(int item = 0; item < NumItems; item++){

		if( ResultRatings[user][item] < DBL_MAX ){
			continue;
		}

		int count = 0;
		double sumRatings = 0.0, sumWeights = 0.0;
		for (index = 0 ; index < neighborNum; index++ ) {

			if( neighborIndexes[index] == user ){
				continue;
			}

			if( RealRatings[ neighborIndexes[index] ][item] ){
				count++;
				sumRatings += neighborSim[index] * ResultRatings[ neighborIndexes[index] ][item];
				if( neighborSim[index] > 0.0 ){
					sumWeights += neighborSim[index];	// TODO
				}
				else{
					sumWeights += -1.0 * neighborSim[index];	// TODO
				}
			}
		}

		if( count == 0){
			ResultRatings[user][item] = MeanRatings[user];
		}
		else{
			ResultRatings[user][item] = MeanRatings[user] + (sumRatings / sumWeights);
		}
	}

	delete[] neighborIndexes;
	delete[] neighborSim;
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
	PointMap = new HashTable<PointIndex, Point*> (NumUsers / 16);
	for(int i = 0; i < NumUsers; i++){			// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
		PointMap->insert( new PointIndex(i, PointTable[i]), true );
	}

}

void RecommendManager::evaluate(std::ofstream& outFile, string Message){
	outFile << Message << endl;
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

void RecommendManager::finalise(void){
	if( PointTable != NULL ){
		for(int i = 0; i < NumUsers; i++){
			delete PointTable[i];
		}
		delete[] PointTable;
		PointTable = NULL;

		for(int i = 0; i < NumUsers; i++){
			delete[] ResultRatings[i];
			delete[] RealRatings[i];
		}
		delete[] ResultRatings;
		delete[] RealRatings;
		delete[] MeanRatings;
		delete PointMap;
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
	L_hash = 10;
	K_hash = 3;
	barrier = 3;
	P = 50;
}

void NNRecommendManager::fillTable(std::string dataPath){
	RecommendManager::fillTable(dataPath);

	int TableSize = 1 << K_hash;								// 2^K slots in the Hash Tables

	hashFunctions = new hash_function*[L_hash];					// allocate 1 for each Hash Table
	for( int i = 0; i < L_hash; i++){
		hashFunctions[i] = new Cosine_g( K_hash, NumItems );	// and initialise it
	}

	// create the LSHT (false: not Euclidean!)
	LSH = new LocalHashTable<Point, Point*> (barrier, L_hash, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)

	for(int i = 0; i < NumUsers; i++){
		LSH->insert( PointTable[i] );					// store all the points
	}
}

Quantity* NNRecommendManager::getRadius(void){
	return new Quantity(0.5);
}

void NNRecommendManager::runTests(std::ofstream& outFile){

	for(int i = 0; i < NumUsers; i++){
		std::cout << "Examine Neighbors of user " << i << std::endl;

		clock_t begin = clock();
		List<Point, Point*>* Neighbors = findNeighbours(PointTable[i]);
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		std::cout << "time spent for findNeighbours() = " << time_spent << " secs." << std::endl;

		begin = clock();
		estimateRating(i, Neighbors);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		std::cout << "time spent for estimateRating() = " << time_spent << " secs." << std::endl;


		delete Neighbors;
	}
	evaluate(outFile, "Cosine LSH");

}

int abs(int x){
	if( x < 0 ){
		return -x;
	}
	return x;
}

List<Point, Point*>* NNRecommendManager::findNeighbours(Point* point){

	int times = 0;
	int BarrierTimes = 3;
	int Tolerance = P / 4;

	List<Point, Point*> *ResultPointsSmall, *ResultPointsBig;
	Quantity *RSmall, *RBig;

	ResultPointsSmall = new List<Point, Point*>();
	ResultPointsBig = new List<Point, Point*>();
	RSmall = getRadius();
	RBig = getRadius();

	clock_t begin = clock();
	LSH->inRange( point, RSmall, *ResultPointsSmall );
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	std::cout << "time spent for first inRange() = " << time_spent << " secs." << std::endl;



	// std::cout << "initially, RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;

	// if( ResultPointsSmall->count() == P ){
	if( abs(ResultPointsSmall->count() - P) < Tolerance ){
		delete RSmall;
		delete RBig;
		delete ResultPointsBig;
		return ResultPointsSmall;
	}
	else if( ResultPointsSmall->count() < P ){

		// std::cout << "try to find RBig" << std::endl;

		do{
			ResultPointsBig->flush();
			RBig->multiply(2.0);
			LSH->inRange( point, RBig, *ResultPointsBig );
			times++;
			// std::cout << "RBig = " << RBig->getDouble() << " -> " << ResultPointsBig->count() << std::endl;
		} while( ResultPointsBig->count() < P && times < BarrierTimes );

		// if( ResultPointsBig->count() == P || times == BarrierTimes ){
		if( abs(ResultPointsBig->count() - P) < Tolerance || times == BarrierTimes ){
			delete RBig;
			delete RSmall;
			delete ResultPointsSmall;
			return ResultPointsBig;
		}

		RSmall->setDouble(RBig->getDouble());
		RSmall->multiply(0.5);
	}
	else{

		// std::cout << "try to find RSmall" << std::endl;


		do{
			ResultPointsSmall->flush();
			RSmall->multiply(0.5);
			LSH->inRange( point, RSmall, *ResultPointsSmall );
			times++;
			// std::cout << "RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;
		} while( ResultPointsSmall->count() > P && times < BarrierTimes );

		// if( ResultPointsSmall->count() == P || times == BarrierTimes ){
		if(  abs(ResultPointsSmall->count() - P) < Tolerance || times == BarrierTimes ){
			delete RSmall;
			delete RBig;
			delete ResultPointsBig;
			return ResultPointsSmall;
		}

		RBig->setDouble(RSmall->getDouble());
		RBig->multiply(2.0);
	}

	// std::cout  << std::endl << std::endl;

	do {
		Quantity* RMean = RSmall->mean(RBig);
		List<Point, Point*> *ResultPointsMean = new List<Point, Point*>();
		LSH->inRange( point, RMean, *ResultPointsMean );

		// std::cout << std::endl<< std::endl;

		// std::cout << "NOW, RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;
		// std::cout << "NOW, RBig = " << RBig->getDouble() << " -> " << ResultPointsBig->count() << std::endl;
		// std::cout << "NOW, RMean = " << RMean->getDouble() << " -> " << ResultPointsMean->count() << std::endl;


		// if( ResultPointsMean->count() == P || times == BarrierTimes ){
		if( abs(ResultPointsMean->count() - P) < Tolerance || times == BarrierTimes ){
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

	} while(true);
}

NNRecommendManager::~NNRecommendManager(){
	delete LSH;
	for(int i = 0; i < L_hash; i++){
		delete hashFunctions[i];
	}
	delete[] hashFunctions;
}


/*****************************************************************
***************** dRMSDManager class methods *********************
******************************************************************/

ClusterRecommendManager::ClusterRecommendManager(bool complete)
: RecommendManager(complete) {
	K_clusters = 0;
	d = NULL;
	Algorithm = NULL;
}

void ClusterRecommendManager::fillTable(std::string dataPath){
	RecommendManager::fillTable(dataPath);

	d = new TriangularMatrix(NumUsers, PointTable);

	double bestScore = 0.0;
	ofstream outFile("");
	int UpperBound = log2(NumUsers) * 5;
	for( int clusters = 2; clusters < UpperBound; clusters++){
		ClusterAlgorithm* tempClustering = new ClusterAlgorithm(PointTable, d, NumUsers, clusters, 4, -1, -1, -1, -1);

		tempClustering->run();

		double tempScore = tempClustering->evaluate(outFile, Complete, false);
		if( tempScore < bestScore ){
			bestScore = tempScore;
			K_clusters = clusters;

			if( Algorithm != NULL ){
				delete Algorithm;
			}
			Algorithm = tempClustering;
		}
		else{
			delete tempClustering;
		}
	}

}

void ClusterRecommendManager::runTests(std::ofstream& outFile){

	for(int k = 0; k < K_clusters; k++){
		List<AssignPair>* cluster = Algorithm->getCluster(k);
		List<Point, Point*>* Neighbors = findNeighbours(cluster);

		for (Node<AssignPair>* node = cluster->start() ; node != NULL; node = node->next() ) {
			std::cout << "Examine Neighbors of user " << node->data()->assigned() << std::endl;
			clock_t begin = clock();
			estimateRating(node->data()->assigned(), Neighbors);
			clock_t end = clock();
			double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			std::cout << "time spent for estimateRating() = " << time_spent << " secs." << std::endl;
		}
		delete Neighbors;
	}

	evaluate(outFile, "Clustering");

}

List<Point, Point*>* ClusterRecommendManager::findNeighbours(List<AssignPair>* cluster){
	List<Point, Point*>* Neighbors = new List<Point, Point*>();
	for (Node<AssignPair>* node = cluster->start() ; node != NULL; node = node->next() ) {
		Neighbors->insertAtEnd( PointTable[node->data()->assigned()], false );
	}

	return Neighbors;
}

ClusterRecommendManager::~ClusterRecommendManager(){
	delete Algorithm;
	delete d;
}
