#include "ProteinsManager.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/MetricSpace.h"

#include <sstream>
#include <float.h>

using namespace std;

string optionString(pickOption opt){
	switch ( opt ) {
		case SMALLEST: return "Smallest";
		case LARGEST: return "Largest";
		default: return "Random";
	}
}

string rString(rOption opt){
	switch ( opt ) {
		case SMALL: return "N";
		case MEDIUM: return "N^3/2";
		default: return "N*(N-1)/2";
	}
}


int small(int N){
	return N;
}

int medium(int N){
	return N * sqrt(N);
}

int large(int N){
	return N * (N - 1) / 2;
}


/*****************************************************************
***************** ProteinsManager class methods *************************
******************************************************************/

void ProteinsManager::getPath(string& path, string message){
	if( path != "" ){											// if the path is already set
		return;													// just return
	}

	cout << message << endl;									// else, promt the user to give a path
	cin >> path;												// and actually store the path
}

void ProteinsManager::openFileWrite(string& path, ofstream& file){
	file.open( path.c_str(), std::ios_base::app );								//open a file to write, create it if it doesn't exist
}

void ProteinsManager::openFileRead(string& path, ifstream& file){
	file.open( path.c_str() );									// open a file to read
	while( !file.is_open() ){									// if somethind went wrong
		path = "";												// clear the path
		getPath(path, "Cannot open file : " + path + ". Please, enter a valid path." );	// promt the user to re-enter a path
		file.open( path.c_str() );								// and attempt to open the new file
	}

	string temp;
	file >> temp;			// skip "numConform:"
	file >> numConform;
	file >> temp;			// skip "N:"
	file >> N;
}

ProteinsManager::ProteinsManager(bool complete) {
	numConform = 0;
	N = 0;
	PointTable = NULL;
	BestTime = 0.0;
	K_clusters = 0;
	Complete = complete;
}

void ProteinsManager::run(std::string& dataPath, std::string& outPath){
	printMessage();
	fillTable(dataPath);												// create the LSHT (as needed for every Point type)
	getPath( outPath,  "Please, enter the path for the output file" );	// promt the user for output file
	ofstream outfFile;
	openFileWrite(outPath, outfFile);									// open the above file
	runTests(outfFile);
	finalise();
}

void ProteinsManager::runCUTests(void){
	ClusterAlgorithm::runCUTests();
}

void ProteinsManager::runTests(ofstream& outfFile){
	double bestTime = 0.0;			// running time of best clustering
	double bestScore = -2.0;		// initialise best silhouette with -2
	Algorithm = NULL;				// initially, no algorithm was picked
	int MaxK = log2(numConform) * 5;

	std::cout << "Ready to select optimal K in [2, " << MaxK - 1 << "] through Silhouette..." << '\n';
	for( int clusters = 2; clusters < MaxK; clusters++){	// for every possible cluster

		if( (clusters - 1) % 20 == 0 ){
			std::cout << "Done " << clusters - 1 << " iterations..." << '\n';
		}

		// pick an algorithm
		ProteinsCluster* tempClustering = new ProteinsCluster(PointTable, d, numConform, clusters, 4, -1, -1, -1, -1, RandCluster);

		// run the algorithm
		clock_t start = clock();
		tempClustering->run();
		clock_t end = clock();
		bestTime = (end - start)/(double)CLOCKS_PER_SEC;

		double tempScore = tempClustering->evaluate(outfFile, Complete, false);

		if( tempScore > bestScore ){
			bestScore = tempScore;
			K_clusters = clusters;
			BestTime = bestTime;
			if( Algorithm != NULL ){
				delete Algorithm;
			}
			Algorithm = tempClustering;
		}
		else{
			delete tempClustering;
		}
	}

	std::cout << "Done clustering, start evaluating..." << '\n';
	evaluate(outfFile);

	// std::cout << "clusters = " << K_clusters << '\n';
	// for(int i = 0; i < K_clusters; i++){
	// 	std::cout << "i = " << i << ", size = " << Algorithm->getCluster(i)->count() << '\n';
	// }
	// std::cout << '\n' << '\n' << '\n';

}

void ProteinsManager::fillTable(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	std::cout << "Parsing protein data..." << '\n';

	PointTable = new Point*[numConform];
	for(int i = 0; i < numConform; i++){					// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
		if( (i + 1) % 1000 == 0 ){
			std::cout << "Parsed " << i << " points so far." << '\n';
		}
	}

	d = new TriangularMatrixLazy(numConform, PointTable);
	// d = new TriangularMatrix(numConform, PointTable);
}

void ProteinsManager::finalise(void){
	if( PointTable != NULL ){
		for(int i = 0; i < numConform; i++){
			delete PointTable[i];
		}
		delete[] PointTable;

		if( Algorithm != NULL ){
			delete Algorithm;
		}

		delete d;
		PointTable = NULL;
	}
}

