#include "Clusters.h"
#include "../General/Math.h"
#include "../Metrics/Hamming.h"

#include "CUnit/Basic.h"
#include <sstream>
#include <time.h>
#include <float.h>
#include <iostream>

using namespace std;

int initSuite(void){
	std::cout << "init!" << '\n';
	return 0;
}

int finalSuite(void){
	std::cout << "\nfinal!" << '\n';
	return 0;
}

void Clustering::printInfo(std::ofstream& outfFile){
	outfFile << message << '\n';
}

ClusterAlgorithm::ClusterAlgorithm(Point** pointTable, TriangularMatrix* dPtr, int n, int k, int type, int k_hash, int l, int q, int s) {

	N = n;											// keep the number of points
	PointTable = pointTable;						// the table is filled by the caller, just keep a reference to it
	d = dPtr;										// the distances are also computed by the caller
	K = k;											// keep the number of clusters
	Centers = new int[K];							// allocate space for the "K" centers
	OldCenters = new int[K];						// also for the previous centers
	AssignedPoints = new List<AssignPair>[K];		// "K" clusters in form of lists

	SumSilhouette = DBL_MAX;
	ClusterSilhouette = new double[K];

	for(int k = 0; k < K; k++){
		OldCenters[k] = NONE;						// initially, no old centers are present
	}


	message = "Algorithm: ";

	// pick an initializer
	if( (type / 4) % 2 == 0){
		Init = new Park_Jun( Centers, d, K, N );
		message += "I1";
	}
	else{
		message += "I2";
		Init = new K_MedoidsPP( Centers, d, K, N );
	}

	// pick an assigner
	if( (type / 2) % 2 == 0){
		message += "A1";
		Assign = new PAM_Simple( d, N, Centers, K, AssignedPoints);
	}
	else{
		message += "A2";
		Assign = new Reverse_LSH( d, PointTable, N, Centers, K, AssignedPoints, k_hash, l);
	}

	// pick an updater
	if( (type / 1) % 2 == 0){
		message += "U1";
		Update = new LloydsUpdate(d, &J, K, Centers, OldCenters, AssignedPoints);
	}
	else{
		message += "U2";
		Update = new CLARANS(d, &J, N, K, Centers, OldCenters, AssignedPoints, s, q);
	}

}

void ClusterAlgorithm::run(void){
	clock_t start = clock();		// start measuring time
	Init->initialise();				// initialise the clusters
	do {							// repeat
		J = Assign->assign();		// assign the points to clusters
		Update->update();			// update the centers
	} while( changes() );			// while changes occur in the centers
	clock_t end = clock();			// stop measuring
	execTime = (end - start)/(double)CLOCKS_PER_SEC;
}

bool ClusterAlgorithm::changes(void){
	for(int k = 0; k < K; k++){						// for every new center

		bool found = false;							// initially, not found
		for(int oldk = 0; oldk < K; oldk++){		// search the old clusters
			if( OldCenters[oldk] == Centers[k] ){	// if found
				found = true;						// keep it
			}
		}

		if(!found){									// if ont found
			return true;							// changes occured
		}

	}
	return false;									// here, everything is the same
}

void ClusterAlgorithm::printClusters(std::ofstream& outfFile){
	for(int k = 0; k < K; k++){
		outfFile << "CLUSTER-" << k + 1 << " { ";
		for (Node<AssignPair>* inode = AssignedPoints[k].start() ; inode != NULL; inode = inode->next() ) {
			outfFile << PointTable[inode->data()->assigned()]->name();
			if( inode->next() != NULL ){
				outfFile << ", ";
			}
		}
		outfFile << " }" << std::endl;
	}
}

void ClusterAlgorithm::printCenters(std::ofstream& outfFile){
	for(int k = 0; k < K; k++){
		outfFile << "CLUSTER-" << k + 1 << " { size: " << AssignedPoints[k].count();
		outfFile << ", medoid: " << PointTable[Centers[k]]->name() << " } " << std::endl;
	}
}

