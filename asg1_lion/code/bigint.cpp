// $Id: bigint.cpp,v 1.78 2019-04-03 16:44:33-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const bigint& that):
                uvalue(that.uvalue), is_negative(that.is_negative) {

}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   if (is_negative != that.is_negative) {
      bigint big = *this;
      bigint little = that;
      if (that.uvalue.ubig_value.size() > uvalue.ubig_value.size()) {
         big = that;
         little = *this;
      }
      return {big.uvalue - little.uvalue, big.is_negative};
   }
   return {uvalue + that.uvalue, is_negative};
}

bigint bigint::operator- (const bigint& that) const {
   if (is_negative != that.is_negative) return {uvalue + that.uvalue, is_negative};
   return {uvalue - that.uvalue};
}


bigint bigint::operator* (const bigint& that) const {
   bool sign = false;
   if (is_negative != that.is_negative) sign = true;
   return {uvalue * that.uvalue, sign};
}

bigint bigint::operator/ (const bigint& that) const {
   bool sign = false;
   if (is_negative != that.is_negative) sign = true;
   return {uvalue / that.uvalue, sign};
}

bigint bigint::operator% (const bigint& that) const {
   bool sign = false;
   if (is_negative != that.is_negative) sign = true;
   return {uvalue % that.uvalue, sign};
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   if (that.is_negative) out << "-";
   return out << that.uvalue;
}
