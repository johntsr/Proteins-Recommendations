#ifndef __LOCALITY_HASHING__
#define __LOCALITY_HASHING__

#include "HashTablePoint.h"
#include "HashTableEuclidean.h"

template<class T, class U>
class LocalHashTable{
	private:
		PointHashTable<T,U>** HashTables;			// table of Hash Tables used to access the Points
		int L;										// the number of the above Hash Tables
		List<T,U>		 PointList;					// list used for "scanning" the Points in search of the true nearest neighbour
		List<T,U>		 MarkedPoints;				// list that contains the points assigned to a cluster

		// number of changes in the current best point in the pseudocode of NN
		int Barrier;
	public:
		LocalHashTable(int barrier, int l, int tableSize, hash_function** hashFunctions, bool isEuclidean ){
			Barrier = barrier;
			L = l;									// keep the nubmer of hash tables
			HashTables = new PointHashTable<T,U>*[L];	// allocate the table

			for( int i = 0; i < L; i++){			// for every slot in the table
				if( isEuclidean ){					// allocate the appropriate bucket
					HashTables[i] = new EuclideanHashTable<T,U>( tableSize, hashFunctions[i] );
				}
				else{
					HashTables[i] = new PointHashTable<T,U>( tableSize, hashFunctions[i] );
				}
			}
		}

		void insert(T* point){							// insert a point in the structure


			PointList.insertAtStart(point, false);		// insert it in the list, mark it to be deleted when the program terminates

			for( int i = 0; i < L; i++){
				HashTables[i]->insert( point );	// also insert it in the Hash Tables, mark it no to be deleted!
			}

		}

		void remove(T* point){							// insert a point in the structure
			PointList.removeFirst(point);		// insert it in the list, mark it to be deleted when the program terminates

			for( int i = 0; i < L; i++){
				HashTables[i]->remove( point );	// also insert it in the Hash Tables, mark it no to be deleted!
			}

		}

		void inRange( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range
			for( int i = 0; i < L; i++){							// search all Hash Tables
				HashTables[i]->inRange( point, R, ResultPoints );	// keep all the unique neighbours I find
			}
		}

		void inRangeBarrier( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range
			for( int i = 0; i < L; i++){							// search all Hash Tables
				HashTables[i]->inRangeBarrier( point, R, ResultPoints );	// keep all the unique neighbours I find
			}

			// keep the points that have been assigned somewhere
			// later, will have to skip them form the queries
			for (Node<T>* node = ResultPoints.start() ; node != NULL; node = node->next() ) {
				if( !MarkedPoints.contains(node->data()) ){				// unique insert
					MarkedPoints.insertAtStart( node->data(), false );
				}
			}

		}

		void nearestNeighbour( U point, double& minDist, U& bestPoint ){
			// "bestPoint": the point closest to "point"
			// "minDist" : the distance of "bestPoint" from "point"

			minDist = point->maxDistance();
			bestPoint = NULL;

			int timesChanged = 0;
			for( int i = 0; i < L; i++){										// search all Hash Tables
				HashTables[i]->nearestNeighbour( point, minDist, bestPoint, timesChanged, Barrier );	// keep the nearest neighbour between them all
			}

		}

		void nearestNeighbourBrute( U point, double& minDist, U& bestPoint ){
			// "bestPoint": the point closest to "point"
			// "minDist" : the distance of "bestPoint" from "point"
			minDist = point->maxDistance();
			bestPoint = NULL;

			Node<T>* node = PointList.start();
			for( ; node != NULL; node = node->next() ){							// iterate over the list of Points

				if( point->inRange( node->data(), minDist ) ){					// if a point is closer than "bestPoint"
					delete minDist;												// delete the previous value
					minDist = point->distance( node->data() );					// update the "bestPoint"
					bestPoint = node->data();									// and its corresponding distance
				}
			}

		}

		void print(void){
			for( int i = 0; i < L; i++){
				HashTables[i]->print();
			}
		}

		int count(){
			if( HashTables[0] != NULL )
				return HashTables[0]->count();	// all the hash tables have the same amount of points
			return -1;
		}

		void prepareBarrier(T* barrier){
			for( int i = 0; i < L; i++){
				HashTables[i]->prepareBarrier(barrier);
			}
		}

		void markPoints(void){
			// for every assigned point
			// mark it to be skipped from queries for every ahsh table
			for (Node<T>* node = MarkedPoints.start() ; node != NULL; node = node->next() ) {
				for( int i = 0; i < L; i++){
					HashTables[i]->markPoint( node->data() );
				}
			}
		}

		void unmarkPoints(void){
			for( int i = 0; i < L; i++){
				HashTables[i]->resetBarrier();
			}

			MarkedPoints.flush();
		}


		~LocalHashTable(){
			for( int i = 0; i < L; i++){
				delete HashTables[i];
			}
			delete[] HashTables;
		}

};


#endif
