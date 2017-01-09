#include "ProteinsManager.h"
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

void parseArguments(int argc, char** argv, string& dataPath, string& outCPath, string& outDPath, bool& CUTest );


int low(int N){
	return N;
}

int medium(int N){
	return N * sqrt(N);
}

int high(int N){
	return N * (N - 1) / 2;
}








#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>


double computeDistance1(double* x, double* y, int N){
	// std::cout << "get in!" << std::endl;
	double u[3*3];

	gsl_matrix_view X = gsl_matrix_view_array(x, N, 3);
	gsl_matrix_view Y = gsl_matrix_view_array(y, N, 3);
	gsl_matrix_view U = gsl_matrix_view_array(u, 3, 3);

	/* Compute C = A B */

	// TODO: C IS REPLACED IS BY C!!!!
	gsl_blas_dgemm (CblasTrans, CblasNoTrans,
	              1.0, &X.matrix, &Y.matrix,
	              0.0, &U.matrix);


	gsl_matrix * V = gsl_matrix_alloc(3, 3);
	gsl_vector * S = gsl_vector_alloc(3);
	gsl_vector * work = gsl_vector_alloc(3);

	// TODO: C IS REPLACED IS BY U!!!!
	gsl_linalg_SV_decomp(&U.matrix, V, S, work);

	// for(int i = 0; i < 3; i++){
	// 	for(int j = 0; j < 3; j++){
	// 		std::cout << gsl_matrix_get (V, i, j) << ' ';
	// 	}
	// 	std::cout << '\n';
	// }
	//
	// std::cout << '\n';
	// std::cout << '\n';
	// for(int i = 0; i < 3; i++){
	// 	for(int j = 0; j < 3; j++){
	// 		std::cout << u[i*3 + j] << ' ';
	// 	}
	// 	std::cout << '\n';
	// }
	//
	// std::cout << '\n';
	// std::cout << '\n';
	// for(int j = 0; j < 3; j++){
	// 	std::cout << gsl_vector_get (S, j) << ' ';
	// 	std::cout << '\n';
	// }



	double q[3*3];
	gsl_matrix_view Q = gsl_matrix_view_array(q, 3, 3);
	gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &U.matrix, V, 0.0, &Q.matrix);

	std::cout << "before q = " << '\n';
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			std::cout << q[i*3 + j] << ' ';
		}
		std::cout << '\n';
	}

	std::cout << "before u = " << '\n';
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			std::cout << u[i*3 + j] << ' ';
		}
		std::cout << '\n';
	}

	int signum = 0.0;
	gsl_permutation *p = gsl_permutation_alloc(3);
	gsl_matrix *tmpQ = gsl_matrix_alloc(3, 3);
	gsl_matrix_memcpy(tmpQ, &Q.matrix);
	gsl_linalg_LU_decomp(tmpQ , p , &signum);
	double det = gsl_linalg_LU_det(tmpQ, signum);
	std::cout << "det = " << det << '\n';
	if( det < 0.0 ){
		for(int i = 0; i < 3; i++){
			u[i*3 + 2] *= -1.0;
		}

		gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &U.matrix, V, 0.0, &Q.matrix);
	}

	std::cout << "after q = " << '\n';
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			std::cout << q[i*3 + j] << ' ';
		}
		std::cout << '\n';
	}

	std::cout << "after u = " << '\n';
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			std::cout << u[i*3 + j] << ' ';
		}
		std::cout << '\n';
	}
	// exit(0);


	double* d = new double[N*3];
	gsl_matrix_view Diff = gsl_matrix_view_array(d, N, 3);
	gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &X.matrix, &Q.matrix, 0.0, &Diff.matrix);
	gsl_matrix_sub(&Diff.matrix, &Y.matrix);

	double result = 0.0;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < 3; j++){
			result += d[i*3 + j] * d[i*3 + j];
		}
	}
	result /= sqrt(N);


	gsl_permutation_free(p);
	gsl_matrix_free(tmpQ);
	gsl_matrix_free(V);
	gsl_vector_free(S);
	gsl_vector_free(work);
	delete[] d;

	// std::cout << "get out! (result = " << result << ")" << std::endl;
	return result;
}


// { 	{1, 2, 3},
// 	{5, 6, 7},
// 	{9, 10, 11},
// 	{13, 14, 15}
// }

// { 	{11, 2, 31},
// 	{51, 6, 71},
// 	{91, 10, 11},
// 	{13, 114, 15}
// }

int main(int argc, char *argv[]) {

	// double x[] = { 	1, 2, 3,
	//  				5, 6, 7,
	// 				9, 10, 11,
	// 				13, 14, 15};
	//
	// double y[] = { 	11, 2, 31,
	//  				51, 6, 71,
	// 				91, 10, 11,
	// 				13, 114, 15};
	//
	// std::cout << "dist = " << computeDistance1(x, y, 4) << '\n';
	//
	// return 0;

	srand(time(NULL));
	//createBinaryFile(64);
	//createVectorFile();
	//createVectorQueryFile();
	// createProteins();

	string dataPath, outCPath, outDPath;
	fstream dataFile;
	bool complete = false;
	bool CUTest = false;

	parseArguments(argc, argv, dataPath, outCPath, outDPath, CUTest );

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

	if(cManager != NULL){
		ofstream file( outCPath.c_str() );
		cManager->run(dataPath, outCPath);
		if( CUTest ){
			cManager->runCUTests();
		}
		delete cManager;
	}
	exit(0);

	if( dManagers[0] != NULL ){
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
