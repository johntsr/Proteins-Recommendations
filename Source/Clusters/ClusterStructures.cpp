#include "ClusterStructures.h"

#include <iostream>

TriangularMatrix::TriangularMatrix(int n, Point** PointTable ){
	N = n;							// the matrix has N rows

	d = new Quantity**[N];			// allocate pointers for these N rows
	for(int i = 0; i < N; i++){
		d[i] = new Quantity*[i];	// row number "i" has "i" elements : triangular!
	}

	Zero = new Quantity(0.0);

	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			d[i][j] = PointTable[i]->distance( PointTable[j] );	// fill the distance matrix
		}
	}
}

Quantity*& TriangularMatrix::operator ()(int i, int j){
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

TriangularMatrix::~TriangularMatrix(){

	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			delete d[i][j];
		}
		delete[] d[i];
	}
	delete[] d;

	delete Zero;
}
