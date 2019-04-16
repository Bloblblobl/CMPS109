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
                uvalue(that.uvalue), is_negative(that.is_negative)

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

struct ordered_bigints { bigint big; bigint little; };
ordered_bigints bigint::order_bigints (const bigint& bi1, const bigint& bi2) {
   if (bi2.uvalue.ubig_value.size() > bi1.uvalue.ubig_value.size()) return {.big = &bigint(bi2), .little = &bigint(bi1)};
   return {.big = &bigint(bi1), .little = &bigint(bi2)};
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   if (is_negative != that.is_negative) {
      auto ordered = that.order_bigints(*this, that);
      return {ordered.big.uvalue - ordered.little.uvalue, ordered.big.is_negative}
   }
   return {uvalue + that.uvalue, is_negative};
}

bigint bigint::operator- (const bigint& that) const {
   if (is_negative != that.is_negative) return {uvalue + that.uvalue, is_negative}
   return {uvalue - that.uvalue};
}


bigint bigint::operator* (const bigint& that) const {
   bool sign = true;
   if (is_negative != that.is_negative) sign = false;
   return {uvalue * that.uvalue, sign};
}

bigint bigint::operator/ (const bigint& that) const {
   bool sign = true;
   if (is_negative != that.is_negative) sign = false;
   return {uvalue / that.uvalue, sign};
}

bigint bigint::operator% (const bigint& that) const {
   bool sign = true;
   if (is_negative != that.is_negative) sign = false;
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
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}

