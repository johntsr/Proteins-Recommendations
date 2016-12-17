#ifndef __STATIC_EUCLIDEAN_HASH__
#define __STATIC_EUCLIDEAN_HASH__
#include "DoubleList.h"
#include "../General/HashFunction.h"
#include "HashTablePoint.h"
#include "../General/Math.h"
#include <iostream>

template<class T, class U>
class BucketStructureEuclidean: public PointBucketStructure<T,U>{		// bucket used for Euclidean Points
	private:
		List< uint64_t, uint64_t >* IDList;							// just the point is not enough, its ID is also needed

		template<class U1, class U2>								// all methods are protected
		friend class EuclideanHashTable;							// so, allow the hash table to call them

		BucketStructureEuclidean(void) : PointBucketStructure<T,U>() {	// no-argument constructor
			IDList = new List<uint64_t, uint64_t>();					// just initialise the list
		}

		void add(T* value, uint64_t ID){							// add a <Point-ID> pair in the bucket
			PointBucketStructure<T,U>::add(value);						// first, add the point
			IDList->insertAtStart( new uint64_t(ID), true );		// then, store the ID in the same position
			// true: flag indicating to delete the point when the program terminates
		}

		void inRange( U point, Quantity* R, List<T,U>& ResultPoints, uint64_t ID ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range
			// "ID"				: the ID of the above "point" computed from the hash function

			Node<T>* nodeBucket 	= PointBucketStructure<T,U>::BucketList->start();		// iterator for the list of points
			Node<uint64_t>* nodeID 	= IDList->start();									// iterator for the list of IDs

			double Rd = R->getDouble();				// the range I am interested in
			double Rd2 = Rd *Rd;					// for speed's sake, Euclidean Points don't compute their actual distances
			Quantity R2( Rd2 );						// they compute their squares, so I have to convert it exxplicitely

			for( ; nodeBucket != NULL; nodeBucket = nodeBucket->next(), nodeID = nodeID->next() ) {		// iterate over the bucket
				if( 		*(nodeID->data()) == ID									// if 2 Points have the same ID
						&& point->inRange( nodeBucket->data(), &R2 )				// and they are neighbours
						&& !ResultPoints.contains(nodeBucket->data()) ){			// and the neighbour is found for the first time
					ResultPoints.insertAtStart( nodeBucket->data(), false );		// add the neighbour to the results
				}
			}

		}

		void prepareBarrier(T* barrier){
			// insert barrier at the end of the point-list
			// also, insert a "special" id the end of the id-list
			PointBucketStructure<T,U>::Barrier = barrier;
			PointBucketStructure<T,U>::BucketList->insertAtEnd( PointBucketStructure<T,U>::Barrier, false );
			IDList->insertAtEnd( new uint64_t(-1), true );
		}

		void resetBarrier(void){
			// find the barrier, put it back in the end again
			Node<T>* findPoint = PointBucketStructure<T,U>::BucketList->findFirst(PointBucketStructure<T,U>::Barrier);
			T* data = PointBucketStructure<T,U>::BucketList->remove(findPoint);
			PointBucketStructure<T,U>::BucketList->insertAtEnd(data, false);

			// the same for the "special" id
			Node<uint64_t>* findID = IDList->findFirst(-1);
			uint64_t* id = IDList->remove( findID );
			IDList->insertAtEnd(id, true);
		}

		void markPoint(T* point){

			// find the "point", place it after barrier
			// have to search the point-list manually,
			// because the "special" id also has to be moved!

			Node<T>* nodeBucket 	= PointBucketStructure<T,U>::BucketList->start();		// iterator for the list of points
			Node<uint64_t>* nodeID 	= IDList->start();									// iterator for the list of IDs


			for( ; nodeBucket != NULL; nodeBucket = nodeBucket->next(), nodeID = nodeID->next() ) {
				if( nodeBucket->data() == point ){

					// remove() forwards the iterator one position!
					T* data = PointBucketStructure<T,U>::BucketList->remove( nodeBucket );
					PointBucketStructure<T,U>::BucketList->insertAtEnd(data, false);

					uint64_t* id = IDList->remove( nodeID );
					IDList->insertAtEnd(id, true);
					break;
				}
			}

		}

		void inRangeBarrier( U point, Quantity* R, List<T,U>& ResultPoints, uint64_t ID ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range
			// "ID"				: the ID of the above "point" computed from the hash function

			Node<T>* nodeBucket 	= PointBucketStructure<T,U>::BucketList->start();		// iterator for the list of points
			Node<uint64_t>* nodeID 	= IDList->start();									// iterator for the list of IDs

			double Rd = R->getDouble();				// the range I am interested in
			double Rd2 = Rd *Rd;					// for speed's sake, Euclidean Points don't compute their actual distances
			Quantity R2( Rd2 );						// they compute their squares, so I have to convert it exxplicitely


			for( ; !( PointBucketStructure<T,U>::Barrier == nodeBucket->data() );
			 ) {			// iterate over the bucket

				if( 		*(nodeID->data()) == ID									// if 2 Points have the same ID
						&& point->inRange( nodeBucket->data(), &R2 )				// and they are neighbours
						&& !ResultPoints.contains(nodeBucket->data()) ){			// and the neighbour is found for the first time
					ResultPoints.insertAtStart( nodeBucket->data(), false );		// add the neighbour to the results

					T* data = PointBucketStructure<T,U>::BucketList->remove( nodeBucket );
					PointBucketStructure<T,U>::BucketList->insertAtEnd(data, false);

					uint64_t* id = IDList->remove( nodeID );
					IDList->insertAtEnd(id, true);
				}
				else{
					nodeBucket = nodeBucket->next();
					nodeID = nodeID->next();
				}
			}

		}

