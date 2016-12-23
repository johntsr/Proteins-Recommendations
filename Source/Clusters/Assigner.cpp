#include "Assigner.h"

#include "../General/Quantity.h"

#include "../Metrics/Hamming.h"
#include "../Metrics/CosineSimilarity.h"
#include "../Metrics/Euclidean.h"
#include "../Metrics/MetricSpace.h"

Assigner::Assigner(TriangularMatrix* dPtr, int n, int* centers, int k, List<AssignPair>* assignedPoints ){
	d = dPtr;
	N = n;
	Centers = centers;
	K = k;
	AssignedPoints = assignedPoints;
}

void Assigner::assignBrute(int i, int& center, int& center2){
	double min  = 0.0;										// just an initial value
	double min2 = 0.0;										// just an initial value
	for(int k = 0; k < K; k++){								// iterate over centers to find the min distance from the point
		double temp = (*d)( i, Centers[k] );				// get the distance

		if( center == NONE || temp < min ){					// if it is smaller than the so-far-best
			min = temp;										// update the distance
			center2 = center;								// the best
			center = k;										// and the second best
		}
		else if( center2 == NONE || temp < min2 ){
			min2 = temp;
			center2 = k;
		}

	}
}

void Assigner::flush(void){
	for(int k = 0; k < K; k++){									// before each assignment
		AssignedPoints[k].flush();								// empty the clusters
	}
}

PAM_Simple::PAM_Simple(TriangularMatrix* dPtr, int n, int* centers, int k, List<AssignPair>* assignedPoints )
		: Assigner(dPtr, n, centers, k, assignedPoints ){
}

double PAM_Simple::assign(void){

	Assigner::flush();

	double J = 0.0;												// the objective function will be computed here
	for(int i = 0; i < N; i++){									// for every point
		int center  = NONE;										// keep both the closest center	 (just an initial value )
		int center2 = NONE;										// and the second closest center (just an initial value )

		Assigner::assignBrute(i, center, center2);

		AssignPair* pair = new AssignPair( i , Centers[center2] );// create a new pair (point, second best center)
		AssignedPoints[center].insertAtStart( pair , true );	// and insert it in the cluster

		J += (*d)(i, Centers[center]);							// update the objective Function
	}

	return J;
}



void Reverse_LSH::lshArguments(int& TableSize, hash_function**& hashFunctions, int LSH_K, int L){
	int W 	  = 4;
	int Dimension = PointTable[0]->dimension();				// the dimension can be determined by the 1st point
	PointType type = PointTable[0]->type();					// so does the type

	hashFunctions = new hash_function*[L];					// allocate 1 for each Hash Table
	switch ( type ) {
		case HAMMING:
			TableSize = 1 << LSH_K;							// 2^K slots in the Hash Tables
			for( int i = 0; i < L; i++){
				hashFunctions[i] = new Hamming_g( LSH_K, Dimension );
			}

			break;
		case COSINE:
			TableSize = 1 << LSH_K;									// 2^K slots in the Hash Tables
			for( int i = 0; i < L; i++){
				hashFunctions[i] = new Cosine_g( LSH_K, Dimension );
			}
			break;
		case EUCLIDEAN:
			TableSize = N / 8;
			for( int i = 0; i < L; i++){
				hashFunctions[i] = new Euclidean_g(TableSize, LSH_K, Dimension , W );
			}
			break;
		case METRIC_SPACE:
			TableSize = 1 << LSH_K;									// 2^K slots in the Hash Tables
			for( int i = 0; i < L; i++){
				hashFunctions[i] = new MetricSpace_g( LSH_K, PointTable, Dimension );
			}
			break;
		default:
			break;
	}
}

Reverse_LSH::Reverse_LSH(TriangularMatrix* dPtr, Point** pointTable, int n, int* centers, int k, List<AssignPair>* assignedPoints, int k_hash, int l)
	: Assigner(dPtr, n, centers, k, assignedPoints ) {

	PointTable = pointTable;				// table of points for the assigner

	L = l;
	int LSH_K = k_hash;
	int barrier = 3*L;

	int TableSize = 0;

	lshArguments( TableSize, hashFunctions, LSH_K, L );

	PointMap = new HashTable<PointIndex, Point*> (TableSize);

	// create the LSH
	bool isEuclidean = ( PointTable[0]->type() == EUCLIDEAN );
	LSH = new LocalHashTable<Point, Point*> (barrier, L, TableSize, hashFunctions, isEuclidean );

	for( int i = 0; i < N; i++ ){
		LSH->insert( PointTable[i] );					// store all the points
		PointMap->insert( new PointIndex(i, PointTable[i]), true );	// map every point to its index in the "global" table
	}

	Barrier = new BarrierPoint();						// create the arrier point
	LSH->prepareBarrier(Barrier);						// inform the LSH about the barrier

	// for every point, store information
	NearestCenter = new int[N];							// what the nearest center is
	NearestCenter2 = new int[N];						// what the 2nd nearest center is

}

