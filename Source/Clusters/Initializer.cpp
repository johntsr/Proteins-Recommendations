#include "Initializer.h"
#include "../General/Math.h"

#include <iostream>

using namespace std;

Initializer::Initializer( int* centers, TriangularMatrix* dPtr, int k, int n ){
	Centers = centers;
	d = dPtr;
	K = k;
	N = n;
}


Park_Jun::Park_Jun( int* centers, TriangularMatrix* dPtr, int k, int n )
	: Initializer( centers, dPtr, k, n ){}

void Park_Jun::initialise(void){
	// compute v[i] as the e-class slides instruct us
	double* v = new double[N];

	double sumT = 0.0;
	for(int j = 0; j < N; j++){
		for(int t = 0; t < N; t++){
			sumT += (*d)(j,t);
		}
	}

	for(int i = 0; i < N; i++){
		v[i] = 0.0;
		for(int j = 0; j < N; j++){
			v[i] += (*d)(i,j) / sumT;
		}
	}

	// next, we sort v[] table and pick the 4 points that correspond to the 4 smallest values
	// in order to maintain the matching between "point i" and "v[i]", a table of indexes keeps track
	// of the changes that occur during the sort
	int* indexes = new int[N];
	for(int i = 0; i < N; i++){
		indexes[i] = i;				// initially, "point i" matches the value "v[i]"
	}

	Math::sort(v, indexes, N);

	for(int i = 0; i < K; i++){
		Centers[i] = indexes[i];	// store the indexes of the 4 points in the centers table
		std::cout << "center["<<i<<"] = " << Centers[i] << '\n';
	}

	delete[] indexes;
	delete[] v;
}




K_MedoidsPP::K_MedoidsPP( int* centers, TriangularMatrix* dPtr, int k, int n )
	: Initializer( centers, dPtr, k, n ){
	}

void K_MedoidsPP::initialise(void){
	double* DistMin = new double[N];				// minimum distance of point "i" from cluster
	double* P = new double[N+1];					// the P[] table from the slides
	P[0] = 0.0;

	Centers[0] = (int)Math::dRand(0,N-1);			// initially, pick a random point
	int k = 1;										// 1 point so far is selected
	int r = 0;										// the center that I choose in each loop
	do {											// repeat

		for(int i = 1; i < N + 1; i++){				// for every point
			DistMin[i-1] = minFromCluster(i-1, k);	// find its minimum distance
			P[i] = P[i-1] + DistMin[i-1];			// compute its P[i] value
		}

		do{
			double x;
			do {
				x = Math::dRand(0.0, P[N]);			// pick a random number
			} while( !(x > 0.0) );
			r = Math::binarySearch(P, N+1, x) - 1;	// search in the P table the matching point
		} while( !uniqueCenter(k, r) );				// repeat if this point is already chosen as center
		Centers[k] = r;								// pick that point as a center
		k++;										// another point was selected
	} while( k < K );								// repeat untill "K" centers are chosen

	delete[] DistMin;
	delete[] P;
}

double K_MedoidsPP::minFromCluster(int i, int k){

	double min = (*d)(i, Centers[0]);
	for( int center = 1; center < k; center++){					// iterate over the centers
		double temp = (*d)(i, Centers[center]);					// store the distance from point "i"
		if( temp < min ){										// if the distance is smaller than "min"
			min = temp;											// update "min"
		}
	}
	return min;
}

bool K_MedoidsPP::uniqueCenter(int k, int r){
	for(int i = 0; i < k; i++){
		if( Centers[k] == r){
			return false;
		}
	}
	return true;
}


RandInit::RandInit( int* centers, TriangularMatrix* dPtr, int k, int n )
: Initializer( centers, dPtr, k, n ){}

void RandInit::initialise(void){
	for(int k = 0; k < K; k++){
		Centers[k] = (int)Math::dRand(0,N-1);
	}
}