void ClusterAlgorithm::computeSilhouette(void){
	SumSilhouette = 0.0;
	for(int k = 0; k < K; k++){				// for every cluster, compute its Silhouette
		ClusterSilhouette[k] = 0.0;

		// for every point in the cluster
		for (Node<AssignPair>* inode = AssignedPoints[k].start() ; inode != NULL; inode = inode->next() ) {
			double a_i = avgDistFromCluster( inode->data()->assigned(), Centers[k] );
			double b_i = avgDistFromCluster( inode->data()->assigned(), inode->data()->center2() );
			if( a_i > b_i){
				ClusterSilhouette[k] += (b_i - a_i) / a_i;
			}
			else{
				ClusterSilhouette[k] += (b_i - a_i) / b_i;
			}
		}
		ClusterSilhouette[k] /= AssignedPoints[k].count();

		SumSilhouette += ClusterSilhouette[k];
	}
	SumSilhouette /= K;
}

double ClusterAlgorithm::evaluate(ofstream& outfFile, bool complete, bool print){
	if( SumSilhouette == DBL_MAX ){
		computeSilhouette();
	}

	if( print ){
		std::stringstream SilhouetteMessage;
		printInfo(outfFile);
		printCenters(outfFile);
		outfFile << "clustering_time: " << execTime << '\n';
		SilhouetteMessage << "Silhouette: [";

		for(int k = 0; k < K; k++){
			SilhouetteMessage << ClusterSilhouette[k] << ", ";
		}

		SilhouetteMessage << SumSilhouette << "]";
		outfFile << SilhouetteMessage.str() << '\n';
		if( complete ){
			printClusters(outfFile);
		}

		outfFile << '\n' << '\n' << '\n' << '\n';
	}

	return SumSilhouette;
}

double ClusterAlgorithm::avgDistFromCluster( int i, int center ){
	int k = -1;
	// find the index of "center" int the "Centers" table
	for(k = 0; k < K; k++){
		if( Centers[k] == center ) break;
	}

	double sum = 0.0;
	// iterate over the aboce cluster, compute the sum
	for (Node<AssignPair>* inode = AssignedPoints[k].start() ; inode != NULL; inode = inode->next() ) {
		sum += (*d)( i, inode->data()->assigned() );
	}

	return sum / AssignedPoints[k].count();	// return mean value
}

ClusterAlgorithm::~ClusterAlgorithm(){
	delete[] Centers;
	delete[] OldCenters;
	delete[] AssignedPoints;
	delete[] ClusterSilhouette;

	delete Assign;
	delete Init;
	delete Update;
}

int ClusterAlgorithm::runCUTests(void){
	CU_pSuite pSuite = NULL;

	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("Cluster Suite", initSuite, finalSuite );
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if ( ( CU_add_test(pSuite, "dummyClustering", dummyClustering ) == NULL ) ) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}