double Reverse_LSH::assign(void){

	Assigner::flush();												// reset the clusters
	double R = minDistBetweenCenters();							// find the min distance between 2 centers (initialize "R" of query)
	for(int i = 0; i < N; i++){										// initially, no best and second best centers are present
		NearestCenter[i] = NONE;
		NearestCenter2[i] = NONE;
	}

	List<Point,Point* > * ResultPoints = new List<Point,Point* >[K];// list of points I match through the LSH queries
	int countEmpty = 0;												// count how many of thw above lists are empty after a query
	int countBarrier = K/2;											// max number of empty lists: if exceeded, exit the loop

	do {															// repeat
		countEmpty = 0;												// initialize its value to 0
		for(int k = 0; k < K; k++){									// for every cluster
			ResultPoints[k].flush();								// reset its result-list (output of previous query)
			LSH->inRangeBarrier( PointTable[Centers[k]], R, ResultPoints[k] );	// perform a query
			assignToCenters( ResultPoints, k );						// assignt the points found to their respective centers
			if( ResultPoints[k].empty() ){							// if a list was empty
				countEmpty++;										// count it
			}
		}
		LSH->markPoints();											// mark the assigned points to skip them in next queries
		R = PointTable[0]->multiplyDouble(R, 2.0);					// prepare the "R" for the query of the next loop
	} while( countEmpty < countBarrier );							// until too many queries return empty results

	assignRestToCenters();											// assign the rest of the points to their centers

	double J = 0.0;													// the value of the objective function
	for( int i = 0; i < N; i++){									// for every point
		AssignPair* pair = new AssignPair( i, Centers[NearestCenter2[i]] );	// pair of a point and its second closest center
		AssignedPoints[ NearestCenter[i] ].insertAtStart( pair, true );	// add that pair to the cluster
		J += (*d)(i, Centers[NearestCenter[i]] );					// update the value of the objective function
	}

	LSH->unmarkPoints();											// before exit, reset the inner structures of the LSH

	delete[] ResultPoints;
	return J;
}

void Reverse_LSH::assignRestToCenters(void){
	for (int i = 0; i < N; i++ ) {
		// if the LSH did not find a 2nd best (implicitely, it didn't find the best either)
		// assign the old-fashion way
		if( NearestCenter2[i] == NONE ){
			Assigner::assignBrute(i, NearestCenter[i], NearestCenter2[i]);
		}
	}
}

void Reverse_LSH::assignToCenters(List<Point,Point* >* ResultPoints, int k){
	// process the results of the query for center "k" (called once for every "R")

	for (Node<Point>* node = ResultPoints[k].start() ; node != NULL; node = node->next() ) {
		int i = (*PointMap)[node->data()]->i;		// get the index of a result point

		if( NearestCenter[i] == NONE ){				// if no best center has been found yet
			NearestCenter[i] = k;					// then make this center the best center
		}
		else{										// now, a best center has been found
			if( (*d)(i, Centers[k]) < (*d)(i, Centers[NearestCenter[i]] ) ){	// if an even better center has been found
				NearestCenter2[i] = NearestCenter[i];							// update it
				NearestCenter[i] = k;
			}
			else{																// now, check for second best, same logic applies
				if( NearestCenter2[i] == NONE ){
					NearestCenter2[i] = k;
				}
				else if( (*d)(i, Centers[k]) < (*d)(i, Centers[NearestCenter2[i]] ) ){
					NearestCenter2[i] = k;
				}
			}
		}
	}
}

double Reverse_LSH::minDistBetweenCenters(void){
	double minDist  = DBL_MAX;										// initialise to infinity
	for(int k1 = 0; k1 < K; k1++){									// for every
		for( int k2 = 0; k2 < k1; k2++ ){							// 	pair of clusters
			double temp = (*d)(Centers[k1] , Centers[k2]);
			if( temp < minDist ){									// if a better pair is found
				minDist = temp;										// keep its distance
			}
		}
	}
	return minDist;
}

Reverse_LSH::~Reverse_LSH(){
	delete[] NearestCenter;
	delete[] NearestCenter2;
	delete LSH;
	delete Barrier;
	delete PointMap;

	for( int i = 0; i < L; i++){
		delete hashFunctions[i];
	}
	delete[] hashFunctions;
}
