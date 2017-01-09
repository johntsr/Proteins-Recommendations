#include "MetricSpace.h"
#include "../General/Math.h"

#include <float.h>
#include <limits.h>
#include <sstream>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

double computeDistance(double* x, double* y, int N){
	// std::cout << "get in!" << std::endl;
	double u[3*3];

	gsl_matrix_view X = gsl_matrix_view_array(x, N, 3);
	gsl_matrix_view Y = gsl_matrix_view_array(y, N, 3);
	gsl_matrix_view U = gsl_matrix_view_array(u, 3, 3);

	/* Compute C = A B */

	// TODO: C IS REPLACED IS BY C!!!!
	gsl_blas_dgemm (CblasTrans, CblasNoTrans,
	              1.0, &X.matrix, &Y.matrix,
	              0.0, &U.matrix);


	gsl_matrix * V = gsl_matrix_alloc(3, 3);
	gsl_vector * S = gsl_vector_alloc(3);
	gsl_vector * work = gsl_vector_alloc(3);

	// TODO: C IS REPLACED IS BY U!!!!
	gsl_linalg_SV_decomp(&U.matrix, V, S, work);

	double q[3*3];
	gsl_matrix_view Q = gsl_matrix_view_array(q, 3, 3);
	gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &U.matrix, V, 0.0, &Q.matrix);


	int signum = 0.0;
	gsl_permutation *p = gsl_permutation_alloc(3);
	gsl_matrix *tmpQ = gsl_matrix_alloc(3, 3);
	gsl_matrix_memcpy(tmpQ, &Q.matrix);
	gsl_linalg_LU_decomp(tmpQ , p , &signum);
	double det = gsl_linalg_LU_det(tmpQ, signum);
	if( det < 0.0 ){
		for(int i = 0; i < N; i++){
			u[i*3 + 2] *= -1.0;
		}

		gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &U.matrix, V, 0.0, &Q.matrix);
	}

	double* d = new double[N*3];
	gsl_matrix_view Diff = gsl_matrix_view_array(d, N, 3);
	gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &X.matrix, &Q.matrix, 0.0, &Diff.matrix);
	gsl_matrix_sub(&Diff.matrix, &Y.matrix);

	double result = 0.0;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < 3; j++){
			result += d[i*3 + j] * d[i*3 + j];
		}
	}
	result /= sqrt(N);


	gsl_permutation_free(p);
	gsl_matrix_free(tmpQ);
	gsl_matrix_free(V);
	gsl_vector_free(S);
	gsl_vector_free(work);
	delete[] d;

	// std::cout << "get out! (result = " << result << ")" << std::endl;
	return result;
}

/*****************************************************************
***************** MetricSpacePoint class methods ******************
******************************************************************/

MetricSpacePoint::MetricSpacePoint(std::string name, int length, int n, int position, double* configuration){

	Point::Name = name;
	Position = position;

	N  = n;
	Configuration = configuration;

	Length = length;
	DistanceMatrix = new double[Length];
	for(int i = 0; i < Length; i++){
		DistanceMatrix[i] = -1.0;
	}

	double Xc[3] = { 0.0, 0.0, 0.0};
	for(int i = 0; i < 3*N; i += 3){
		Xc[0] += Configuration[i];
		Xc[1] += Configuration[i+1];
		Xc[2] += Configuration[i+2];
	}
	Xc[0] /= N * 1.0;
	Xc[1] /= N * 1.0;
	Xc[2] /= N * 1.0;

	for(int i = 0; i < 3*N; i += 3){
		Configuration[i] -= Xc[0];
		Configuration[i+1] -= Xc[1];
		Configuration[i+2] -= Xc[2];
	}

}

// @override
std::string MetricSpacePoint::name(void) {			// the human-readable representation of the point
	return Point::Name;
}

// @override
int MetricSpacePoint::dimension(void) {				// the dimensionality of the point
	return Length;
}

// @override
Quantity* MetricSpacePoint::value(void){			// maps a point into a non-negative integer value
	return new Quantity( Position * 1.0 );
}

// @override
Quantity* MetricSpacePoint::multiply (Point* p){	// defines the multiplication between two points
	return NULL;									// NOTHING!
}

// @override
double MetricSpacePoint::distance(Point* p){
	Quantity* temp = p->value();
	int position = (int)temp->getDouble();
	delete temp;

	if( DistanceMatrix[position] == -1.0 ){
		DistanceMatrix[position] = -2.0;
		DistanceMatrix[position] = p->distance(this);
		return DistanceMatrix[position];
	}
	else if( DistanceMatrix[position] == -2.0 ){
		// calculate distance, store in DistanceMatrix[ position ]
		MetricSpacePoint* pMetric = (MetricSpacePoint*)p;
		double* otherConfiguration = pMetric->Configuration;

		double* x = new double[N*3];
		double* y = new double[N*3];

		for(int i = 0; i < 3*N; i++){
			x[i] = Configuration[i];
			y[i] = otherConfiguration[i];
		}

		double result = computeDistance(x, y, N);
		// return new Quantity(result);

		delete[] x;
		delete[] y;

		return result;
	}
	else{
		return DistanceMatrix[ position ];
	}
}

// @override
double MetricSpacePoint::similarity(Point* p){
	return DBL_MAX;
}

// @override
bool MetricSpacePoint::operator == (Point* p){
	if( dimension() != p->dimension() ){
		return false;
	}

	return Point::Name == p->name();
}

// @override
void MetricSpacePoint::print(void){
	std::cout 	<< "\t(" << name() << ")"  << std::endl;
	for(int i = 0; i < Length; i++){
		std::cout << DistanceMatrix[i] << " ";
	}
	std::cout << std::endl << std::endl;
}

MetricSpacePoint::~MetricSpacePoint(){
	delete[] DistanceMatrix;
	delete[] Configuration;
}

// @override
PointType MetricSpacePoint::type(void){
	return METRIC_SPACE;
}

/*****************************************************************
***************** MetricSpace_h class methods ************************
******************************************************************/

double MetricSpace_h::compute(Point* x){
	double d1 = x->distance(x1);
	d1 *= d1;

	double d2 = x->distance(x2);
	d2 *= d2;

	return ( d1 + d2 - dd ) / ( _2d );
}

MetricSpace_h::MetricSpace_h(Point** Points, int n){

	// "Points": table of ala points of the data set
	// randomply pick "x1" and "x2" from there
	x1 = Points[ (int)Math::dRand(0, n) ];
	x2 = Points[ (int)Math::dRand(0, n) ];

	double tempd = x1->distance(x2);

	_2d = tempd * 2;
	dd = tempd * tempd;

	// t1 is median of ...


	// double sum = 0.0;
	// for( int i = 0; i < n; i++){
	// 	sum += compute( Points[i] );
	// }
	// t1 = sum / n;

	double* Results = new double[n];
	for( int i = 0; i < n; i++){
		Results[i] = compute( Points[i] );
	}
	t1 = Math::selection(Results, n, n/2 + 1);
	delete[] Results;
}

uint64_t MetricSpace_h::hash(Point* p ){					// map a multi-dimensional hamming point into an integer
	if (  compute(p) < t1 ){								// if the result in 0, then the bit was 0
		return 0;
	}
	else{												// non negative result means the bit was 1
		return 1;
	}
}
