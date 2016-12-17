#ifndef __IO_NAMAGER__
#define  __IO_NAMAGER__

#include "../DataStructures/LocalityHashing.h"
#include "Point.h"

#include <iostream>
#include <string>
#include <fstream>


class EOF_Exception{};


class IOManager{													// abstract class used to communicate with the user
	protected:
		LocalHashTable<Point, Point*>* LSH;							// the Locality Sensitive Hash Table
		int L;														// the number of Hash Tables the above table uses
		int K;														// the number of "h" hash functions we use
		int Dimension;												// the dimension of the Points we store
		hash_function** hashFunctions;								// table of hash functions

		int  countLines		(std::string fileName, int& dimension ); // count the number of lines of a file (equivalent to countin the points)
		// the above method also counts the dimensionality of the points, store it in "dimension"

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		bool wantsMore		(void);									 // whether the user wants to perform more queries or not


		void inRange				(Point* newPoint, Quantity* R, std::ofstream& outFile);	// computes the neigbours of a given Point
		double nearestNeighbour		(Point* newPoint, std::ofstream& outFile);				// computes the approximate nearest neigbour
		double nearestNeighbourBrute(Point* newPoint, std::ofstream& outFile);				// computes the true nearest neigbour
		void runTests				(std::string& queryPath, std::ofstream& outfFile);		// calls the above methods for every query point

		// the above methods vary between the different Points
		virtual Point* 	 	getNextPoint	(std::ifstream& queryFile)=0;			// get the next point from a file
		virtual Quantity* 	getRadius		(std::ifstream& queryFile)=0;			// get the radius of the "inRange" query
		virtual void 		createLSH		(int barrier, std::string dataPath)=0;	// create the LSH
		virtual int  		countDimension	(std::string fileName);					// count the dimensionality of the points

	public:
		IOManager(int l, int k);

		void run(int barrier, std::string& dataPath, std::string& outPath, std::string& queryPath);		// initiate the procedure of "train & test"

		virtual ~IOManager(){
			delete LSH;
			for(int i = 0; i < L; i++){
				delete hashFunctions[i];
			}
			delete[] hashFunctions;
		}
};


class HammingManager: public IOManager{
	private:
		Point* 		getNextPoint(std::ifstream& queryFile);					// get the next point from a file
		Quantity* 	getRadius	(std::ifstream& queryFile);					// get the radius of the "inRange" query
		void 		createLSH	(int barrier, std::string dataPath);		// create the LSH
		int 	 	countDimension	(std::string fileName);				 	// count the dimensionality of the points

	public:
		HammingManager(int l, int k);

};

class CosineManager: public IOManager{
	private:
		Point* 		getNextPoint(std::ifstream& queryFile);					// get the next point from a file
		Quantity* 	getRadius	(std::ifstream& queryFile);					// get the radius of the "inRange" query
		void 		createLSH	(int barrier, std::string dataPath);		// create the LSH
	public:
		CosineManager(int l, int k);

};

class EuclideanManager: public IOManager{
	private:
		int W;
		bool Explicit;

		Point* 		getNextPoint(std::ifstream& queryFile);					// get the next point from a file
		Quantity* 	getRadius	(std::ifstream& queryFile);					// get the radius of the "inRange" query
		void 		createLSH	(int barrier, std::string dataPath);		// create the LSH
	public:
		EuclideanManager(int l, int k, bool flag, int w = 4);

};

class MetricSpaceManager: public IOManager{
	private:
		std::string* Names;
		Point** Points;
		int currentPointIndex;

		bool readingData;

		Point* 		getNextPoint(std::ifstream& queryFile);					// get the next point from a file
		Quantity* 	getRadius	(std::ifstream& queryFile);					// get the radius of the "inRange" query
		void 		createLSH	(int barrier, std::string dataPath);		// create the LSH
	public:
		MetricSpaceManager(int l, int k);
		~MetricSpaceManager();
};

#endif
