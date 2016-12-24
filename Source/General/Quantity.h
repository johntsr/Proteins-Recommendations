#ifndef __QUANTITY__
#define __QUANTITY__

#include <string>
#include <stdint.h>
#include <bitset>

struct Pair{
	int Item;
	int Rating;

	Pair(int item, int rating){
		Item = item;
		Rating = rating;
	}
};

class Bitset{
	private:
		std::bitset<64>* Array;
		int Size;
		int Dimension;

		int Count;
	public:
		Bitset(std::string bitString);
		Bitset(int size, int count = 0);

		~Bitset();

		int dimension(void);

		Bitset* operator & (Bitset& b);
		Bitset* operator ^ (Bitset& b);
		unsigned int count(void);

		void setFirst(int count);
		void set(int position);

		std::string getString(void);
};

// class used for exchanging information between various classes and methods
class Quantity{
	protected:
		// this information can always be stored in 64 bits
		// that's because either "double" or "uint64_t" is exhanged
		// since only 1 class (Hamming) needs "uint64_t"
		// we use "double" for speed's sake
		double value;
		Bitset* Bits;
		bool toDelete;

	public:

		Quantity(void);							// no argument constructor
		Quantity(double d);						// store a "double"
		Quantity(std::string bitString);
		Quantity(Bitset* bits, bool del);

		virtual double 		getDouble	(void);	// parse the stored info as "double"
		virtual Bitset* 	getBits		(void);	// parse the stored info as "uint64_t"

		virtual void setDouble(double d);				// store a "double"
		virtual void setBits(Bitset* u);			// store a "uint64_t"

		virtual ~Quantity(){
			if( toDelete ){
				delete Bits;
			}
		}
};

#endif
