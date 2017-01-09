#include "ProteinsManager.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/MetricSpace.h"

#include <sstream>
#include <float.h>

using namespace std;

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
	double bestScore = DBL_MAX;
	double bestTime = 0.0;
	Algorithm = NULL;
	int times = 0;
	int UpperBound = log2(numConform) * 5;
	for( int clusters = 2; clusters < UpperBound; clusters++){
		ProteinsCluster* tempClustering = new ProteinsCluster(PointTable, d, numConform, clusters, 4, -1, -1, -1, -1);

		clock_t start = clock();		// start measuring time
		tempClustering->run();
		clock_t end = clock();			// stop measuring
		bestTime = (end - start)/(double)CLOCKS_PER_SEC;

		double tempScore = tempClustering->evaluate(outfFile, Complete, false);
		if( tempScore < bestScore ){
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

		times++;
		if( times % 10 == 0){
			std::cout << "Done " << times << "!" << '\n';
		}
	}
	evaluate(outfFile);
}

void ProteinsManager::fillTable(std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	PointTable = new Point*[numConform];
	for(int i = 0; i < numConform; i++){					// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
	}

	d = new TriangularMatrix(numConform, PointTable);
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
: ProteinsManager(complete) {}

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

void cRMSDManager::evaluate(std::ofstream& outfFile){
	Algorithm->evaluate(outfFile, Complete, true);
}

cRMSDManager::~cRMSDManager(){

}


/*****************************************************************
***************** dRMSDManager class methods *********************
******************************************************************/

int dRMSDManager::R = 0;
bool dRMSDManager::firstTime = true;
PairDummy* dRMSDManager::Indexes = NULL;

double* dRMSDManager::Configuration = NULL;

dRMSDManager::dRMSDManager(dOption t, rGenerator func, bool complete)
: ProteinsManager(complete) {
	Func = func;
	T = t;
}

Point* dRMSDManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	static int count = -1;

	stringstream name;
	count++;
	name << count + 1;
	stringstream rCoordinates;

	if( Configuration == NULL ){
		Configuration = new double[numConform*N*3];
	}

	if( count < numConform ){
		for(int i = 0; i < 3*N; i++){
			queryFile >> Configuration[N*3*count + i];
		}
	}


	if( firstTime ){
		firstTime = false;
		R = Func(N);
		Indexes = new PairDummy[R];


		if( T == SMALLEST || T == LARGEST ){
			int DistLength = N * (N - 1) / 2;
			PairDummy* tempIndexes = new PairDummy[DistLength];
			double* Distances = new double[DistLength];

			for(int i = 0, d = 0; i < 3*N; i += 3){
				for(int j = 0; j < i; j += 3, d++){
					tempIndexes[d].i = i;
					tempIndexes[d].j = j;

					double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
					dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
					dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
					Distances[d] = dist;
				}
			}

			Math::sort(Distances, tempIndexes, DistLength);

			if( T == SMALLEST ){
				for(int r = 0; r < R; r++){
					Indexes[r] = tempIndexes[r];
					rCoordinates << Distances[r] << " ";
				}
			}
			else{
				for(int r = DistLength - R; r < DistLength; r++){
					Indexes[r - (DistLength - R) ] = tempIndexes[r];
					rCoordinates << Distances[r] << " ";
				}
			}

			delete[] tempIndexes;
			delete[] Distances;
		}
		else{
			for( int r = 0; r < R; r++ ){
				int i = ( (int)Math::dRand(0,N) ) * 3;
				int j = ( (int)Math::dRand(0,N) ) * 3;
				Indexes[r].i = i;
				Indexes[r].j = j;

				double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
				dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
				dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
				rCoordinates << dist << " ";
			}
		}
	}
	else{
		for( int r = 0; r < R; r++ ){
			int i = N*3*(count % numConform) + Indexes[r].i;
			int j = N*3*(count % numConform) + Indexes[r].j;

			double dist = (Configuration[i] - Configuration[j]) * (Configuration[i] - Configuration[j]);
			dist += (Configuration[i+1] - Configuration[j+1]) * (Configuration[i+1] - Configuration[j+1]);
			dist += (Configuration[i+2] - Configuration[j+2]) * (Configuration[i+2] - Configuration[j+2]);
			rCoordinates << dist << " ";
		}
	}

	string buffer = rCoordinates.str();
	return new EuclideanPoint(name.str(), buffer, R);
}

void dRMSDManager::evaluate(std::ofstream& outfFile){
	double Silhouette = Algorithm->evaluate(outfFile, Complete, false);
	outfFile 	<< "r: " << Func(N) << " , T = " << T << " , k = " << K_clusters
				<< ", silhouette = " << Silhouette << " execution time = " << BestTime << " secs." << endl;
}

dRMSDManager::~dRMSDManager(){
	if( Indexes != NULL ){
		delete[] Indexes;
		Indexes = NULL;
	}

	if( Configuration != NULL ){
		delete[] Configuration;
		Configuration = NULL;
	}
}
