#include "Quantity.h"
#include "Math.h"
#include <sstream>

/*****************************************************************
***************** Quantity class methods **************************
******************************************************************/

Quantity::Quantity(void){
	set(0.0);
}

Quantity::Quantity(double d){
	set(d);
}

Quantity::Quantity(uint64_t u){
	set(u);
}

std::string Quantity::getString(void){
	std::ostringstream parseStr;
	parseStr << value;
	return parseStr.str();
}

double Quantity::getDouble(void){
	return value;
}

uint64_t Quantity::getBits(void){
	return Math::doubleToint64(value);
}

double Quantity::castAsDouble(void){
	if( isDouble ){
		return getDouble();
	}
	else{
		return Math::doubleToint64(value) * 1.0 ;
	}
}


void Quantity::set(double d){
	value = d;
	isDouble = true;
}

void Quantity::set(uint64_t u){
	value = Math::int64ToDouble(u);
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
	parseStr << getBits();
	return parseStr.str();
}

void QuantityBit::multiply(int times){
	uint64_t temp = getBits();
	temp *= times;
	set(temp);
}
