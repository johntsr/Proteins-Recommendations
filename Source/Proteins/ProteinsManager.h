#ifndef __IO_NAMAGER__
#define  __IO_NAMAGER__

#include "../General/Point.h"
#include "../Clusters/Clusters.h"

#include <iostream>
#include <string>
#include <fstream>

typedef int (*rGenerator)(int N);

class ProteinsManager{													// abstract class used to communicate with the user
	protected:
		Point** PointTable;					// the table of points to be clustered
		TriangularMatrix* d;				// the table of precomputed distances
		int 	numConform;					// thr number of points
		int 	N;
		ProteinsCluster* Algorithm;			// the algorithms to run
		double BestTime;
		int K_clusters;						// the number of clusters
		bool Complete;						// whether to print all the clusters or just the centers

		void getPath		(std::string& path, std::string message);// promts "message" to the user, then takes "path" from stdin
		void openFileWrite	(std::string& path, std::ofstream& file);// open a file in order to write to it
		void fillTable		(std::string dataPath);					// fills "PointTable" form a file
		void runTests		(std::ofstream& outfFile);				// calls the above methods for every query point
		void finalise		(void);

		// the above methods vary between the different Points
		virtual void 	openFileRead 	(std::string& path, std::ifstream& file);// open a file in order to read it
		virtual Point* 	getNextPoint	(std::ifstream& queryFile)=0;// get the next point from a file
		virtual void 	evaluate		(std::ofstream& outfFile)=0;
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
public:
	cRMSDManager(bool complete);
	~cRMSDManager();
};


enum dOption { SMALLEST, LARGEST, RANDOM };

class dRMSDManager: public ProteinsManager{
	private:
		rGenerator Func;
		dOption T;

		Point* 	getNextPoint	(std::ifstream& queryFile);		// get the next point from a file
		void 	evaluate		(std::ofstream& outfFile);

	public:
		dRMSDManager(dOption t, rGenerator func, bool complete);

};



#endif
