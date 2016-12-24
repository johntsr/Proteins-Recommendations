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

Point** RecommendManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
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

	for(int i = 0; i < NumItems; i++){
		ResultRatings[COS_INDEX][count][i] = DBL_MAX;
		ResultRatings[EUCL_INDEX][count][i] = DBL_MAX;
		ResultRatings[HAM_INDEX][count][i] = DBL_MAX;
		RealRatings[count][i] = false;
	}

	for (Node<Pair>* node = ratingList->start() ; node != NULL; node = node->next() ) {
		ResultRatings[COS_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		ResultRatings[EUCL_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		ResultRatings[HAM_INDEX][count][node->data()->Item] = node->data()->Rating - mean;
		RealRatings[count][node->data()->Item] = true;
	}

	// Point* temp = new CosinePoint( name.str(), coordinates, NumItems);
	Point** temp = new Point*[3];
	temp[COS_INDEX] = new CosinePointSparse( name.str(), ratingList);
	temp[EUCL_INDEX] = new EuclideanPointSparse( name.str(), ratingList);
	temp[HAM_INDEX] = new HammingPoint( name.str(), ratingList, NumItems);

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

		Ratings.end()->data()->insertAtEnd( new Pair(tempItem - 1, rating), true );
	}

	NumItems = maxItem;

	ResultRatings[COS_INDEX] = new double*[NumUsers];
	ResultRatings[EUCL_INDEX] = new double*[NumUsers];
	ResultRatings[HAM_INDEX] = new double*[NumUsers];

	RealRatings = new bool*[NumUsers];
	MeanRatings = new double[NumUsers];
	for(int i = 0; i < NumUsers; i++){
		ResultRatings[COS_INDEX][i] = new double[NumItems];
		ResultRatings[EUCL_INDEX][i] = new double[NumItems];
		ResultRatings[HAM_INDEX][i] = new double[NumItems];
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

void RecommendManager::estimateRating(int metric, int user, List<Point, Point*>* Neighbors){
	int neighborNum = Neighbors->count();

	if( neighborNum == 0 ){
		for(int item = 0; item < NumItems; item++){
			ResultRatings[metric][user][item] = MeanRatings[user];
		}
		return;
	}

	int* neighborIndexes = new int[neighborNum];
	double* neighborSim = new double[neighborNum];
	int index = 0;
	for (Node<Point>* node = Neighbors->start() ; node != NULL; node = node->next(), index++ ) {
		int neighbor = (*PointMap[metric])[node->data()]->i;
		neighborIndexes[index] = neighbor;
		neighborSim[index] = PointTable[metric][user]->similarity( PointTable[metric][neighbor] );
	}

	for(int item = 0; item < NumItems; item++){

		if( RealRatings[user][item] ){
			continue;
		}

		int count = 0;
		double sumRatings = 0.0, sumWeights = 0.0;
		for (index = 0 ; index < neighborNum; index++ ) {

			if( neighborIndexes[index] == user || ResultRatings[metric][user][item] == DBL_MAX ){
				continue;
			}

			count++;
			sumRatings += neighborSim[index] * ResultRatings[metric][ neighborIndexes[index] ][item];
			if( neighborSim[index] > 0.0 ){
				sumWeights += neighborSim[index];	// TODO
			}
			else{
				sumWeights += -1.0 * neighborSim[index];	// TODO
			}

		}

		if( count == 0 || sumWeights == 0.0 ){
			ResultRatings[metric][user][item] = MeanRatings[user];
		}
		else{
			// if( user == 0 ){
			//
			// 	std::cout << "item = " << item << " " << "sumRatings = " << sumRatings
			// 	<< " " <<  "sumWeights = " << sumWeights << " "
			// 	<< (sumRatings * 1000) / (sumWeights * 1000) << '\n';
			// }
			ResultRatings[metric][user][item] = MeanRatings[user] + (sumRatings / sumWeights);
		}
	}
	//
	// if( user == 0 ){
	// 	exit(0);
	// }

	delete[] neighborIndexes;
	delete[] neighborSim;
}

void RecommendManager::run(std::string& dataPath, std::string& outPath){
	fillTable(dataPath);												// create the LSHT (as needed for every Point type)
	getPath( outPath,  "Please, enter the path for the output file" );	// promt the user for output file
	ofstream outFile;
	openFileWrite(outPath, outFile);									// open the above file

	for(int metric = 0; metric < 3; metric++){
		runTests(metric, outFile);
		evaluate(metric, outFile, Messages[metric]);
	}

	if( Validate ){
		validate(outFile);
	}

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
	int* itemIndexes = new int[NumItems];

	outFile << Message << endl;
	for(int i = 0; i < NumUsers; i++){
		for( int j = 0; j < NumItems; j++ ){
			itemIndexes[j] = j;
		}

		Math::sort(ResultRatings[metric][i], itemIndexes, NumItems);
		outFile << i << " ";
		for(int j = 0, count = 0; j < NumItems && count < 5; j++ ){
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
	int F = 10;

	int validSize = NumUsers * (1.0) / F;
	List<int> positions;
	for(int i = 0; i < NumUsers; i++){
		positions.insertRandom( new int(i), true);
	}

	Partition* Partitions[F];
	for(int f = 0; f < F - 1; f++){
		Partitions[f] = new Partition(validSize, positions);
	}
	Partitions[F-1] = new Partition(positions.count(), positions);

	bool** RealRatingsCopy = new bool*[NumUsers];
	for(int i = 0; i < NumUsers; i++){
		RealRatingsCopy[i] = new bool[NumItems];
		for(int j = 0; j < NumItems; j++){
			RealRatingsCopy[i][j] = RealRatings[i][j];
		}
	}

	for(int metric = 0; metric < 3; metric++){
		MAE[metric] = 0.0;
	}

	for(int f = 0; f < F; f++){ // for every validation fold

		double** ResultRatingsCopy[3];
		// store initial results
		for(int metric = 0; metric < 3; metric++){
			ResultRatingsCopy[metric] = new double*[Partitions[f]->size()];
			for(int user = 0; user < Partitions[f]->size(); user++){
				ResultRatingsCopy[metric][user] = new double[NumItems];
				for(int item = 0; item < NumItems; item++){
					ResultRatingsCopy[metric][user][item] = ResultRatings[metric][user][item];
				}
			}
		}

		Partition DataSet(Partitions, F, f);
		// skip them in predictions
		for(int user = 0; user < DataSet.size(); user++){
			for(int item = 0; item < NumItems; item++){
				RealRatings[user][item] = true;
			}
		}

		// predict only the already rated
		for(int user = 0; user < Partitions[f]->size(); user++){
			for(int item = 0; item < NumItems; item++){
				RealRatings[user][item] = !RealRatings[user][item];
			}
		}

		// runTests
		// reset results
		// compute MAE
		// delete ResultRatingsCopy
		double fMAE[3] = {0.0, 0.0, 0.0};
		for(int metric = 0; metric < 3; metric++){
			runTests(metric, outFile);
			for(int user = 0; user < Partitions[f]->size(); user++){
				for(int item = 0; item < NumItems; item++){
					if( RealRatingsCopy[user][item] ){
						fMAE[metric] += abs( ResultRatingsCopy[metric][user][item] - ResultRatings[metric][user][item] );
					}
					ResultRatings[metric][user][item] = ResultRatingsCopy[metric][user][item];
				}
				delete[] ResultRatingsCopy[metric][user];
			}
			delete[] ResultRatingsCopy[metric];
			fMAE[metric] /= Partitions[f]->size();
			MAE[metric] += fMAE[metric];
		}

		// reset RealRatings
		for(int i = 0; i < NumUsers; i++){
			for(int j = 0; j < NumItems; j++){
				RealRatings[i][j] = RealRatingsCopy[i][j];
			}
		}

	}

	for(int metric = 0; metric < 3; metric++){
		MAE[metric] /= F;
		outFile << Messages[metric] << " MAE: " << MAE[metric] << endl;
	}


	for(int i = 0; i < NumUsers; i++){
		delete[] RealRatingsCopy[i];
	}
	delete[] RealRatingsCopy;


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

		// std::cout << "NumUsers = " << NumUsers << std::endl;
		// std::cout << "NumItems = " << NumItems << std::endl;

		for(int i = 0; i < NumUsers; i++){

			// std::cout << "i = " << i << std::endl<< std::endl<< std::endl<< std::endl;
			//
			// for(int j = 0; j < NumItems; j++){
			// 	std::cout << "rating = " << ResultRatings[i][j]
			// 	 			<< " - real = " << RealRatings[i][j] << std::endl;
			// }
			//
			// std::cout << "go to the next!" << std::endl;

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
		return NumItems / 2;
	}
	else{
		// mean diff is 3 (sqr -> 9)
		// mean "true" NumItems is 10, so approximately 20 when calculating distance
		return 180.0;
	}
}

void NNRecommendManager::runTests(int metric, std::ofstream& outFile){
	for(int i = 0; i < NumUsers; i++){
		std::cout << "Examine Neighbors of user " << i << std::endl;
		List<Point, Point*>* Neighbors;
		Neighbors = findNeighbours(metric, PointTable[metric][i]);
		estimateRating(metric, i, Neighbors);
		delete Neighbors;
	}
}

double abs(double x){
	if( x < 0.0 ){
		return -x;
	}
	return x;
}

List<Point, Point*>* NNRecommendManager::findNeighbours(int metric, Point* point){

	// TODO
	if( metric == HAM_INDEX ){
		return new List<Point, Point*>();
	}

	int times = 0;
	int BarrierTimes = 3;
	int Tolerance = P / 4;

	List<Point, Point*> *ResultPointsSmall, *ResultPointsBig;
	double RSmall, RBig;

	ResultPointsSmall = new List<Point, Point*>();
	ResultPointsBig = new List<Point, Point*>();
	RSmall = getRadius(metric);
	RBig = getRadius(metric);


	clock_t begin = clock();
	LSH[metric]->inRange( point, RSmall, *ResultPointsSmall );
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;


	if( metric == HAM_INDEX ){
		std::cout << "time of inRange() = " << time_spent << " secs." << std::endl;
		std::cout << "RSmall = " << RSmall << std::endl;
		std::cout << "count = " << ResultPointsSmall->count() << std::endl;
		exit(0);
	}

	// std::cout << "initially, RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;

	// if( ResultPointsSmall->count() == P ){
	if( abs(ResultPointsSmall->count() - P) < Tolerance ){
		delete ResultPointsBig;
		return ResultPointsSmall;
	}
	else if( ResultPointsSmall->count() < P ){

		// std::cout << "try to find RBig" << std::endl;

		do{
			ResultPointsBig->flush();
			RBig = PointTable[metric][0]->multiplyDouble(RBig, 2.0);
			LSH[metric]->inRange( point, RBig, *ResultPointsBig );
			times++;
			// std::cout << "RBig = " << RBig->getDouble() << " -> " << ResultPointsBig->count() << std::endl;
		} while( ResultPointsBig->count() < P && times < BarrierTimes );

		// if( ResultPointsBig->count() == P || times == BarrierTimes ){
		if( abs(ResultPointsBig->count() - P) < Tolerance || times == BarrierTimes ){
			delete ResultPointsSmall;
			return ResultPointsBig;
		}

		RSmall = PointTable[metric][0]->multiplyDouble(RBig, 0.5);
	}
	else{

		// std::cout << "try to find RSmall" << std::endl;


		do{
			ResultPointsSmall->flush();
			RSmall = PointTable[metric][0]->multiplyDouble(RSmall, 0.5);

			LSH[metric]->inRange( point, RSmall, *ResultPointsSmall );
			times++;
			// std::cout << "RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;
		} while( ResultPointsSmall->count() > P && times < BarrierTimes );

		// if( ResultPointsSmall->count() == P || times == BarrierTimes ){
		if(  abs(ResultPointsSmall->count() - P) < Tolerance || times == BarrierTimes ){
			delete ResultPointsBig;
			return ResultPointsSmall;
		}

		RBig = PointTable[metric][0]->multiplyDouble(RSmall, 2.0);

	}

	// std::cout  << std::endl << std::endl;

	do {
		double RMean = (RSmall + RBig) / 2.0;
		List<Point, Point*> *ResultPointsMean = new List<Point, Point*>();
		LSH[metric]->inRange( point, RMean, *ResultPointsMean );

		// std::cout << std::endl<< std::endl;

		// std::cout << "NOW, RSmall = " << RSmall->getDouble() << " -> " << ResultPointsSmall->count() << std::endl;
		// std::cout << "NOW, RBig = " << RBig->getDouble() << " -> " << ResultPointsBig->count() << std::endl;
		// std::cout << "NOW, RMean = " << RMean->getDouble() << " -> " << ResultPointsMean->count() << std::endl;


		// if( ResultPointsMean->count() == P || times == BarrierTimes ){
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
}

void ClusterRecommendManager::findAlgorithm(int metric){
	Algorithm[metric] = NULL;
	d[metric] = new TriangularMatrix(NumUsers, PointTable[metric]);

	double bestScore = DBL_MAX;
	ofstream outFile("");
	int UpperBound = log2(NumUsers);
	for( int clusters = 2; clusters < UpperBound; clusters++){
		ClusterAlgorithm* tempClustering = new ClusterAlgorithm(PointTable[metric], d[metric], NumUsers, clusters, 4, -1, -1, -1, -1);

		tempClustering->run();

		double tempScore = tempClustering->evaluate(outFile, false, false);
		if( tempScore < bestScore ){
			bestScore = tempScore;
			K_clusters[metric] = clusters;
			if( Algorithm[metric] != NULL ){
				delete Algorithm[metric];
			}
			Algorithm[metric] = tempClustering;
		}
		else{
			delete tempClustering;
		}
	}
}

void ClusterRecommendManager::runTests(int metric, std::ofstream& outFile){
	findAlgorithm(metric);
	for(int k = 0; k < K_clusters[metric]; k++){
		List<AssignPair>* cluster = Algorithm[metric]->getCluster(k);
		List<Point, Point*>* Neighbors = findNeighbours(metric,cluster);
		for (Node<AssignPair>* node = cluster->start() ; node != NULL; node = node->next() ) {
			estimateRating(metric, node->data()->assigned(), Neighbors);
		}
		delete Neighbors;
	}
	delete Algorithm[metric];
}


List<Point, Point*>* ClusterRecommendManager::findNeighbours(int metric, List<AssignPair>* cluster){
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
}
