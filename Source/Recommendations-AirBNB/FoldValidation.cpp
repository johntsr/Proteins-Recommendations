#include "FoldValidation.h"

// create a partition out of a list containing the indexes
Partition::Partition(int size, List<int>& indexList){
	Size = size;
	Indexes = new int[Size];
	for(int i = 0; i < Size; i++){
		int* temp = indexList.removeAtStart();	// just pop form the list
		Indexes[i] = *temp;
		delete temp;
	}
}

// partition out of a set of partitions
// where the "valPartition" partition will be used as validation
// so, skip it
Partition::Partition(Partition** Partitions, int PartitionsNum, int valPartition){

	// first, calculate the total size
	Size = 0;
	for(int f = 0; f < PartitionsNum; f++){
		if( f != valPartition){
			Size += Partitions[f]->size();
		}
	}

	// then, fill in the indexes
	Indexes = new int[Size];
	for(int f = 0, i = 0; f < PartitionsNum; f++){
		if( f != valPartition){
			for(int j = 0; j < Partitions[f]->size(); j++, i++){
				Indexes[i] = (*Partitions[f])[j];
			}
		}
	}
}


int Partition::size(void){
	return Size;
}

int Partition:: operator [] (int i){
	return Indexes[i];
}

Partition::~Partition(){
	delete[] Indexes;
}
