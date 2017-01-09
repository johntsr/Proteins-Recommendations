#ifndef __PROTEIN_MANAGER__
#define  __PROTEIN_MANAGER__

#include "../General/Point.h"
#include "../Clusters/Clusters.h"

#include <iostream>
#include <string>
#include <fstream>

typedef int (*rGenerator)(int N);			// functions used to generate "r" for d-RMSD

// abstract class used to communicate with the user
class ProteinsManager{
	protected:
		Point** PointTable;					// the table of points to be clustered
		TriangularMatrix* d;				// the table of precomputed distances
		int 	numConform;					// the number of proteins
		int 	N;							// the number of aminoacids per protein
		ProteinsCluster* Algorithm;			// the algorithms to run
		double BestTime;					// clustering time of best algorithm
		int K_clusters;						// the number of clusters
		bool Complete;						// whether to print all the clusters or just the centers

		bool RandCluster;					// NOTE: typical Initializers compute everything, random does not!

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void fillTable		(std::string dataPath);					// fills "PointTable" form a file
		void runTests		(std::ofstream& outfFile);				// calls the above methods for every query point
		void finalise		(void);

		// the above methods vary between the different Points
		virtual void 	openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		virtual Point* 	getNextPoint	(std::ifstream& queryFile)=0;// get the next point from a file
		virtual void 	evaluate		(std::ofstream& outfFile)=0;
		virtual void 	printMessage	(void)=0;
	public:
		ProteinsManager(bool complete);

		void run(std::string& dataPath, std::string& outPath);		// initiate the procedure of "train & test"
		void runCUTests(void);

		virtual ~ProteinsManager();
};


class cRMSDManager: public ProteinsManager{
private:
	Point* 	getNextPoint	(std::ifstream& queryFile);					// get the next point from a file
	void 	evaluate		(std::ofstream& outfFile);
	void 	printMessage	(void);
public:
	cRMSDManager(bool complete);
	~cRMSDManager();
};


enum rOption { SMALL, MEDIUM, LARGE };
enum pickOption { SMALLEST, LARGEST, RANDOM };

struct PairDummy{
	int i;
	int j;
};

class dRMSDManager: public ProteinsManager{
	private:
		static double* Configuration;	// array that holds all the info from file

		int R; 							// # of distances needed to form a vector out of a protein
		PairDummy* Indexes;				// the pairs of the edges of the above distances (stored as indexes)
		rGenerator Func;				// function that generates "R" out of "N"
		pickOption T;					// option for LARGEST, SMALLEST or RANDOM for picking the distances
		rOption rOpt;

		Point* 	getNextPoint	(std::ifstream& queryFile);		// get the next point from a file
		void 	evaluate		(std::ofstream& outfFile);
		double 	distance		(int i, int j);					// distance of protein i from protein j
		void 	printMessage	(void);

	public:
		dRMSDManager(pickOption t, rOption, bool complete);
		~dRMSDManager();
};



#endif
