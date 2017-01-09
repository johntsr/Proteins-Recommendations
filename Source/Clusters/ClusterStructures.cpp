#include "ClusterStructures.h"

#include <iostream>
#include <cstdlib>
#include <time.h>

TriangularMatrix::TriangularMatrix(int n, Point** PointTable ){
	N = n;							// the matrix has N rows

	d = new double*[N];			// allocate pointers for these N rows
	for(int i = 0; i < N; i++){
		d[i] = new double[i];	// row number "i" has "i" elements : triangular!
	}

	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			d[i][j] = PointTable[i]->distance( PointTable[j] );
		}
	}

	Zero = 0.0;

}

double& TriangularMatrix::operator ()(int i, int j){
	if( i == j ){				// i == j => main diagonal => always 0
		return Zero;
	}
	else if( i < j ){			// i < j => upper triangular part => get corresponding lower part
		return d[j][i];
	}
	else{						// i > j => procceed normally
		return d[i][j];
	}

}

void TriangularMatrix::print(void){
	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			std::cout << d[i][j] << std::endl;
		}
	}
}

TriangularMatrix::~TriangularMatrix(){
	if( d != NULL ){
		for(int i = 0; i < N; i++){
			delete[] d[i];
		}
		delete[] d;
	}
}




TriangularMatrixLazy::TriangularMatrixLazy(int n, Point** pointTable){
	N = n;							// the matrix has N rows
	PointTable = pointTable;
	Zero = 0.0;

	d = new double*[N];			// allocate pointers for these N rows
	for(int i = 0; i < N; i++){
		d[i] = new double[i];	// row number "i" has "i" elements : triangular!
		for(int j = 0; j < i; j++){
			d[i][j] = -1.0;
		}
	}
}


double& TriangularMatrixLazy::operator ()(int i, int j){
	// if( i == j ){				// i == j => main diagonal => always 0
	// 	return Zero;
	// }
	// else{
	// 	result = PointTable[i]->distance( PointTable[j] );
	// 	return result;
	// }

	if( i == j ){				// i == j => main diagonal => always 0
		return Zero;
	}
	else if( i < j ){			// i < j => upper triangular part => get corresponding lower part

		if( d[j][i] == -1.0 ){
			d[j][i] = PointTable[i]->distance( PointTable[j] );
		}
		return d[j][i];

	}
	else{						// i > j => procceed normally
		if( d[i][j] == -1.0 ){
			d[i][j] = PointTable[i]->distance( PointTable[j] );
		}
		return d[i][j];

	}

}
