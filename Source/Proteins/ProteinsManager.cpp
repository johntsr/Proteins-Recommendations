#include "ProteinsManager.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/MetricSpace.h"

#include <sstream>
#include <bitset>

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
	file.open( path.c_str() );									//open a file to write, create it if it doesn't exist
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

ProteinsManager::ProteinsManager(int k_clusters, int k_hash, int l, int q, int s, bool complete) {
	Dimension = 0;
	numConform = 0;
	N = 0;
	PointTable = NULL;

	K_clusters = k_clusters;
	K_hash = k_hash;
	L = l;
	Q = q;
	S = s;
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
	Algorithm->run();
	Algorithm->evaluate(outfFile, Complete);
}

void ProteinsManager::fillTable(std::string dataPath){


	ifstream file;
	openFileRead(dataPath, file);

	PointTable = new Point*[numConform];
	for(int i = 0; i < numConform; i++){					// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
	}

	d = new TriangularMatrix(numConform, PointTable);

	Algorithm = new ClusterAlgorithm(PointTable, d, numConform, K_clusters, 4, K_hash, L, Q, S);
}

void ProteinsManager::finalise(void){
	if( PointTable != NULL ){
		for(int i = 0; i < numConform; i++){
			delete PointTable[i];
		}
		delete[] PointTable;

		delete Algorithm;

		delete d;
		PointTable = NULL;
	}
}

ProteinsManager::~ProteinsManager(){
	finalise();
}


/*****************************************************************
***************** EuclideanManager class methods ******************
******************************************************************/

// EuclideanManager::EuclideanManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete, bool flag)
// : ProteinsManager(K_clusters, K_hash, L, Q, S, complete) {
// 	Explicit = flag;
// }
//
// void EuclideanManager::openFileRead(string& path, ifstream& file){
// 	ProteinsManager::openFileRead(path, file);
// 	if( Explicit ){					// "Euclidean" metric was given explicitely, so skip this line
// 		string buffer;
// 		getline( file, buffer);		// skip second line
// 	}
// }
//
// Point* EuclideanManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
// 	string buffer, name;
// 	queryFile >> name;
// 	getline(queryFile, buffer);
// 	return new EuclideanPoint(name, buffer , Dimension );
// }


/*****************************************************************
***************** MetricSpaceManager class methods ******************
******************************************************************/

MetricSpaceManager::MetricSpaceManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete)
: ProteinsManager(K_clusters, K_hash, L, Q, S, complete) {}

Point* MetricSpaceManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	static int count = -1;
	double* Configuration = new double[N*3];
	for(int i = 0; i < 3*N; i++){
		queryFile >> Configuration[i];
	}

	stringstream name;
	count++;
	name << "Protein " << count;
	return new MetricSpacePoint( name.str(), numConform, N, count, Configuration);
}

MetricSpaceManager::~MetricSpaceManager(){
}
