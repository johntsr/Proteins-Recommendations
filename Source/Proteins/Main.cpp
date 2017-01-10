#include "ProteinsManager.h"
#include "../General/Math.h"
#include "../General/Timing.h"

using namespace std;

#define INTERVAL 5000

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

void parseArguments(int argc, char** argv, string& dataPath, string& outCPath, string& outDPath, bool& CUTest );




int main(int argc, char *argv[]) {
	srand(time(NULL));
	setup(INTERVAL);

	string dataPath, outCPath, outDPath;
	fstream dataFile;
	bool complete = false;
	bool CUTest = false;

	parseArguments(argc, argv, dataPath, outCPath, outDPath, CUTest );

	ProteinsManager* cManager =  new cRMSDManager(complete);

	int ManagersNum = 7;
	ProteinsManager* dManagers[ManagersNum];
	pickOption T[ManagersNum] = {SMALLEST, LARGEST, RANDOM, SMALLEST, LARGEST, RANDOM, SMALLEST};

	rOption r[ManagersNum] = { SMALL, SMALL, SMALL, MEDIUM, MEDIUM, MEDIUM, LARGE};
	for(int i = 0; i < ManagersNum; i++){
		dManagers[i] = new dRMSDManager(T[i], r[i], complete);
	 }

	getPath( dataPath, "Please, enter the path for the data set file" );
	openFile(dataPath, dataFile);

	ofstream file( outDPath.c_str() );
	for(int i = 0; i < ManagersNum; i++){
		dManagers[i]->run(dataPath, outDPath);
		if( CUTest ){
			dManagers[i]->runCUTests();
		}
	}

	for(int i = 0; i < ManagersNum; i++){
		delete dManagers[i];
	}

	if(cManager != NULL){
		ofstream file( outCPath.c_str() );
		cManager->run(dataPath, outCPath);
		if( CUTest ){
			cManager->runCUTests();
		}
		delete cManager;
	}
	return 0;
}

void parseArguments(int argc, char** argv, string& dataPath, string& outCPath, string& outDPath, bool& CUTest){

	dataPath = outCPath = outDPath = "";
	CUTest = false;

	for(int i = 0 ; i < argc; i++){
		if( !strcmp( "-d", argv[i] ) ){
			dataPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-od", argv[i] ) ){
			outDPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-oc", argv[i] ) ){
			outCPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-cu", argv[i] ) ){
			CUTest = true;
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
