#ifndef __FOLD_VALIDATION__
#define  __FOLD_VALIDATION__

#include "../DataStructures/DoubleList.h"

#include <iostream>

using namespace std;


// helper class for defining a partition of a bigger table
class Partition{
	private:
		int* Indexes;		// indexes to the larger table
		int Size;			// the number of indexes

	public:
		Partition(Partition** Partitions, int PartitionsNum, int valPartition);
		Partition(int size, List<int>& indexList);
		Partition(List<int>& indexList);

		void print(int f){
			std::cout << "Partition " << f << std::endl;
			for(int i = 0; i < Size; i++){
				std::cout << "Indexes[" << i << "] = " << Indexes[i] << std::endl;
			}
			std::cout << std::endl << std::endl << std::endl;

		}

		int size(void);
		int operator [] (int i);

		~Partition();
};


// helper class for iterating over a partition
class Index{
	private:
		Partition* Part;					// the partition to iterate on
		int _Index;							// the current index in the partition
	public:

		Index(Partition* part, int i = 0){
			_Index = i;
			Part = part;
		}

		int index(void){
			return _Index;
		}

		bool operator < (int size){
			return _Index < size;
		}

		void operator ++ (int){
			_Index++;
		}

		int operator * (void){
			return (*Part)[_Index];
		}
};


#endif
