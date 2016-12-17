#include "ClusterManager.h"
#include "../Metrics/Hamming.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Metrics/MetricSpace.h"

#include <sstream>
#include <bitset>

using namespace std;

/*****************************************************************
***************** ClusterManager class methods *************************
******************************************************************/

int ClusterManager::countPoints( string fileName){
	ifstream inFile(fileName.c_str());							// open the file for reading

	string temp;
	// @metric_space Hamming
	inFile >> temp;			// skip "@metric_space"
	inFile >> temp;			// get metric function


	int numLines = 0;											// initially, zero lines read
	if( temp == "vector" || temp == "matrix" ){
		inFile >> temp;
		if( temp != "@metric" && temp != "@items" ){
			numLines++;
		}
	}
	getline(inFile, temp);


	string stringLine;											// variable that holds a line each time
	while ( getline(inFile, stringLine) ){						// while there are more lines
	   numLines++;												// increment their number
   	}
	return numLines;											// return the number of lines
}

int  ClusterManager::countDimension	(string fileName){
	ifstream inFile(fileName.c_str());							// open the file for reading
	int numLines = 0;											// initially, zero lines read
	int dimension = 0;											// initially, zero dimensions read
	string stringLine;											// variable that holds a line each time
	while ( getline(inFile, stringLine) ){						// while there are more lines
	   numLines++;												// increment their number
		if( numLines == 3){										// just for the first point
			istringstream streamLine(stringLine);				// convert the line to an easily parsed form
			string temp;										// temporraty variable to hold the co-ordinates
			streamLine >> temp;									// skip first word, i.e. the name of the point
			while( streamLine >> temp ){						// while more co-ordinates are found
				dimension++;									// increase the dimensionality
			}
			return dimension;									// and return immediately
		}
   	}
	return 0;													// if something unexpeccted occured, retrn 0 dimensionality
}


void ClusterManager::getPath(string& path, string message){
	if( path != "" ){											// if the path is already set
		return;													// just return
	}

	cout << message << endl;									// else, promt the user to give a path
	cin >> path;												// and actually store the path
}

void ClusterManager::openFileWrite(string& path, ofstream& file){
	file.open( path.c_str() );									//open a file to write, create it if it doesn't exist
}

void ClusterManager::openFileRead(string& path, ifstream& file){
	file.open( path.c_str() );									// open a file to read
	while( !file.is_open() ){									// if somethind went wrong
		path = "";												// clear the path
		getPath(path, "Cannot open file : " + path + ". Please, enter a valid path." );	// promt the user to re-enter a path
		file.open( path.c_str() );								// and attempt to open the new file
	}
	string buffer;
	getline( file, buffer);								// skip first line
}

ClusterManager::ClusterManager(int k_clusters, int k_hash, int l, int q, int s, bool complete) {
	Dimension = 0;
	N = 0;
	PointTable = NULL;

	K_clusters = k_clusters;
	K_hash = k_hash;
	L = l;
	Q = q;
	S = s;
	Complete = complete;
}

void ClusterManager::run(std::string& dataPath, std::string& outPath){
	fillTable(dataPath);												// create the LSHT (as needed for every Point type)
	getPath( outPath,  "Please, enter the path for the output file" );	// promt the user for output file
	ofstream outfFile;
	openFileWrite(outPath, outfFile);									// open the above file
	runTests(outfFile);
	finalise();
}

void ClusterManager::runCUTests(void){
	ClusterAlgorithm::runCUTests();
}

void ClusterManager::runTests(ofstream& outfFile){
	for(int i = 0; i < 9; i++){						// run all variation of the algorithms
		Algorithm[i]->run();
		Algorithm[i]->evaluate(outfFile, Complete);
	}
}

void ClusterManager::fillTable(std::string dataPath){
	Dimension = countDimension(dataPath);
	N = countPoints( dataPath );
	PointTable = new Point*[N];

	ifstream file;
	openFileRead(dataPath, file);

	for(int i = 0; i < N; i++){					// read the whole file
		PointTable[i] = getNextPoint(file);		// store all the points
	}

	// exit(0);
	d = new TriangularMatrix(N, PointTable);

	for(int i = 0; i < 8; i++){
		Algorithm[i] = new ClusterAlgorithm(PointTable, d, N, K_clusters, i, K_hash, L, Q, S);
	}

	int n_ = 40 + 2*K_clusters;
	Algorithm[8] = new CLARA(PointTable, N, S, n_, K_clusters);

}

