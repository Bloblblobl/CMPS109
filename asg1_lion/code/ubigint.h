// $Id: ubigint.h,v 1.11 2016-03-24 19:43:57-07 - - $

#ifndef __UBIGINT_H__
#define __UBIGINT_H__

#include <exception>
#include <iostream>
#include <limits>
#include <utility>
using namespace std;

#include "debug.h"
#include "relops.h"

class ubigint {
   friend ostream& operator<< (ostream&, const ubigint&);
   private:
      using udigit_t = unsigned char;
   public:
      using ubigvalue_t = vector<udigit_t>;
      ubigvalue_t ubig_value;

      void multiply_by_2();
      void divide_by_2();

      ubigint() = default; // Need default ctor as well.
      ubigint (const ubigint&);
      ubigint (const string&);

      void clear_zeroes();

      ubigint operator+ (const ubigint&) const;
      ubigint operator- (const ubigint&) const;
      ubigint operator* (const ubigint&) const;
      ubigint operator/ (const ubigint&) const;
      ubigint operator% (const ubigint&) const;

      bool operator== (const ubigint&) const;
      bool operator<  (const ubigint&) const;
};

#endif