ProteinsManager::~ProteinsManager(){
	finalise();
}

/*****************************************************************
***************** cRMSDManager class methods *********************
******************************************************************/

cRMSDManager::cRMSDManager(bool complete)
: ProteinsManager(complete) {
	RandCluster = true;			// TODO: faster!
}

Point* cRMSDManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	static int count = -1;
	double* Configuration = new double[N*3];
	for(int i = 0; i < 3*N; i++){
		queryFile >> Configuration[i];
	}

	stringstream name;
	count++;
	name << count + 1;
	return new MetricSpacePoint( name.str(), numConform, N, count, Configuration);
}

void cRMSDManager::printMessage(void){
	std::cout << endl << endl << "Ready to evaluate with c-RMSD" << '\n';
}

void cRMSDManager::evaluate(std::ofstream& outfFile){
	Algorithm->evaluate(outfFile, Complete, true);
}

cRMSDManager::~cRMSDManager(){}


/*****************************************************************
***************** dRMSDManager class methods *********************
******************************************************************/

double* dRMSDManager::Configuration = NULL;

dRMSDManager::dRMSDManager(pickOption t, rOption r, bool complete)
: ProteinsManager(complete) {
	R = 0;
	Indexes = NULL;
	rOpt = r;
	T = t;
	switch ( rOpt ) {
		case SMALL: Func = &small; break;
		case MEDIUM: Func = &medium; break;
		default: 	Func = &large; break;
	}

	RandCluster = false;
}

double dRMSDManager::distance(int i, int j){
	double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
	dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
	dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
	return dist;
}

Point* dRMSDManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	static int count = -1;											// the id of the next protein is stored here

	stringstream name;
	count++;
	name << count + 1;
	stringstream rCoordinates;

	if( Configuration == NULL ){									// if the array is uninitialised
		Configuration = new double[numConform*N*3];					// allocate it
	}

	if( count < numConform ){										// read every point only once!
		for(int i = 0; i < 3*N; i++){
			queryFile >> Configuration[N*3*count + i];
		}
	}


	if( Indexes == NULL ){											// for the first point of every manager
		R = Func(N);												// calculate R
		Indexes = new PairDummy[R];									// allocate array for indexes

		if( T == SMALLEST || T == LARGEST ){						// when LARGEST or SMALLEST
			// compute all distances, sort and pick
			int DistLength = N * (N - 1) / 2;
			PairDummy* tempIndexes = new PairDummy[DistLength];
			double* Distances = new double[DistLength];

			for(int i = 0, d = 0; i < 3*N; i += 3){
				for(int j = 0; j < i; j += 3, d++){
					tempIndexes[d].i = i;
					tempIndexes[d].j = j;
					Distances[d] = distance(i, j);
				}
			}

			// sort ascending
			Math::sort(Distances, tempIndexes, DistLength);

			if( T == SMALLEST ){							// SMALLEST, pick the first "R" values
				for(int r = 0; r < R; r++){
					Indexes[r] = tempIndexes[r];
					rCoordinates << Distances[r] << " ";
				}
			}
			else{
				for(int r = DistLength - R; r < DistLength; r++){	// LARGEST, pick the last "R" values
					Indexes[r - (DistLength - R) ] = tempIndexes[r];
					rCoordinates << Distances[r] << " ";
				}
			}

			delete[] tempIndexes;
			delete[] Distances;
		}
		else{
			// else, RANDOM is selected
			for( int r = 0; r < R; r++ ){
				int i = ( (int)Math::dRand(0,N) ) * 3;
				int j = 0;
				do{
					j = ( (int)Math::dRand(0,N) ) * 3;
				}while( i == j);

				Indexes[r].i = i;
				Indexes[r].j = j;
				rCoordinates << distance(i, j) << " ";
			}
		}
	}
	else{
		// for the rest of the points, pick whatever te first point picked
		for( int r = 0; r < R; r++ ){
			int i = N*3*(count % numConform) + Indexes[r].i;
			int j = N*3*(count % numConform) + Indexes[r].j;
			rCoordinates << distance(i, j) << " ";
		}
	}

	string buffer = rCoordinates.str();
	return new EuclideanPoint(name.str(), buffer, R);
}

void dRMSDManager::printMessage(void){
	std::cout << endl << endl << "Ready to evaluate with d-RMSD ( r = " << rString(rOpt) << ", T = " << optionString(T) << " )" << '\n';
}

void dRMSDManager::evaluate(std::ofstream& outfFile){
	double Silhouette = Algorithm->evaluate(outfFile, Complete, false);
	outfFile 	<< "r: " << R << " , T = " << optionString(T) << " , k = " << K_clusters
				<< ", silhouette = " << Silhouette << " execution time = " << BestTime << " secs." << endl;
}

dRMSDManager::~dRMSDManager(){
	delete[] Indexes;

	if( Configuration != NULL ){
		delete[] Configuration;
		Configuration = NULL;
	}
}