		void nearestNeighbour( U point, Quantity*& minDist, U& bestPoint, uint64_t ID, int& timesChanged, int barrier ){
			// "point"  	: the Point whose nearest neighbour I am searching
			// "bestPoint"	: the point closest to "point" so far
			// "minDist" 	: the distance of "bestPoint" from "point"
			// "ID"				: the ID of the above "point" computed from the hash function

			Node<T>* nodeBucket 	= PointBucketStructure<T,U>::BucketList->start();	// iterator for the list of points
			Node<uint64_t>* nodeID 	= IDList->start();								// iterator for the list of IDs

			//std::cout << "minDist before: " << minDist->getString() << std::endl;
			for( ; nodeBucket != NULL; nodeBucket = nodeBucket->next(), nodeID = nodeID->next() ) {	// iterate over the bucket
				if( 	*(nodeID->data()) == ID										// if 2 Points have the same ID
					&& 	point->inRange( nodeBucket->data(), minDist ) ){			// and point is closer than "bestPoint"

						delete minDist;												// delete the previous value
						minDist =  point->distance( nodeBucket->data() );			// update the "bestPoint"
						bestPoint = nodeBucket->data();								// and its corresponding distance
						timesChanged++;												// one more change happened
						if( timesChanged == barrier)								// if too many changes took place
							return;													// current solution is OK
				}
			}
		}

		~BucketStructureEuclidean(void){
			delete IDList;
		}

};

template<class T, class U>
class EuclideanHashTable: public PointHashTable<T,U>{						// class representing a hash table of Euclidean Points
	private:
		BucketStructureEuclidean<T,U>** BucketTableEuclidean;			// a table of buckets
	public:

		EuclideanHashTable(int bucketsNum, hash_function* h_function){
			// don't call the super constructor, he uses different buckets!
			PointHashTable<T,U>::PointCount = 0;											// initiallly, no Points are in the hash table
			PointHashTable<T,U>::BucketsNum = bucketsNum;								// keep the nubmer of buckets
			PointHashTable<T,U>::H_function = h_function;								// as well as the hash function
			BucketTableEuclidean = new BucketStructureEuclidean<T,U>*[bucketsNum];	// allocate the table of buckets

			for(int i = 0 ; i < PointHashTable<T,U>::BucketsNum ; i++){					// and, for each slot in the table
				BucketTableEuclidean[i] = new BucketStructureEuclidean<T,U>();		// allocate one bucket
			}
		}

		void insert(T* value){														// insert an Euclidean Point in the Hash Table
			uint64_t position = PointHashTable<T,U>::H_function->hash( value );			// hash the point, keep the ID that is returned
			BucketTableEuclidean[ position % PointHashTable<T,U>::BucketsNum ]->add( value, position ); // insert the <Point,ID> pair in the bucket
			PointHashTable<T,U>::PointCount++;											// another point is added
		}

		void inRange( U point, Quantity* R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			uint64_t position = PointHashTable<T,U>::H_function->hash( point );			// hash the point, keep the ID that is returned
			// search in the appropriate bucket for possible neighbours
			BucketTableEuclidean[ position % PointHashTable<T,U>::BucketsNum ]->inRange( point, R, ResultPoints, position );
		}

		void inRangeBarrier( U point, Quantity* R, List<T,U>& ResultPoints ){
			// "point"  		: the Point whose neighbours I am searching
			// "R"				: the range I am interested in
			// "ResultPoints"	: list of Points I discover in the above range

			uint64_t position = PointHashTable<T,U>::H_function->hash( point );			// hash the point, keep the ID that is returned
			// search in the appropriate bucket for possible neighbours
			BucketTableEuclidean[ position % PointHashTable<T,U>::BucketsNum ]->inRangeBarrier( point, R, ResultPoints, position );
		}

		void nearestNeighbour( U point, Quantity*& minDist, U& bestPoint, int& timesChanged, int barrier ){
			// "bestPoint": the point closest to "point"
			// "minDist" : the distance of "bestPoint" from "point"

			uint64_t position = PointHashTable<T,U>::H_function->hash( point );			// hash the point, keep the ID that is returned
			// search in the appropriate bucket the possible nearest neighbour
			BucketTableEuclidean[ position % PointHashTable<T,U>::BucketsNum ]->nearestNeighbour( point, minDist, bestPoint, position, timesChanged, barrier );
		}

		void prepareBarrier(T* barrier){
			for(int i = 0; i < PointHashTable<T,U>::BucketsNum; i++){
				BucketTableEuclidean[i]->prepareBarrier(barrier);
			}
		}

		void resetBarrier(void){
			for(int i = 0; i < PointHashTable<T,U>::BucketsNum; i++){
				BucketTableEuclidean[i]->resetBarrier();
			}
		}

		void markPoint(T* point){
			uint64_t position = PointHashTable<T,U>::H_function->hash( point );				// hash the point
			// search in the appropriate bucket the possible nearest neighbour
			BucketTableEuclidean[position % PointHashTable<T,U>::BucketsNum]->markPoint( point );

		}

		void print(void){
			for(int i = 0; i < PointHashTable<T,U>::BucketsNum; i++){
				std::cout << "Bucket " << i << " : " << BucketTableEuclidean[i]->size() << std::endl;
				//BucketTable[i]->print();
			}
		}

		virtual ~EuclideanHashTable(void){

			if( BucketTableEuclidean != NULL ){
				for(int i = 0 ; i < PointHashTable<T,U>::BucketsNum ; i++)
					delete BucketTableEuclidean[i];
				delete[] BucketTableEuclidean;
			}
		}

};

#endif
