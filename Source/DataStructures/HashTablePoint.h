#ifndef __STATIC_HASH__
#define __STATIC_HASH__
#include "DoubleList.h"
#include "../General/HashFunction.h"
#include "../General/Math.h"
#include <iostream>

template<class T, class U>
class PointBucketStructure{								// class representing the bucket of a hashtable
	protected:
		List<T,U>*	BucketList;							// simply a list

		T* Barrier;

		template<class U1, class U2>					// all methods are protected
		friend class PointHashTable;					// so, allow the hash table to call them

		PointBucketStructure(void){						// no-argument constructor
			BucketList = new List<T,U>();				// just initialise the list
		}

		void add(T* value){								// add a point to the bucket
			// false: flag indicating NOT to delete the point when the program terminates
			BucketList->insertAtStart( value , false);	// insert it at the start of the list
		}

		void inRange( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			for (Node<T>* node = BucketList->start() ; node != NULL; node = node->next() ) {		// iterate over the bucket
				if( point->inRange( node->data(), R ) && !ResultPoints.contains(node->data()) ){	// if a point is in range
					ResultPoints.insertAtStart( node->data(), false );								// add it ONCE to the results
				}
			}
		}

		virtual void prepareBarrier(T* barrier){
			// insert barrier at the end of the list
			Barrier = barrier;
			BucketList->insertAtEnd( Barrier, false );
		}

		virtual void resetBarrier(void){
			// find the barrier, put it back in the end again
			Node<T>* findBarrier = BucketList->findFirst(Barrier);
			T* data = BucketList->remove( findBarrier );
			BucketList->insertAtEnd(data, false);
		}

		virtual void markPoint(T* point){
			// find the "point", place it after barrier
			Node<T>* findPoint = BucketList->findFirst(point);
			T* data = BucketList->remove( findPoint );
			BucketList->insertAtEnd(data, false);
		}

		virtual void inRangeBarrier( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			for (Node<T>* node = BucketList->start() ; !( Barrier == node->data() ); ) {			// iterate over the bucket
				if( point->inRange( node->data(), R ) && !ResultPoints.contains(node->data()) ){	// if a point is in range
					ResultPoints.insertAtStart( node->data(), false );								// add it ONCE to the results

					// remove() forwards the iterator one position!
					T* data = BucketList->remove( node );
					BucketList->insertAtEnd(data, false);
				}
				else{
					 node = node->next();
				}
			}
		}

		void nearestNeighbour( U point, double& minDist, U& bestPoint, int& timesChanged, int barrier ){
			// "point"  	: the Point whose nearest neighbour I am searching
			// "bestPoint"	: the point closest to "point" so far
			// "minDist" 	: the distance of "bestPoint" from "point"

			for (Node<T>* node = BucketList->start() ; node != NULL; node = node->next() ) {		// iterate over the bucket
				if( point->inRange( node->data(), minDist ) ){				// if a point is closer than "bestPoint"
					minDist = point->distance( node->data() );				// update the "bestPoint"
					bestPoint = node->data();								// and its corresponding distance
					timesChanged++;											// one more change happened
					if( timesChanged == barrier)							// if too many changes took place
						return;												// current solution is OK
				}
			}
		}

		void print(void){
			for (Node<T>* node = BucketList->start() ; node != NULL; node = node->next() ) {
				std::cout << "\t" << node->data()->name() ;
			}
			std::cout << std::endl;
		}

		int size(){ return BucketList->count(); }

		virtual ~PointBucketStructure(void){
			delete BucketList;
		}

};

template<class T, class U>
class PointHashTable{									// class representing a hash table of points
	protected:
		PointBucketStructure<T,U>** BucketTable;	// a table of buckets
		int BucketsNum;								// the number of the above buckets
		int PointCount;								// how many Points I have inserted
		hash_function* H_function;					// hash functions used to insert a Point into the proper bucket

		PointHashTable(void){							// no-argument constructor, only used by the Euclidean Hash Table
			BucketTable = NULL;						// it uses different structures for the bucket, so this table will be empty
		}

	public:

		PointHashTable(int bucketsNum, hash_function* h_function) {
			PointCount = 0;										// initiallly, no Points are in the hash table
			BucketsNum = bucketsNum;							// keep the nubmer of buckets
			H_function = h_function;							// as well as the hash function
			BucketTable = new PointBucketStructure<T,U>*[BucketsNum];// allocate the table of buckets

			for(int i = 0 ; i < BucketsNum ; i++){				// and, for each slot in the table
				BucketTable[i] = new PointBucketStructure<T,U>();	// allocate one bucket
			}

		}

		virtual void insert(T* value){							// insert a Point in the Hash Table
			uint64_t position = H_function->hash( value );		// hash the point, keep the index that is returned
			BucketTable[position]->add( value );				// store the point in the appropriate bucket
			PointCount++;										// another point is added
		}

		virtual void print(void){
			for(int i = 0; i < BucketsNum; i++){
				std::cout << "Bucket " << i << " : " << BucketTable[i]->size() << std::endl;
				//BucketTable[i]->print();
			}
		}

		virtual void inRange( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			uint64_t position = H_function->hash( point );				// hash the point
			BucketTable[position]->inRange( point, R, ResultPoints );	// search in the appropriate bucket for possible neighbours
		}

		virtual void inRangeBarrier( U point, double R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			uint64_t position = H_function->hash( point );				// hash the point
			BucketTable[position]->inRangeBarrier( point, R, ResultPoints );	// search in the appropriate bucket for possible neighbours
		}

		virtual void nearestNeighbour( U point, double& minDist, U& bestPoint, int& timesChanged,  int barrier){
			// "bestPoint": the point closest to "point"
			// "minDist" : the distance of "bestPoint" from "point"
			uint64_t position = H_function->hash( point );				// hash the point
			// search in the appropriate bucket the possible nearest neighbour
			BucketTable[position]->nearestNeighbour( point, minDist, bestPoint, timesChanged, barrier);
		}

		int count(){ return PointCount; }

		int size(){ return BucketsNum; }

		virtual void prepareBarrier(T* barrier){
			for(int i = 0; i < BucketsNum; i++){
				BucketTable[i]->prepareBarrier(barrier);
			}
		}

		virtual void resetBarrier(void){
			for(int i = 0; i < BucketsNum; i++){
				BucketTable[i]->resetBarrier();
			}
		}

		virtual void markPoint(T* point){
			uint64_t position = H_function->hash( point );				// hash the point
			// search in the appropriate bucket the possible nearest neighbour
			BucketTable[position]->markPoint( point );

		}

		virtual ~PointHashTable(void){

			if( BucketTable != NULL ){
				for(int i = 0 ; i < BucketsNum ; i++)
					delete BucketTable[i];
				delete[] BucketTable;
			}
		}
};

#endif
