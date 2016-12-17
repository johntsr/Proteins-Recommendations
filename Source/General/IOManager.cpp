#include "IOManager.h"
#include "Math.h"
#include "../Metrics/Hamming.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Metrics/MetricSpace.h"
#include <sstream>

#include <bitset>

using namespace std;

/*****************************************************************
***************** IOManager class methods *************************
******************************************************************/

int IOManager::countLines( string fileName, int& dimension ){
	ifstream inFile(fileName.c_str());							// open the file for reading
	int numLines = 0;											// initially, zero lines read
	dimension = 0;
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
		}
   	}
	return numLines;											// return the number of lines
}

int  IOManager::countDimension	(string fileName){
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


void IOManager::getPath(string& path, string message){
	if( path != "" ){											// if the path is already set
		return;													// just return
	}

	cout << message << endl;									// else, promt the user to give a path
	cin >> path;												// and actually store the path
}

bool IOManager::wantsMore(void){
	cout << "Done with test, for more test please enter MORE, else enter EXIT " << endl;
	string answer;
	cin >> answer;
	while( answer != "MORE" && answer != "EXIT" ){				// only "MORE" and "EXIT" are valid answers
		cout << "Please, enter MORE or EXIT" << endl;
		cin >> answer;
	}

	return answer == "MORE";									// return true if the user wants to perform more queries
}

void IOManager::openFileWrite(string& path, ofstream& file){
	file.open( path.c_str() );									//open a file to write, create it if it doesn't exist
}

void IOManager::openFileRead(string& path, ifstream& file){
	file.open( path.c_str() );									// open a file to read
	while( !file.is_open() ){									// if somethind went wrong
		path = "";												// clear the path
		getPath(path, "Cannot open file : " + path + ". Please, enter a valid path." );	// promt the user to re-enter a path
		file.open( path.c_str() );								// and attempt to open the new file
	}
}

IOManager::IOManager(int l, int k) {
	L = l;														// the number of Hash Tables the LSHT uses
	K = k;														// the number of "h" hash functions we use
}

void IOManager::run(int barrier, std::string& dataPath, std::string& outPath, std::string& queryPath){
	std::cout << "Ready to create LSH..." << std::endl;
	createLSH(barrier, dataPath);											// create the LSHT (as needed for every Point type)
	std::cout << "Done with LSH, start queries..." << std::endl;
	getPath( outPath,  "Please, enter the path for the output file" );		// promt the user for output file
	ofstream outfFile;
	openFileWrite(outPath, outfFile);										// open the above file

	do {
		getPath( queryPath,  "Please, enter the path for the query file" );	// promt the user for query file
		runTests(queryPath, outfFile);										// run test on this query set
		queryPath = "";														// clear the query path
	} while( wantsMore() );													// repeat if the user wants to


}

void IOManager::inRange(Point* newPoint, Quantity* R, ofstream& outFile){
	// "newPoint"  		: the Point whose neighbours I am searching
	// "R"				: the range I am interested in
	List<Point, Point*> ResultPoints;			// list of Points I discover in the above range

	LSH->inRange( newPoint, R, ResultPoints );	// query the LSHT for the neighbours

	outFile << "R-near neighbors:" << endl;
	for (Node<Point>* node = ResultPoints.start() ; node != NULL; node = node->next() ) {
		outFile << node->data()->name() << endl;
	}

}

double IOManager::nearestNeighbour(Point* newPoint, ofstream& outFile){
	// "newPoint"  		: the Point whose neighbours I am searching

	Point* bestPoint = NULL;	// the point closest to "newPoint"
	Quantity* minDist;			// the distanve between "newPoint" and the above point

	clock_t start = clock();
	LSH->nearestNeighbour( newPoint, minDist, bestPoint );	// query the LSHT for the approximate nearest neighbour
	clock_t end = clock();

	if( bestPoint != NULL){
		outFile << "Nearest neighbor: " << bestPoint->name() << endl;
		outFile << "distanceLSH: " << minDist->getString() << endl;
	}

	delete minDist;
	return (end - start)/(double)CLOCKS_PER_SEC;
}

double IOManager::nearestNeighbourBrute(Point* newPoint, ofstream& outFile){
	// "newPoint"  		: the Point whose neighbours I am searching

	Point* bestPoint = NULL;	// the point closest to "newPoint"
	Quantity* minDist;			// the distanve between "newPoint" and the above point
	clock_t start = clock();
	LSH->nearestNeighbourBrute( newPoint, minDist, bestPoint );		// query the LSHT for the true nearest neighbour
	clock_t end = clock();


	if( bestPoint != NULL){
		outFile << "True nearest neighbor: " << bestPoint->name() << endl;
		outFile << "distanceTrue: " << minDist->getString() << endl;
	}

	delete minDist;
	return (end - start)/(double)CLOCKS_PER_SEC;
}

void IOManager::runTests(string& queryPath, ofstream& outfFile){
	ifstream queryFile;
	openFileRead( queryPath, queryFile );						// open the query file to read it

	Quantity* R = getRadius(queryFile);							// get the radius for the inRange() query
	try{
		while ( true ) {										// while the file has more data
			Point* point = getNextPoint( queryFile );			// get the next point

			cout << "Query: " << point->name() << std::endl;

			outfFile << "Query: " << point->name() << std::endl;

			inRange( point, R , outfFile);						// ask for its neighbours in the specified range
			double t1 = nearestNeighbour(point, outfFile);		//,for its approximate nearest neighbour
			double t2 = nearestNeighbourBrute(point, outfFile);	// and for its true nearest neighbour

			outfFile << "tLSH: " << t1 << std::endl;
			outfFile << "tTrue: " << t2 << std::endl;
			outfFile << std::endl;

			delete point;
		}
	}
	catch( EOF_Exception ex ){}
}


/*****************************************************************
***************** HammingManager class methods ********************
******************************************************************/


HammingManager::HammingManager(int l, int k ): IOManager(l, k) {}

void HammingManager::createLSH(int barrier, std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	string buffer;
	getline( file, buffer);		// skip first line

	Dimension = countDimension(dataPath);

	// get the first point, extract its information in order to create the appropriate structures
	Point* firstPoint = getNextPoint(file);


	int TableSize = 1 << K;									// 2^K slots in the Hash Tables

	hashFunctions = new hash_function*[L];					// allocate 1 for each Hash Table
	for( int i = 0; i < L; i++){
		hashFunctions[i] = new Hamming_g( K, Dimension );	// and initialise it
	}

	LSH = new LocalHashTable<Point, Point*>(barrier, L, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)


	// insert the first point we extracted
	LSH->insert( firstPoint );

	int num = 0;
	try{
		while( true ){									// read the whole file
			LSH->insert( getNextPoint(file) );			// store the rest of the points
			num++;
			if( num % 10000 == 0){
				std::cout << "Number of points inserted so far: " << num << std::endl;
			}
		}
	}
	catch( EOF_Exception ex ){}
}

Point* HammingManager::getNextPoint(ifstream& queryFile){	// depends on the format of the file
	string buffer, name;

	if( ! (queryFile >> name >> buffer) ){
		throw EOF_Exception();
	}

	return new HammingPoint(name, (uint64_t)strtoull( buffer.c_str(), NULL , 2) , Dimension );
}

Quantity* HammingManager::getRadius(std::ifstream& queryFile){
	// the radius is stored as as integer in the file
	static QuantityBit result;

	uint64_t R;
	string buffer;
	queryFile >> buffer;			// skip "Radius:"
	queryFile >> R;

	result.set(R);
	return &result;
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



CosineManager::CosineManager(int l, int k )
		: IOManager(l, k) {}

void CosineManager::createLSH(int barrier, std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);
	string buffer, name;
	getline( file, buffer);		// skip first line
	getline( file, buffer);		// skip second line


	Dimension = countDimension( dataPath );					// count the dimensionality of the points
	int TableSize = 1 << K;									// 2^K slots in the Hash Tables

	hashFunctions = new hash_function*[L];					// allocate 1 for each Hash Table
	for( int i = 0; i < L; i++){
		hashFunctions[i] = new Cosine_g( K, Dimension );	// and initialise it
	}


	// create the LSHT (false: not Euclidean!)
	LSH = new LocalHashTable<Point, Point*> (barrier, L, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)

	Point* temp;
	int num = 0;
	try{
		while( true ){								// read the whole file
			temp = getNextPoint(file);
			LSH->insert( temp );					// store all the points
			num++;
			if( num % 10000 == 0){
				std::cout << "Number of points inserted so far: " << num << std::endl;
			}
		}
	}
	catch( EOF_Exception ex ){}
}

Point* CosineManager::getNextPoint(ifstream& queryFile){		// depends on the format of the file
	string buffer, name;

	if( ! (queryFile >> name ) ){
		throw EOF_Exception();
	}

	if( ! getline(queryFile, buffer) ){
		throw EOF_Exception();
	}

	return new CosinePoint(name, buffer , Dimension );
}

Quantity* CosineManager::getRadius(std::ifstream& queryFile){
	// the radius is stored as as double in the file
	static Quantity result;

	double R;
	string buffer;
	queryFile >> buffer;			// skip "Radius:"
	queryFile >> R;

	result.set(R);
	return &result;
}

/*****************************************************************
***************** EuclideanManager class methods ******************
******************************************************************/



EuclideanManager::EuclideanManager(int l, int k, bool flag, int w )
		: IOManager(l, k) {
	W = w;
	Explicit = flag;
}

void EuclideanManager::createLSH(int barrier, std::string dataPath){
	ifstream file;
	openFileRead(dataPath, file);

	string buffer, name;
	getline( file, buffer);			// skip first line
	if( Explicit ){					// "Euclidean" metric was given explicitely, so skip this line
		getline( file, buffer);		// skip second line
	}


	int TableSize  = ( countLines( dataPath , Dimension) - 2 ) / 4;				// Table Size = (#of points) / 4
	hashFunctions = new hash_function*[L];										// allocate 1 for each Hash Table
	for( int i = 0; i < L; i++){
		hashFunctions[i] = new Euclidean_g(TableSize, K, Dimension , W );		// and initialise it
	}

	LSH = new LocalHashTable<Point, Point*> (barrier, L, TableSize, hashFunctions, true );// create the LSHT (true: Euclidean!)

	int num = 0;
	try{
		while( true ){									// read the whole file
			LSH->insert( getNextPoint(file) );			// store all the points
			num++;
			if( num % 10000 == 0){
				std::cout << "Number of points inserted so far: " << num << std::endl;
			}
		}
	}
	catch( EOF_Exception ex ){}
}

Point* EuclideanManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	string buffer, name;

	if( ! (queryFile >> name ) ){
		throw EOF_Exception();
	}

	if( ! getline(queryFile, buffer) ){
		throw EOF_Exception();
	}

	return new EuclideanPoint(name, buffer , Dimension );
}

