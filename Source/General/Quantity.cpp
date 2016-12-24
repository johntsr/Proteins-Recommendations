#include "Quantity.h"
#include "Math.h"
#include <sstream>
#include <iostream>

using namespace std;

/*****************************************************************
***************** Bitset class methods **************************
******************************************************************/

Bitset::Bitset(string bitString){
	Dimension = bitString.length();
	Size = Dimension / 64;
	if( Dimension % 64 > 0 ){
		Size++;
	}

	Array = new bitset<64>[Size];

	Count = 0;
	for(int b = 0, strIndex = 0; b < Size; b++){
		for(int i = 0; i < 64 && strIndex < Dimension; i++, strIndex++ ){
			Array[b][i] = (bool)(bitString[strIndex] - '0');
			if( Array[b][i] ){
				Count++;
			}
		}
	}
}

Bitset::Bitset(int size, int count){
	Dimension = size;
	Size = Dimension / 64;
	if( Dimension % 64 > 0 ){
		Size++;
	}
	Array = new bitset<64>[Size];
	Count = 0;
	setFirst(count);
}

Bitset::~Bitset(){
	delete[] Array;
}

int Bitset::dimension(void){
	return Dimension;
}

Bitset* Bitset::operator & (Bitset& bset){
	Bitset* ResultBits = new Bitset(Dimension);
	int count = 0;
	for(int b = 0; b < Size; b++){
		ResultBits->Array[b] = Array[b] & bset.Array[b];
		count += ResultBits->Array[b].count();
	}
	ResultBits->Count = count;
	return ResultBits;
}

Bitset* Bitset::operator ^ (Bitset& bset){
	Bitset* ResultBits = new Bitset(Dimension);
	int count = 0;
	for(int b = 0; b < Size; b++){
		ResultBits->Array[b] = Array[b] ^ bset.Array[b];
		count += ResultBits->Array[b].count();
	}
	ResultBits->Count = count;
	return ResultBits;
}

unsigned int Bitset::count(void){
	return Count;
}

void Bitset::setFirst(int count){
	int allSet = count / 64;
	int remain = count % 64;

	for(int b = 0; b < allSet; b++){
		Array[b].set();
	}

	int b = allSet;
	for(int i = 0, strIndex = 0 ; i < 64 && strIndex < remain; i++, strIndex++ ){
		Array[b][i] = true;
	}

	Count = count;
}

void Bitset::set(int position){
	int b = position / 64;
	int i = position % 64;
	if( !Array[b][i] ){
		Count++;
	}

	Array[b][i] = true;
}

string Bitset::getString(void){
	string result;

	for(int b = 0, strIndex = 0; b < Size; b++){
		for(int i = 0; i < 64; i++, strIndex++ ){
			if( Array[b][i] ){
				result += "1";
			}
			else{
				result += "0";
			}

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
