#include "Quantity.h"
#include "Math.h"
#include <sstream>
#include <iostream>

using namespace std;

/*****************************************************************
***************** Bitset class methods **************************
******************************************************************/

Bitset::Bitset(string bitString){
	Size = bitString.length();
	Array = new bool[Size];
	Count = 0;

	for(int i = 0; i < Size; i++){
		Array[i] = (bool)(bitString[i] - '0');
		if( Array[i] ){
			Count++;
		}
	}
}

Bitset::Bitset(int size, int count){
	Size = size;
	Array = new bool[Size];
	Count = 0;
	setFirst(count);
}

Bitset::~Bitset(){
	delete[] Array;
}

int Bitset::size(void){
	return Size;
}

Bitset* Bitset::operator & (Bitset& b){
	Bitset* ResultBits = new Bitset(Size);
	for(int i = 0; i < Size; i++){
		if( Array[i] & b.Array[i] ){
			ResultBits->set(i);
		}
	}
	return ResultBits;
}

Bitset* Bitset::operator ^ (Bitset& b){
	Bitset* ResultBits = new Bitset(Size);
	for(int i = 0; i < Size; i++){
		if( Array[i] ^ b.Array[i] ){
			ResultBits->set(i);
		}

	}
	return ResultBits;
}

unsigned int Bitset::count(void){
	return Count;
}

void Bitset::setFirst(int count){
	for(int i = 0; i < Size; i++){
		if( i < count ){
			Array[i] = true;
			Count++;
		}
		else{
			Array[i] = false;
		}
	}
}

void Bitset::set(int position){
	if( !Array[position] ){
		Count++;
	}
	Array[position] = true;
}

string Bitset::getString(void){
	string result;
	for(int i = 0; i < Size; i++){
		if( Array[i] ){
			result += "1";
		}
		else{
			result += "0";
		}
	}
	return result;
}

/*****************************************************************
***************** Quantity class methods **************************
******************************************************************/

Quantity::Quantity(void){
	setDouble(0.0);
	Bits = NULL;
	toDelete = false;
}

Quantity::Quantity(double d){
	setDouble(d);
	Bits = NULL;
	toDelete = false;
}

Quantity::Quantity(std::string bitString){
	setDouble(0.0);
	setBits(new Bitset(bitString));
	toDelete = true;
}

Quantity::Quantity(Bitset* bits, bool del){
	setDouble(0.0);
	setBits(bits);
	toDelete = del;
}

double Quantity::getDouble(void){
	return value;
}

Bitset* Quantity::getBits(void){
	return Bits;
}

void Quantity::setDouble(double d){
	value = d;
}

void Quantity::setBits(Bitset* bits){
	Bits = bits;
}
