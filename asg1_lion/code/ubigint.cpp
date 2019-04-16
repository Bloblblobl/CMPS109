// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

struct ordered_bigints { ubigint big; ubigint little; };

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

ubigint ubigint::operator+ (const ubigint& that) const {
   ordered_bigints ordered;
   ordered.big = *this;
   ordered.little = that;
   if (that.ubig_value.size() > this->ubig_value.size()) {
      ordered.big = that;
      ordered.little = *this;
   }
   int remainder = 0;
   for (std::vector<int>::size_type i = 0; i < ordered.big.ubig_value.size(); i++) {
      int curr_digit = static_cast<int>(ordered.big[i]);
      curr_digit += remainder;
      remainder = 0;
      if (i <= ordered.little.ubig_value.size() + 1) curr_digit += static_cast<int>(ordered.big.ubig_value[i]);
      if (curr_digit > 9) {
         remainder = curr_digit % 10;
         curr_digit = curr_digit / 10;
      }
      ordered.big.ubig_value[i] = static_cast<udigit_t >(curr_digit);
   }
   if (remainder > 0) ordered.big.ubig_value.push_back(remainder);
   return ordered.big;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (ubig_value.size() < that.ubig_value.size()) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result = ubigint(*this);
   bool borrow = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      int curr_digit = static_cast<int>(ubig_value[i]);
      if (borrow) {
         borrow = false;
         curr_digit -= 1;
      }
      if (i <= that.ubig_value.size()) curr_digit -= that.ubig_value[i];
      if (curr_digit < 0) {
         borrow = true;
         curr_digit += 10;
      }
      result.ubig_value[i] = static_cast<udigit_t >(curr_digit);
   }
   if (borrow) throw domain_error ("ubigint::operator-(a<b)");
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint product = ubigint();
   ordered_bigints ordered;
   ordered.big = *this;
   ordered.little = that;
   if (that.ubig_value.size() > this->ubig_value.size()) {
      ordered.big = that;
      ordered.little = *this;
   }
   ubigint big = ordered.big;
   ubigint little = ordered.little;
   for (std::vector<int>::size_type i = 0; i < big.ubig_value.size(); i++) {
      int carry = 0;
      for (std::vector<int>::size_type j = 0; j < big.ubig_value.size(); j++) {
         int curr_digit = 0;
         bool correct_size = false;
         if (product.ubig_value.size() > i + j) {
            correct_size = true;
            curr_digit += static_cast<int>(product.ubig_value[i + j]);
         }
         curr_digit += carry + static_cast<int>(big.ubig_value[i] * little.ubig_value[j]);
         if (curr_digit > 9) {
            carry = curr_digit / 10;
            curr_digit = curr_digit % 10;
         }
         if (product.ubig_value.size() <= i + j) {
            while (product.ubig_value.size() < i + j) {
               product.ubig_value.push_back(0);
            }
            product.ubig_value.push_back(static_cast<udigit_t >(curr_digit));
         }
         else product.ubig_value[i + j] = static_cast<int>(curr_digit);
      }
      if (product.ubig_value.size() <= i + little.ubig_value.size()) {
         while (product.ubig_value.size() < i + little.ubig_value.size()) {
            product.ubig_value.push_back(0);
         }
         product.ubig_value.push_back(static_cast<udigit_t>(carry));
      }
      else product.ubig_value[i + little.ubig_value.size()] = static_cast<udigit_t>(carry);
   }
   return product;
}

void ubigint::multiply_by_2() {
   bool carry = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      int curr_digit = static_cast<int>(ubig_value[i]);
      curr_digit *= 2;
      if (carry) {
         curr_digit += 1;
         carry = false;
      }
      if (curr_digit > 9) {
         curr_digit -= 10;
         carry = true;
      }
      ubig_value[i] = static_cast<udigit_t>(curr_digit);
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
   for (std::vector<int>::size_type i = ubig_value.size(); i > 0; i--)
   {
      if (that.ubig_value[i - 1] <= ubig_value[i - 1]) return false;
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

