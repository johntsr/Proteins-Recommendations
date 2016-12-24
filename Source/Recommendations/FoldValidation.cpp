#include "FoldValidation.h"

Partition::Partition(int size, List<int>& indexList){
	Size = size;
	Indexes = new int[Size];
	for(int i = 0; i < Size; i++){
		int* temp = indexList.removeAtStart();
		Indexes[i] = *temp;
		delete temp;
	}
}

Partition::Partition(Partition** Partitions, int PartitionsNum, int valPartition){
	Size = 0;
	for(int f = 0; f < PartitionsNum; f++){
		if( f != valPartition){
			Size += Partitions[f]->size();
		}
	}

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