Quantity* EuclideanManager::getRadius(std::ifstream& queryFile){
	// the radius is stored as as double in the file
	static QuantitySquare result;

	double R;
	string buffer;
	queryFile >> buffer;			// skip "Radius:"
	queryFile >> R;
	result.set(R * R);
	return &result;		// for speed's sake Euclidean distances are copmuted as squares, so return R^2
}



/*****************************************************************
***************** MetricSpaceManager class methods ******************
******************************************************************/



MetricSpaceManager::MetricSpaceManager(int l, int k)
		: IOManager(l, k) {}

void MetricSpaceManager::createLSH(int barrier, std::string dataPath){
	readingData = true;

	ifstream file;
	openFileRead(dataPath, file);

	Dimension = countDimension(dataPath) + 1;
	currentPointIndex = 0;
	Names = new string[Dimension];

	string buffer;
	getline( file, buffer);				// skip first line


	string temp;						// temporary variable for extracting data from the above "string"

	getline(file, temp, ' ');			// skip first word
	getline(file, temp);
	std::istringstream parser( temp );
	for(int i = 0; i < Dimension; i++){
		getline(parser, temp, ',');
		Names[i] = temp;
	}

	int TableSize = 1 << K;									// 2^K slots in the Hash Tables	hash_function** hashFunctions;												// table of hash functions

	Points = new Point*[Dimension];
	for( int i = 0; i < Dimension; i++ ){					// read the whole file
		Points[i] = getNextPoint(file);						// store all the points
		currentPointIndex++;
	}

	hashFunctions = new hash_function*[L];					// allocate 1 for each Hash Table
	for( int i = 0; i < L; i++){
		hashFunctions[i] = new MetricSpace_g( K, Points, Dimension );	// and initialise it
	}

	// create the LSHT (false: not Euclidean!)
	LSH = new LocalHashTable<Point, Point*> (barrier, L, TableSize, hashFunctions, false );	// create the LSHT (false: not Euclidean!)

	int num = 0;
	for( int i = 0; i < Dimension; i++){
		LSH->insert( Points[i] );
		num++;
		if( num % 10000 == 0){
			std::cout << "Number of points inserted so far: " << num << std::endl;
		}
	}

	readingData = false;
}

Point* MetricSpaceManager::getNextPoint(ifstream& queryFile){				// depends on the format of the file
	if( readingData ){
		string buffer;
		getline(queryFile, buffer);
		Point* temp = new MetricSpacePoint( Names[currentPointIndex], Dimension, buffer, currentPointIndex);
		return temp;
	}
	else{
		string name, buffer;
		if( ! (queryFile >> name) ){
			throw EOF_Exception();
		}

		if( ! getline(queryFile, buffer) ){
			throw EOF_Exception();
		}

		Point* temp = new MetricSpacePoint( name, Dimension, buffer, currentPointIndex);
		return temp;
	}
}

Quantity* MetricSpaceManager::getRadius(std::ifstream& queryFile){
	// the radius is stored as as double in the file
	static Quantity result;

	double R;
	string buffer;
	queryFile >> buffer;			// skip "Radius:"
	queryFile >> R;

	result.set(R);
	return &result;
}

MetricSpaceManager::~MetricSpaceManager(){
	delete[] Names;
	delete[] Points;
}
