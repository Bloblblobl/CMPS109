// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (const ubigint& that){
   DEBUGF ('~', this << " -> " << that)
   for (udigit_t digit: that.ubig_value) {
      ubig_value.insert(ubig_value.begin(), static_cast<udigit_t>(digit));
   }
}

ubigint::ubigint (const string& that){
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      ubig_value.insert(ubig_value.begin(), static_cast<udigit_t>(digit));
   }
}

struct ordered_bigints { ubigint big; ubigint little; };
ordered_bigints ubigint::order_bigints (const ubigint& bi1, const ubigint& bi2) {
   if (bi2.ubig_value.size() > bi1.ubig_value.size()) return {.big = ubigint(bi2), .little = ubigint(bi1)};
   return {.big = ubigint(bi1), .little = ubigint(bi2)};
}

ubigint ubigint::operator+ (const ubigint& that) const {
   auto ordered = that.order_bigints(*this, that);
   udigit_t remainder = 0;
   for (std::vector<int>::size_type i = 0; i < ordered->big.ubig_value.size(); i++) {
      udigit_t curr_digit = ordered->big[i];
      curr_digit += remainder;
      remainder = 0;
      if (i <= ordered->little.ubig_value.size() + 1) curr_digit += ordered->big.ubig_value[i];
      if (curr_digit > 9) {
         remainder = curr_digit % 10;
         curr_digit = curr_digit / 10;
      }
      ordered->big.ubig_value[i] = curr_digit;
   }
   if (remainder > 0) ordered->big.ubig_value.push_back(remainder);
   return ordered->big;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (ubig_value.size() < that.ubig_value.size()) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result = ubigint(*this);
   bool borrow = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      udigit_t curr_digit = ubig_value[i];
      if (borrow) {
         borrow = false;
         curr_digit -= 1;
      }
      if (i <= that.ubig_value.size()) curr_digit -= that.ubig_value[i];
      if (curr_digit < 0) {
         borrow = true;
         curr_digit += 10;
      }
      result.ubig_value[i] = curr_digit;
   }
   if (borrow) throw domain_error ("ubigint::operator-(a<b)");
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint product = ubigint();
   auto ordered = that.order_bigints(*this, that);
   ubigint big = ordered.big;
   ubigint little = ordered.little;
   for (std::vector<int>::size_type i = 0; i < big.ubig_value.size(); i++) {
      udigit_t carry = 0;
      for (std::vector<int>::size_type j = 0; j < big.ubig_value.size(); j++) {
         udigit_t curr_digit = 0;
         bool correct_size = false;
         if (product.ubig_value.size() > i + j) {
            correct_size = true;
            curr_digit += product.ubig_value[i + j];
         }
         curr_digit += carry + big.ubig_value[i] * little.ubig_value[j];
         if (curr_digit > 9) {
            carry = curr_digit / 10;
            curr_digit = curr_digit % 10;
         }
         if (product.ubig_value.size() <= i + j) {
            while (product.ubig_value.size() < i + j) {
               product.ubig_value.push_back(0);
            }
            product.ubig_value.push_back(curr_digit)
         }
         else product.ubig_value[i + j] = curr_digit;
      }
      if (product.ubig_value.size() <= i + little.ubig_value.size()) {
         while (product.ubig_value.size() < i + little.ubig_value.size()) {
            product.ubig_value.push_back(0);
         }
         product.ubig_value.push_back(carry);
      }
      else product.ubig_value[i + little.ubig_value.size()] = carry;
   }
   return product;
}

void ubigint::multiply_by_2() {
   udigit_t carry = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      udigit_t curr_digit = ubig_value[i];
      curr_digit *= 2;
      if (carry) {
         curr_digit += 1;
         carry = false;
      }
      if (curr_digit > 9) {
         curr_digit -= 10;
         carry = true;
      }
      ubig_value[i] = curr_digit;
   }
   if (carry) ubig_value.push_back(1);
}

void ubigint::divide_by_2() {
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      ubig_value[i] /= 2;
   }
   while (ubig_value.size() > 0 and ubig_value.back() == 0) ubig_value.pop_back();
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {"0"};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {"1"};
   ubigint quotient {"0"};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if (that.ubig_value.size() != ubig_value.size()) return false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      if (that.ubig_value[i] != ubig_value[i]) return false;
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if (that.ubig_value.size() > ubig_value.size()) return true;
   if (that.ubig_value.size() < ubig_value.size()) return false;
   for (std::vector<int>::size_type i = ubig_value.size() - 1; i >= 0; i--)
   {
      if (that.ubig_value[i] <= ubig_value.size()) return false;
   }
   return true;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   out << "ubigint(";
   auto vect = that.ubig_value;
   for (std::vector<unsigned char>::reverse_iterator it = vect.rbegin(); it != vect.rend(); ++it) {
      out << *it;
   }
   return out << ")";
}

