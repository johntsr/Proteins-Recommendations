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

unsigned int Bitset::distance(Bitset& b){
	int sum = 0;
	for(int i = 0; i < Size; i++){
		if( Array[i] != b.Array[i] ){
			sum++;
		}
	}
	return sum;
}

Bitset* Bitset::maxDistance(void){
	return new Bitset(Size, Size);
}

bool Bitset::nonZero(void){
	for(int i = 0; i < Size; i++){
		if( Array[i] ){
			return true;
		}
	}
	return false;
}

unsigned int Bitset::count(void){
	// unsigned int count = 0;
	// // std::cout << "Array[0] = " << Array[0] << std::endl;
	// for(int i = 0; i < Size; i++){
	// 	if( Array[i] ){
	// 		count++;
	// 	}
	// }
	// return count;
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
	set(0.0);
	Bits = NULL;
	toDelete = false;
}

Quantity::Quantity(double d){
	set(d);
	Bits = NULL;
	toDelete = false;
}

Quantity::Quantity(std::string bitString){
	set(0.0);
	Bits = new Bitset(bitString);
	toDelete = true;
}

Quantity::Quantity(Bitset* bits, bool del){
	set(0.0);
	set(bits);
	toDelete = del;
}

void Quantity::copy(Quantity* q){
	value = q->value;
	Bits = q->Bits;
	toDelete = false;
	isDouble = q->isDouble;
}

std::string Quantity::getString(void){
	std::ostringstream parseStr;
	parseStr << value;
	return parseStr.str();
}

double Quantity::getDouble(void){
	return value;
}

Bitset* Quantity::getBits(void){
	return Bits;
}

double Quantity::castAsDouble(void){
	if( isDouble ){
		return getDouble();
	}
	else{
		return Bits->count() * 1.0;
	}
}


void Quantity::set(double d){
	value = d;
	isDouble = true;
}

void Quantity::set(Bitset* bits){
	Bits = bits;
	isDouble = false;
}

void Quantity::multiply(int times){
	value *= times * 1.0;
}


std::string QuantitySquare::getString	(void){
	std::ostringstream parseStr;
	parseStr << sqrt( getDouble() );
	return parseStr.str();
}

void QuantitySquare::multiply(int times){
	value *= times * times * 1.0;
}


std::string QuantityBit::getString	(void){
	std::ostringstream parseStr;
	parseStr << getBits()->getString();
	return parseStr.str();
}

void QuantityBit::multiply(int times){
	Bits->setFirst( Bits->count() * times );
}
