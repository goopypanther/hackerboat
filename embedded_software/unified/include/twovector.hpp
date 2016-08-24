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

#include <stdlib.h>
#include <cmath> 
 
/**
 * @brief The TwoVector class provides operations for working with 2D vectors in either cartesian or polar forms.
 */
class TwoVector : public hackerboatStateClass {
	public:
		TwoVector () {};
		TwoVector (double x, double y) :
			_x(x), _y(y) {};
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid ();
		
		// get/set cartesian values, angle, or magnitude 
		double x () {return x_;};
		bool x (double x) {_x=x;};
		double y () {return y_;};
		bool y (double y) {_y=y;};
		double mag () {return sqrt((_x*_x)+(_y*_y));};
		bool mag (double _mag);
		double angle () {return atan2(_y,_x);}; 
		bool angle (double _ang);
		
		// vector math
		TwoVector& operator+= (const TwoVector& r) {					/**< Vector addition */
			this->_x += r->_x; 
			this->_y += r->_y; 
			return *this; 
		};
		
		TwoVector& operator-= (const TwoVector& r) {					/**< Vector subtraction */ 
			this->_x -= r->_x; 
			this->_y -= r->_y; 
			return *this; 
		};
		
		TwoVector& operator*= (const double& r) {						/**< Scalar multiplication */ 
			this->_x *= r; 
			this->_y *= r; 
			return *this; 
		};
		
		friend double operator* (const TwoVector& l, const TwoVector& r) {	/**< Dot product */ 
			return ((l->_x*r->_x)+(l->_y*r->_y)); 
		};
		
		friend TwoVector operator+ (TwoVector l, const TwoVector& r) {	/**< Vector addition */
			l += r;
			return l;
		};
		
		friend TwoVector operator- (TwoVector l, const TwoVector& r) {	/**< Vector subtraction */ 
			l -= r;
			return l;
		}
		
		friend TwoVector operator* (TwoVector l, const double r) {		/**< Scalar multiplication */
			l *= r;
			return l;
		}
		
		friend TwoVector operator/ (TwoVector l, const double r) {		/**< Scalar division */
			l *= (1/r);
			return l;
		}
		
		// other vector functions
		TwoVector rotate (double angle);	/**< Rotate vector through the given angle */
		TwoVector unit ();					/**< Get the corresponding unit vector */
		
	protected:
		double _x;
		double _y;
	
};
