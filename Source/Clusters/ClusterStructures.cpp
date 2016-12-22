#include "ClusterStructures.h"

#include <iostream>
#include <cstdlib>
#include <time.h>

TriangularMatrix::TriangularMatrix(int n, Point** PointTable ){
	N = n;							// the matrix has N rows

	d = new Quantity**[N];			// allocate pointers for these N rows
	for(int i = 0; i < N; i++){
		d[i] = new Quantity*[i];	// row number "i" has "i" elements : triangular!
	}

	Zero = new Quantity(0.0);


	// clock_t start = clock();		// start measuring time

	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			Quantity* temp = PointTable[i]->distance( PointTable[j] );	// fill the distance matrix
			d[i][j] = temp;
		}
	}

	// clock_t end = clock();			// stop measuring
	// std::cout << "time needed = " <<  (end - start)/(double)CLOCKS_PER_SEC << '\n';

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

void TriangularMatrix::print(void){
	for(int i = 0; i < N; i++){
		for(int j = 0; j < i; j++){
			std::cout << d[i][j]->castAsDouble() << std::endl;
		}
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
