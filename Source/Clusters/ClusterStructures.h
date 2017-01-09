#ifndef __CLUSTER_STRUCTURES__
#define __CLUSTER_STRUCTURES__

#include "../General/Quantity.h"
#include "../General/Point.h"

#define NONE -1

// basically struct that holds a point that is assigned to a cluster
// allong with the second best (nearest) cluster, represented by its center)
// used in the "update" step
class AssignPair{
	private:
		int Assigned;					// the point that is assigned in the cluster (0,...,N-1)
		int Center2;					// the center of the second best cluster (0,...,N-1)

	public:
		AssignPair(int assign, int center2 ){
			Assigned = assign;
			Center2 = center2;
		}

		int assigned(){ return Assigned; }
		int center2 (){ return Center2; }
};

// the "initialise" steps store the distances of every point from every point in a matrix
// the matrix is symmetric, and the main diagonal is all "0" (zeroes)
// so, we keep the strict lower triangular matrix
class TriangularMatrix{
	protected:
		double** d;		// the table (stored as pointer to rows)
		int 	N;			// the number of rows

		double Zero;

		TriangularMatrix(){}

	public:

		TriangularMatrix(int n, Point** PointTable );

		virtual double& operator ()(int i, int j);		// access to the d[i][j] element

		virtual void print(void);

		virtual ~TriangularMatrix();
};

class TriangularMatrixLazy: public TriangularMatrix{
	private:
		Point** PointTable;

		double result;

	public:

		TriangularMatrixLazy(int n, Point** PointTable );

		double& operator ()(int i, int j);		// access to the d[i][j] element

		// void print(void);

		~TriangularMatrixLazy(){}
};


#endif
