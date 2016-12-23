#ifndef __POINT__
#define __POINT__

#include "Quantity.h"
#include <stdint.h>
#include <float.h>
#include <string>
#include <iostream>

enum PointType { BARRIER, HAMMING, COSINE, EUCLIDEAN, METRIC_SPACE};

// abstract class that describes the functionalities of a multi-dimensional point
class Point{
	protected:
		std::string Name;										// a human-readable representattion of the point
		static double C;
	public:
		virtual std::string name		(void)=0;				// getter function of the above Name
		virtual int 		dimension	(void)=0;				// the dimensionality of the space the point lives into
		virtual Quantity*	multiply	(Point* )=0;			// multiplication of a point with another point
		virtual Quantity* 	value		(void)=0;				// an integer non-negative value representing the point
		virtual double 		distance	(Point*)=0;				// computes the distance between 2 points
		virtual double	 	similarity	(Point*)=0;
		virtual bool 		operator ==	(Point*)=0;				// defines the equality of 2 points
		virtual void 		print  		(void)=0;
		virtual PointType	type		(void)=0;
		virtual ~Point(){}

		virtual double multiplyDouble(double R, double times){
			return R * times;
		}

		bool inRange (Point* p, double R){
			return distance(p) < C * R;
		}

		static void setC(double c){
			C = c;
		}
};

class BarrierPoint: public Point{
	public:
		std::string name		(void){ return "___Barrier___"; }
		int 		dimension	(void){ return -1; }
		Quantity*	multiply	(Point* p){ return NULL; }
		Quantity* 	value		(void){ return NULL; }
		double 		distance	(Point* p){ return DBL_MAX; }
		double	 	similarity	(Point*){ return DBL_MAX; }
		bool 		operator ==	(Point* p){ return (Point*)this == p; }
		bool 		inRange		(Point* p, Quantity* q){ return false; }
		void 		print  		(void){ std::cout << name() << std::endl;}
		PointType	type		(void){return BARRIER; }
};

#endif
