#ifndef __QUANTITY__
#define __QUANTITY__

#include <string>
#include <stdint.h>

// class used for exchanging information between various classes and methods
class Quantity{
	protected:
		// this information can always be stored in 64 bits
		// that's because either "double" or "uint64_t" is exhanged
		// since only 1 class (Hamming) needs "uint64_t"
		// we use "double" for speed's sake
		double value;
		bool isDouble;

	public:

		Quantity(void);							// no argument constructor
		Quantity(double d);						// store a "double"
		Quantity(uint64_t u);					// store a "uint64_t"

		virtual std::string getString	(void);	// string representation of the content
		virtual double 		getDouble	(void);	// parse the stored info as "double"
		virtual uint64_t 	getBits		(void);	// parse the stored info as "uint64_t"

		virtual double castAsDouble		(void);

		virtual void set(double d);				// store a "double"
		virtual void set(uint64_t u);			// store a "uint64_t"

		virtual void multiply(int times);

		virtual ~Quantity(){}
};


class QuantitySquare: public Quantity {
	public:

		QuantitySquare(void): Quantity(){}
		QuantitySquare(double d): Quantity(d){}
		QuantitySquare(uint64_t u): Quantity(u){}

		// Euclidean stores it's distances as squares (for speed's sake)
		// so, the string representation must return the square root
		std::string getString	(void);

		void multiply(int times);
};

class QuantityBit: public Quantity {
	public:

		QuantityBit(void): Quantity(){}
		QuantityBit(double d): Quantity(d){}
		QuantityBit(uint64_t u): Quantity(u){}

		// Hamming stores it's distances as "uint64_t"
		// so, the string representation must parse the "value" field as "uint64_t"
		std::string getString	(void);

		void multiply(int times);
};



#endif