void ClusterManager::finalise(void){
	if( PointTable != NULL ){
		for(int i = 0; i < N; i++){
			delete PointTable[i];
		}
		delete[] PointTable;

		for(int i = 0; i < 9; i++){
			delete Algorithm[i];
		}

		delete d;
		PointTable = NULL;
	}
}

ClusterManager::~ClusterManager(){
	finalise();
}

/*****************************************************************
***************** HammingManager class methods ********************
******************************************************************/


HammingManager::HammingManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete)
	: ClusterManager(K_clusters, K_hash, L, Q, S, complete) {}

Point* HammingManager::getNextPoint(ifstream& queryFile){	// depends on the format of the file
	string buffer, name;
	queryFile >> name >> buffer;
	// (new HammingPoint(name, buffer ))->print();
	return new HammingPoint(name, buffer );
}

int  HammingManager::countDimension	(string fileName){
	ifstream inFile(fileName.c_str());							// open the file for reading
	int numLines = 0;											// initially, zero lines read
	string stringLine;											// variable that holds a line each time
	while ( getline(inFile, stringLine) ){						// while there are more lines
	   numLines++;												// increment their number
		if( numLines == 2){										// just for the first point
			istringstream streamLine(stringLine);				// convert the line to an easily parsed form
			string temp;										// temporraty variable to hold the co-ordinates
			streamLine >> temp;									// skip first word, i.e. the name of the point
			streamLine >> temp;									// get bit string
			return temp.size();									// and return immediately
		}
   	}
	return 0;													// if something unexpeccted occured, retrn 0 dimensionality
}


/*****************************************************************
***************** CosineManager class methods *********************
******************************************************************/

CosineManager::CosineManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete)
: ClusterManager(K_clusters, K_hash, L, Q, S, complete) {}

Point* CosineManager::getNextPoint(ifstream& queryFile){		// depends on the format of the file
	string buffer, name;
	queryFile >> name;
	getline(queryFile, buffer);
	return new CosinePoint(name, buffer , Dimension );
}

void CosineManager::openFileRead(std::string& path, std::ifstream& file){
	ClusterManager::openFileRead(path, file);
	string buffer;
	getline( file, buffer);		// skip second line
}

/*****************************************************************
***************** EuclideanManager class methods ******************
******************************************************************/

EuclideanManager::EuclideanManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete, bool flag)
: ClusterManager(K_clusters, K_hash, L, Q, S, complete) {
	Explicit = flag;
}

void EuclideanManager::openFileRead(string& path, ifstream& file){
	ClusterManager::openFileRead(path, file);
	if( Explicit ){					// "Euclidean" metric was given explicitely, so skip this line
		string buffer;
		getline( file, buffer);		// skip second line
	}
}

Point* EuclideanManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	string buffer, name;
	queryFile >> name;
	getline(queryFile, buffer);
	return new EuclideanPoint(name, buffer , Dimension );
}


/*****************************************************************
***************** MetricSpaceManager class methods ******************
******************************************************************/

MetricSpaceManager::MetricSpaceManager(int K_clusters, int K_hash, int L, int Q, int S, bool complete)
: ClusterManager(K_clusters, K_hash, L, Q, S, complete) {}

void MetricSpaceManager::openFileRead(std::string& path, std::ifstream& file){

	Dimension = countDimension(path) + 1;
	currentPointIndex = -1;
	Names = new string[Dimension];

	ClusterManager::openFileRead(path, file);
	string temp;
	getline(file, temp, ' ');					// skip first word
	getline(file, temp);
	std::istringstream parser( temp );
	for(int i = 0; i < Dimension; i++){
		getline(parser, temp, ',');
		Names[i] = temp;
	}
}

Point* MetricSpaceManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	string buffer;
	getline(queryFile, buffer);
	currentPointIndex++;
	return new MetricSpacePoint( Names[currentPointIndex], Dimension, buffer, currentPointIndex);
}

MetricSpaceManager::~MetricSpaceManager(){
	delete[] Names;
}
