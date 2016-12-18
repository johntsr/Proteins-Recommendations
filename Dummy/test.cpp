#include <iostream>

// #include <lapacke.h>
// #include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

using namespace std;



int main(int argc, char const *argv[]) {

	// double a[] = { 0.11, 0.12, 0.13,
	//              0.21, 0.22, 0.23 };


	int N = 7;

	double x[N*3] = { 0.11, 0.21, 0.21,
					0.12, 0.2, 0.21,
					0.13, 0.23, 0.21,
					0.13, 0.23, 0.21,
					0.13, 0.23, 0.21,
					0.13, 0.23, 0.21,
					0.13, 0.23 ,0.21};

	double y[N*3] = { 1011, 1012,1012,
	             	1021, 1022,1012,
	             	1031, 1032,1012,
				 	1031, 1032,1012,
					1031, 1032,1012,
					1031, 1032,1012,
					1031, 1032,1012};

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



	printf ("[ %g, %g]\n", gsl_vector_get(S, 0), gsl_vector_get(S, 1) );


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
			u[i*N + 2] *= -1.0;
		}

		gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &U.matrix, V, 0.0, &Q.matrix);

	}

	double d[N*3];
	gsl_matrix_view Diff = gsl_matrix_view_array(d, N, 3);
	gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, &X.matrix, &Q.matrix, 0.0, &Diff.matrix);
	gsl_matrix_sub(&Diff.matrix, &Y.matrix);

	double result = 0.0;
	for(int i = 0; i < N; i++){
		for(int j = 0; j < 3; j++){
			result += d[i*N + j] * d[i*N + j];
		}
	}
	result /= sqrt(N);


	gsl_permutation_free(p);
	gsl_matrix_free(tmpQ);
	gsl_matrix_free(V);
	gsl_vector_free(S);
	gsl_vector_free(work);

	return 0;



	// gsl_matrix* A = gsl_matrix_alloc(3, 2);
	// gsl_matrix* B = gsl_matrix_alloc(3, 2);
	//
	// for(int i = 0; i < 3; i++){
	// 	for(int j = 0; j < 2; j++){
	// 		gsl_matrix_set(A, i, j, i*j);
	// 	}
	// }
	//
	// for(int i = 0; i < 3; i++){
	// 	for(int j = 0; j < 2; j++){
	// 		cout << gsl_matrix_get(A, i, j) << " ";
	// 	}
	// 	std::cout << '\n';
	// }
	//
	// gsl_matrix_set_all(B, 7.0);
	//
	// gsl_matrix_add(A, B);
	//
	// for(int i = 0; i < 3; i++){
	// 	for(int j = 0; j < 2; j++){
	// 		cout << gsl_matrix_get(A, i, j) << " ";
	// 	}
	// 	std::cout << '\n';
	// }
	//
	// gsl_matrix_free(A);
	// gsl_matrix_free(B);
	// return 0;
}