void ClusterAlgorithm::dummyClustering(void){
	Point* DummyPointTable[5];
	string name = "A";
	for(int i = 0; i < 5; i++){
		if( i == 0 || i == 1 ){
			string dummy = "000000";
			dummy[i] = '1';
			DummyPointTable[i] = new HammingPoint( name , dummy);
		}
		else{
			string dummy = "101010";
			dummy[i] = '1';
			DummyPointTable[i] = new HammingPoint( name , dummy);
		}
		name += "A";
	}

	CU_ASSERT( DummyPointTable[4] == DummyPointTable[4] );
	CU_ASSERT( DummyPointTable[0]->dimension() == 6 );

	double dist = DummyPointTable[0]->distance( DummyPointTable[0] );
	CU_ASSERT( dist == 0.0 );

	TriangularMatrix dummyD( 5, DummyPointTable );
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			CU_ASSERT( dummyD(i,j) == dummyD(j,i) );
			if( i == j ){
				CU_ASSERT( dummyD(i,j) == 0.0 );
			}
		}
	}

	int DummyCenters[2];
	Park_Jun init( DummyCenters, &dummyD, 2, 5);
	init.initialise();
	CU_ASSERT( (DummyCenters[0] == 0 && DummyCenters[1] == 1 )
				|| (DummyCenters[0] == 1 && DummyCenters[1] == 0 )  );

	List<AssignPair> DummyAssignedPoints[2];
	PAM_Simple PAM( &dummyD, 5, DummyCenters, 2, DummyAssignedPoints );
	double J = PAM.assign();
	double Jold = J;
	CU_ASSERT( J > 0.0 );
	CU_ASSERT( DummyAssignedPoints[0].count() + DummyAssignedPoints[1].count() == 5 );
	CU_ASSERT( !DummyAssignedPoints[0].empty() && !DummyAssignedPoints[1].empty() );

	int DummyOldCenters[2];
	LloydsUpdate Update( &dummyD, &J, 2, DummyCenters, DummyOldCenters, DummyAssignedPoints );
	Update.update();
	CU_ASSERT( J > 0.0 );
	CU_ASSERT( !(J > Jold) );
	CU_ASSERT( DummyCenters[0] >= 0 && DummyCenters[0] < 5 );
	CU_ASSERT( DummyCenters[1] >= 0 && DummyCenters[1] < 5 );
	CU_ASSERT( DummyOldCenters[0] >= 0 && DummyOldCenters[0] < 5 );
	CU_ASSERT( DummyOldCenters[1] >= 0 && DummyOldCenters[1] < 5 );

	for(int i = 0; i < 5; i++){
		delete DummyPointTable[i];
	}
}

List<AssignPair>* ClusterAlgorithm::getCluster(int k){
	return &(AssignedPoints[k]);
}


ProteinsCluster::ProteinsCluster(Point** pointTable, TriangularMatrix* dPtr, int n, int k, int type, int k_hash, int l, int q, int s)
 : ClusterAlgorithm(pointTable, dPtr, n, k, type, k_hash, l, q, s){}


double ProteinsCluster::evaluate(std::ofstream& outfFile, bool complete, bool print){
	ClusterAlgorithm::evaluate(outfFile, complete, false);

	if( print ){
		outfFile << "k: " << K << endl;
		outfFile << "s: " << SumSilhouette << endl;
		for(int k = 0; k < K; k++){
			for (Node<AssignPair>* inode = AssignedPoints[k].end() ; inode != NULL; inode = inode->previous() ) {
				outfFile << PointTable[inode->data()->assigned()]->name() << " ";
			}
			outfFile << endl;
		}
	}
	return SumSilhouette;
}



CLARA::CLARA(Point** PointTable, int N, int s, int n_, int k){
	S = s;
	N_ = n_;
	K = k;

	message = "CLARA";

	Algorithms = new ClusterAlgorithm*[S];


	for(int s = 0; s < S; s++){					// "S" times

		Point** pointTable = new Point*[N_];	// create a random sub-table of points
		for(int n = 0; n < N_; n++){
			pointTable[n] = PointTable[ (int)Math::dRand(0, N - 1) ];
		}

		TriangularMatrix* D_ = new TriangularMatrix(N_, pointTable);	// compute their distances

		DeleteListPoint.insertAtStart( pointTable, true );				// keep references
		DeleteListMatrix.insertAtStart( D_, true );						// to be deleted

		// create an algorithm
		// the extra parameters are not needed, just give -1
		Algorithms[s] = new ClusterAlgorithm( pointTable, D_, N_, K, 4, -1, -1, -1, -1);
	}
}

void CLARA::run(void){
	double minJ = (double)DBL_MAX;
	for(int i = 0; i < S; i++){					// run every algorithm
		Algorithms[i]->run();
		double tempJ = Algorithms[i]->getJ();
		if( tempJ < minJ ){						// keep the one with the best performance
			minJ = tempJ;
			minIndex = i;
		}
	}

}

double CLARA::evaluate(std::ofstream& outfFile, bool complete, bool print){
	outfFile << "CLARA, ";
	return Algorithms[minIndex]->evaluate(outfFile, complete, print);
}

CLARA::~CLARA(){
	for(int i = 0; i < S; i++){
		delete Algorithms[i];
	}
	delete[] Algorithms;
}
