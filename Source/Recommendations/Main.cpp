#include "RecommendManager.h"
#include "../General/Math.h"

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

void parseArguments(int argc, char** argv, string& dataPath, string& outPath, bool& CUTest );


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

	string dataPath, outPath;
	fstream dataFile;
	bool complete = false;
	bool CUTest = false;

	parseArguments(argc, argv, dataPath, outPath, CUTest );

	RecommendManager* manager =  new NNRecommendManager(complete);

	getPath( dataPath, "Please, enter the path for the data set file" );
	openFile(dataPath, dataFile);

	if(manager != NULL){
		ofstream file( outPath.c_str() );
		manager->run(dataPath, outPath);
		if( CUTest ){
			manager->runCUTests();
		}
		delete manager;
	}

	return 0;
}

void parseArguments(int argc, char** argv, string& dataPath, string& outPath, bool& CUTest){

	dataPath = outPath = "";
	CUTest = false;

	for(int i = 0 ; i < argc; i++){
		if( !strcmp( "-d", argv[i] ) ){
			dataPath = argv[i+1];
			i++;
		}
		else if( !strcmp( "-o", argv[i] ) ){
			outPath = argv[i+1];
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
