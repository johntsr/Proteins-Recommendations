#include "Proteins/ProteinsManager.h"
#include "General/Math.h"

using namespace std;

#include <bitset>

double Point::C = 1.0;


void createBinaryFile(int numOfBits){
	//ofstream file("ModifiedDataHamming.csv");
	ofstream file("ModifiedBigDataHamming.csv");
	//int num = 0;
	string buffer;
	/*while( num < ( 1 << numOfBits ) ){
		file << "item" << num << " " << (bitset<8>(num)) << endl;
		num++;
	}*/
	for (int i=0; i<1000; i++) {
		//num = Math::dRand(0 , 65535);
		//file << "item" << num << " " << (bitset<16>(num)) << endl;
		int num = rand();
		int num2 = rand();
		file << "item" << i << " " << (bitset<31>(num)) << (bitset<31>(num2)) << endl;
	}
}

void createVectorFile(void){
	ofstream file("EuclideanBig.csv");
	string buffer;
	file << "@metric_space vector" << endl;
	file << "@metric euclidean" << endl;
	int num = 100000;
	int dim = 1000;
	while(  num-- > 0 ){
		file << "item" << num << " ";
		for(int i = 0 ; i < dim ; i++){
			file << Math::dRand(-dim, dim) << " ";
		}
		file << endl;
	}

	exit(0);
}

void createVectorQueryFile(void){
	ofstream file("EuclideanQuery.csv");
	string buffer;
	file << "Radius: 100" << endl;
	int num = 10;
	int dim = 1000;
	while(  num-- > 0 ){
		file << "item" << num << " ";
		for(int i = 0 ; i < dim ; i++){
			file << Math::dRand(-1000, 1000) << " ";
		}
		file << endl;
	}

	exit(0);
}

void createProteins(void){
	ofstream file("DataSets/ProteinsSmall.csv");

	int numConform = 100;
	int N = 6;

	file << "numConform: " << numConform << endl;
	file << "N: " << N << endl;

	for(int i = 0; i < numConform; i++){
		for(int j = 0; j < N; j++){
			file << Math::dRand(-100.0, 100.0) << " ";
			file << Math::dRand(-100.0, 100.0) << " ";
			file << Math::dRand(-100.0, 100.0) << '\n';
		}
	}

	file.close();

	// exit(0);
}

void getPath(string& path, string message);

void openFile(string& path, fstream& file);

void parseArguments(int argc, char** argv, bool& complete, string& dataPath, string& outPath, string& confPath, bool& CUTest );
void parseConfiguration( string& confPath, int& K_clusters, int& K_hash, int& L, int& Q, int& S );

int low(int N){
	return N;
}

int medium(int N){
	return N * sqrt(N);
}

int high(int N){
	return N * (N - 1) / 2;
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	//createBinaryFile(64);
	//createVectorFile();
	//createVectorQueryFile();
	// createProteins();

	string dataPath, outPath, confPath, metric;
	fstream dataFile;
	bool complete = false;
	bool CUTest = false;

	parseArguments(argc, argv, complete, dataPath, outPath, confPath, CUTest );

	int K_clusters, K_hash, L, Q, S;
	parseConfiguration( confPath, K_clusters, K_hash, L, Q, S );

	ProteinsManager* cManager =  new cRMSDManager(complete);

	int ManagersNum = 7;
	ProteinsManager* dManagers[ManagersNum];
	dOption T[ManagersNum] = {SMALLEST, LARGEST, RANDOM, SMALLEST, LARGEST, RANDOM, SMALLEST};
	rGenerator r[ManagersNum] = { &low, &low, &low, &medium, &medium, &medium, &high};
	for(int i = 0; i < ManagersNum; i++){
		dManagers[i] = new dRMSDManager( T[i], r[i], complete);
	 }

	getPath( dataPath, "Please, enter the path for the data set file" );
	openFile(dataPath, dataFile);

	if( dManagers[0] != NULL ){
		outPath = "experim.dat";
		ofstream file( outPath.c_str() );
		for(int i = 0; i < ManagersNum; i++){
			dManagers[i]->run(dataPath, outPath);
			if( CUTest ){
				dManagers[i]->runCUTests();
			}
			delete dManagers[i];
		}
	}

	if(cManager != NULL){
		outPath = "conform.dat";
		ofstream file( outPath.c_str() );
		cManager->run(dataPath, outPath);
		if( CUTest ){
			cManager->runCUTests();
		}
		delete cManager;
	}

	return 0;
}

void parseArguments(int argc, char** argv, bool& complete, string& dataPath, string& outPath, string& confPath, bool& CUTest ){

	dataPath = outPath = confPath = "";
	complete = false;

	for(int i = 0 ; i < argc; i++){
		if( !strcmp( "-complete", argv[i] ) ){
			complete = true;
		}
		else if( !strcmp( "-cu", argv[i] ) ){
			CUTest = true;
		}
		else if( !strcmp( "-d", argv[i] ) ){
			dataPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-c", argv[i] ) ){
			confPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-o", argv[i] ) ){
			outPath = argv[i+1];
			i++;
		}
	}

}

void getPath(string& path, string message){
	if( path != "" ){											// if the path is already set
		return;													// just return
	}

	cout << message << endl;
	cin >> path;
}

void openFile(string& path, fstream& file){
	file.open( path.c_str() );
	while( !file.is_open() ){
		getPath(path, "Cannot open file : " + path + ". Please, enter a valid path." );
		file.open( path.c_str() );
	}
}

void parseConfiguration( string& confPath, int& K_clusters, int& K_hash, int& L, int& Q, int& S ){
	K_hash = 4;
	L = 5;
	Q = -1;
	S = 2;

	string temp;
	fstream confFile;
	openFile(confPath, confFile);

	confFile >> temp;	// skip "number_of_clusters"
	confFile >> K_clusters;

	while ( confFile >> temp ) {
		if( temp == "number_of_hash_functions:" ){
			confFile >> K_hash;
		}
		else if( temp == "number_of_hash_tables:" ){
			confFile >> L;
		}
		else if( temp == "clarans_set_fraction:" ){
			confFile >> Q;
		}
		else if( temp == "clarans_iterations:" ){
			confFile >> S;
		}
	}
}
