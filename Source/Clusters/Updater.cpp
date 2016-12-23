#include "Updater.h"
#include "../General/Math.h"

#include <float.h>
#include <iostream>

using namespace std;

Updater::Updater(TriangularMatrix* dPtr, double* j, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints){
	d = dPtr;
	J = j;
	K = k;
	Centers = centers;
	OldCenters = oldCenters;
	AssignedPoints = assignedPoints;

	timesSwapped = 0;
	endOfDay = false;
	minJ = DBL_MAX;
}

void Updater::keepOldCenters(void){
	for(int k = 0; k < K; k++){
		OldCenters[k] = Centers[k];
	}
}

double Updater::PAM_swap(int m, int t){
	// iterate over every point
	// do it through the lists, need to know the cluster that a point belongs to

	// we don't use computeDJ() as the slides suggest
	// after a swap, the 2nd best center may change, so the algorithm does not work for CLARANS
	// also, the distances are pre-computed form the Initializer, so no time is spent in computations
	double J2 = 0.0;
	for( int cluster = 0; cluster < K; cluster++ ){
		for (Node<AssignPair>* inode = AssignedPoints[cluster].start() ; inode != NULL; inode = inode->next() ) {
			if( m == cluster ){
				J2 += (*d)( inode->data()->assigned() , t );
			}
			else{
				J2 += (*d)( inode->data()->assigned() , Centers[cluster] );
			}
		}
	}

	return J2;
}

void Updater::swap(int m, int t, double J2){

	if( endOfDay ){		// if indicated to stop swapping
		return;			// return immediately
	}

	// no, swapping may still occur

	if( J2 < *J ){		// if a better configuration is found
		Centers[m] = t;	// swap the center with "t"
		*J = J2;		// update the objective function
		timesSwapped++;	// another swap occured
	}

	// after "20*K" swaps occur, start checking if the algorithm is just looping endlessly

	if( timesSwapped > 20*K && J2 == minJ ){	// if the minimum value is found (for 2nd time!)
		endOfDay = true;						// mark that no swap will be made, it's pointless!
	}
	else if( timesSwapped > 20*K && J2 < minJ ){// else, if a better configuration is still found
		minJ = J2;								// update the "minJ"
	}

}

double Updater::computeDJ(int cluster, int m, int t, AssignPair* iPair ){
	int i = iPair->assigned();
	int c2 = iPair->center2();

	double dist_it = (*d)(i, t);
	double dist_ic	= (*d)(i, Centers[cluster] );
	double dist_ic2 = (*d)(i, c2);

	if ( cluster == m ) {
		if( dist_it < dist_ic2 ){
			return dist_it - dist_ic;
		}
		else{
			return dist_ic2 - dist_ic;
		}
	}
	else {
		if( dist_it < dist_ic ){
			return dist_it - dist_ic;
		}
		else{
			return 0.0;
		}
	}

}



LloydsUpdate::LloydsUpdate(TriangularMatrix* dPtr, double* j, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints)
	: Updater(dPtr, j, k, centers, oldCenters, assignedPoints){}

AssignPair* LloydsUpdate::findt(int m){

	double min = (double)DBL_MAX;
	AssignPair* tPair = NULL;

	// iterate over the cluster of "m" center
	for (Node<AssignPair>* tnode = AssignedPoints[m].start() ; tnode != NULL; tnode = tnode->next() ) {
		double temp = 0.0;

		// iterate again over the cluster of "m" center, take all possible pairs
		// compute the sum of distances between "t" and the other points
		for (Node<AssignPair>* inode = AssignedPoints[m].start() ; inode != NULL; inode = inode->next() ) {
			temp += (*d)( inode->data()->assigned() , tnode->data()->assigned()  );
		}

		if( temp < min ){			// if a better sum is found
			min = temp;				// update the sum
			tPair = tnode->data();	// keep "t"
		}
	}
	return tPair;					// return "t"
}

void LloydsUpdate::update(void){
	Updater::keepOldCenters();

	for(int m = 0; m < K; m++ ){		// for every cluster
		AssignPair* tPair = findt(m);	// find "t"
		if( tPair != NULL ){
			// if a "t" is present, swap
			double J2 = Updater::PAM_swap( m, tPair->assigned() );
			Updater::swap(m, tPair->assigned(), J2);
		}
	}
}




CLARANS::CLARANS(TriangularMatrix* dPtr, double* j, int n, int k, int* centers, int* oldCenters, List<AssignPair>* assignedPoints, int s, int q)
	: Updater(dPtr, j, k, centers, oldCenters, assignedPoints){
	S = s;
	N = n;

	Q = q;
	if( Q == -1 ){					// if no Q is specified
		Q = 0.12 * K * (N - K);		// follow the e-class slides
		if( Q < 250 ){
			Q = 250;
		}
	}

	Q_n = new int[Q];
	Q_k = new int[Q];
}

bool CLARANS::centroid(int t){
	for(int k = 0; k < K; k++){
		if( Centers[k] == t){
			return true;
		}
	}
	return false;
}

void CLARANS::selectPair(int i){
	do{
		int x = (int)Math::dRand(0.0, K*N - 1.0);
		Q_k[i] = x % K;
		Q_n[i] = x / K;
	} while( centroid(Q_n[i]) );
}

void CLARANS::update(void){
	Updater::keepOldCenters();

	for(int s = 0; s < S; s++){			// itearte "S" times
		int bestIndex = -1;				// keep the best pair
		double bestJ = 0.0;				// and thematching "J"
		for(int i = 0; i < Q; i++ ){	// Q times
			selectPair(i);				// select a pair
			double tempJ = Updater::PAM_swap(Q_k[i], Q_n[i]);

			if( tempJ < *J || bestIndex == -1 ){	// if the swap is the best so far
				bestIndex = i;						// keep the pair
				bestJ = tempJ;						// and the "J" value
			}
		}
		Updater::swap(Q_k[bestIndex], Q_n[bestIndex], bestJ);
	}

}


CLARANS::~CLARANS(){
	delete[] Q_n;
	delete[] Q_k;
}
