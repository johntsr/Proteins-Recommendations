#ifndef __FOLD_VALIDATION__
#define  __FOLD_VALIDATION__

#include "../DataStructures/DoubleList.h"

#include <iostream>

using namespace std;

class Partition{
	private:
		int* Indexes;
		int Size;

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


#endif
