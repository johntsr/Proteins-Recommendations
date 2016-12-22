#ifndef __MATH__
#define __MATH__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

enum Direction{ UP, DOWN };

class Math{													// class used for it's static functions
	public:
		static double dRand(){								// returns numbers in [0,1)
			return ( rand() / ( RAND_MAX + 1.0 ) );
		}
		//
		// static double dRand(int M, int N){					// returns numbers in [M,N]
		// 	return M + ( rand() / ( RAND_MAX + 1.0 ) ) * (N - M );
		// }

		static double dRand(double M, double N){					// returns numbers in [M,N)
			return M + ( rand() / ( RAND_MAX + 1.0 ) ) * (N - M );
		}

		// a double and a pointer are stored in 64 bits
		// we use this to our advantage: we store a value in these 64 bits
		// and we later interpret the contents of these bits, read them either as integers or doubles or addresses

		static double int64ToDouble(uint64_t u){			// store the contents of 64 bits in a double
			double temp;									// a temporary double value to be returned
			memcpy( (void*)&temp, (void*)&u, sizeof(u) );	// just copy the contents of the 64 bits to "temp"
			return temp;
		}

		static uint64_t doubleToint64(double d ){			// store the contents of a double in 64 bits
			uint64_t temp;									// a temporary value to be returned
			memcpy( (void*)&temp, (void*)&d, sizeof(d) );	// just copy the contents of the double bits to "temp"
			return temp;
		}

		static double getGaussian(void){
			// the "Marsaglia" method is used to return a random variable in N(0,1)
			double y1;
			double y2;
			double r;

			do{
				y1 = Math::dRand(-1,1);
				y2 = Math::dRand(-1,1);
				r = y1*y1 + y2*y2;
			}while( !( r < 1 ) );

			return y2 * sqrt( -2.0 * log(r) / r );
		}


		static double selection( double* Table, int n, int select){
			return selection_body( Table, 0, n-1, select );
		}

		static double selection_body(double* Table, int p, int r, int i ){
			if( p == r ){
				return Table[p];
			}
			int q = partition(Table, p, r);
			int k = q - p + 1;
			if( i == k ){
			   return Table[q];
			}
			else if( i < k){
			   return selection_body(Table, p, q - 1, i);
			}
			else{
			   return selection_body(Table, q + 1, r, i - k);
			}
		}

		static int partition( double* Table, int p, int r) {
		   double x;
		   int j, i, temp;

			temp = rand()%(r - p + 1) + p;
			swap( &Table[temp], &Table[r] );

			x = Table[r];
			i = p - 1;
			j = p;

			for( j = p; j < r; j++){
				if( Table[j] < x ){
					i++;
					swap( &Table[i], &Table[j]);
				}
			}
		   swap(&Table[i+1], &Table[r]);
		   return i + 1;
		}

		static void swap(double* a, double* b){
			double temp = *a;
			*a = *b;
			*b = temp;
		}







	template<typename T>
	static void sort( double* keys, T* others , int n){
		quicksort_body( keys, others, 0, n-1);
	}

	template<typename T>
	static void quicksort_body(double* keys, T* others, int p, int r){
		int q;
		if( p < r ){
	   	// divide and conquer
			q = partition ( keys, others, p, r);
			quicksort_body( keys, others, p, q-1);
			quicksort_body( keys, others, q+1, r);
	   }
	}

	template<typename T>
	static int partition( double* keys, T* others, int p, int r) {
	   double x;
	   int j, i, temp;

		temp = rand()%(r - p + 1) + p ;
		swap( &keys[temp], &keys[r] );
		if( others != NULL ){
			swap( &others[temp], &others[r] );
		}

		x = keys[r];
		i = p-1;
		j = p;

		for( j = p; j < r; j++){
			if( keys[j] < x ){
				i++;
				swap( &keys[i], &keys[j]);
				if( others != NULL ){
					swap( &others[i], &others[j]);
				}
			}
		}
	   swap(&keys[i+1], &keys[r]);
	   if( others != NULL ){
		   swap( &others[i+1], &others[r]);
	   }

	   return i + 1;
	}

	template<typename T>
	static void swap(T* a, T* b){
		T temp=*a;
		*a=*b;
		*b=temp;
	}



	static int binarySearch(double* D, int Length, double key){
		int low=0, high=Length-1, mid = 0;

		Direction dir = UP;
		while ( low <= high ){
			mid=(low+high)/2;
			if ( key < D[mid] ){
				dir = DOWN;
				high=mid-1;
			}
			else if ( key > D[mid] ){
				dir = UP;
				low=mid+1;
			}
			else{
				return mid;
			}
		}

		if( dir == UP ){
			return mid + 1;
		}
		else{
			return mid;
		}
	}

};

#endif
