/******************************************************************************
 * Hackerboat Vector module
 * twovector.hpp
 * This module manipulates two dimension vectors 
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef TWOVECTOR_H
#define TWOVECTOR_H
 
#include <stdlib.h>
#include <cmath> 
 
/**
 * @brief The TwoVector class provides operations for working with 2D vectors in either cartesian or polar forms.
 */
class TwoVector : public HackerboatState {
	public:
		TwoVector () {};
		TwoVector (double x, double y) :
			_x(x), _y(y) {};
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid ();
		
		// functions for deriving cartesian coordinates from polar input
		static double getXrad(double ang, double mag);
		static double getYrad(double ang, double mag);
		static double getXdeg(double ang, double mag);
		static double getYdeg(double ang, double mag);
		
		// get/set cartesian values, angle, or magnitude 
		double inline x () {return _x;};
		void inline x (double x) {_x=x;};
		double inline y () {return _y;};
		void inline y (double y) {_y=y;};
		double inline mag () {return sqrt((_x*_x)+(_y*_y));};
		void mag (double _mag);
		double inline angleRad () {return atan2(_y,_x);}; 
		void angleRad (double _ang);
		double inline angleDeg () {return rad2deg(atan2(_y,_x));}; 
		void angleDeg (double _ang);
		
		// rotations
		void rotateDeg (double _deg) {rotateRad(deg2rad(_deg));};
		void rotateRad (double _rad);
		
		// vector math
		inline TwoVector& operator+= (const TwoVector& r) {					/**< Vector addition */
			this->_x += r._x; 
			this->_y += r._y; 
			return *this; 
		};
		
		inline TwoVector& operator-= (const TwoVector& r) {					/**< Vector subtraction */ 
			this->_x -= r._x; 
			this->_y -= r._y; 
			return *this; 
		};
		
		inline TwoVector& operator*= (const double& r) {						/**< Scalar multiplication */ 
			this->_x *= r; 
			this->_y *= r; 
			return *this; 
		};
		
		friend double inline operator* (const TwoVector& l, const TwoVector& r) {	/**< Dot product */ 
			return ((l._x*r._x)+(l._y*r._y)); 
		};
		
		friend TwoVector inline operator+ (TwoVector l, const TwoVector& r) {	/**< Vector addition */
			l += r;
			return l;
		};
		
		friend TwoVector inline operator- (TwoVector l, const TwoVector& r) {	/**< Vector subtraction */ 
			l -= r;
			return l;
		}
		
		friend TwoVector inline operator* (TwoVector l, const double r) {		/**< Scalar multiplication */
			l *= r;
			return l;
		}
		
		friend TwoVector inline operator/ (TwoVector l, const double r) {		/**< Scalar division */
			l *= (1/r);
			return l;
		}
		
		// other vector functions
		TwoVector rotate (double angle);	/**< Rotate vector through the given angle */
		TwoVector unit ();					/**< Get the corresponding unit vector */
		
		// utility functions
		static double inline deg2rad (double deg) { return deg * ( M_PI / 180.0 ); }
		static double inline rad2deg (double rad) { return rad * ( 180.0 / M_PI ); }
		
	protected:
		double _x;
		double _y;
	
};

#endif /* TWOVECTOR_H */
