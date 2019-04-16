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
      ubig_value.push_back(static_cast<udigit_t>(digit));
   }
}

ubigint::ubigint (const string& that){
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      auto it = ubig_value.begin();
      ubig_value.insert(it, digit);
   }
}

void ubigint::clear_zeroes() {
   while (ubig_value.size() > 1 and static_cast<int>(ubig_value.back() - '0') == 0) ubig_value.pop_back();
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint result;
   ubigint big = *this;
   ubigint little = that;
   if (that.ubig_value.size() > this->ubig_value.size()) {
      big = that;
      little = *this;
   }
   bool remainder = false;
   for (std::vector<int>::size_type i = 0; i < big.ubig_value.size(); i++) {
      int curr_digit = (big.ubig_value[i] - '0');
      if (remainder) {
         remainder = false;
         curr_digit += 1;
      }
      if (i < little.ubig_value.size()) {
         curr_digit +=  (little.ubig_value[i] - '0');
      }
      if (curr_digit > 9) {
         remainder = true;
         curr_digit = curr_digit % 10;
      }
      result.ubig_value.push_back((curr_digit + '0'));
   }
   if (remainder) {
      result.ubig_value.push_back('1');
   }
   result.clear_zeroes();
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (ubig_value.size() < that.ubig_value.size()) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result = ubigint(*this);
   bool borrow = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      int curr_digit = (ubig_value[i] - '0');
      if (borrow) {
         borrow = false;
         curr_digit -= 1;
      }
      if (i < that.ubig_value.size()) {
         curr_digit -= (that.ubig_value[i] - '0');
      }
      if (curr_digit < 0) {
         borrow = true;
         curr_digit += 10;
      }
      result.ubig_value[i] = (curr_digit + '0');
   }
   if (borrow) throw domain_error ("ubigint::operator-(a<b)");
   result.clear_zeroes();
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint product = ubigint();
   for (std::vector<int>::size_type i = 0; i < (ubig_value.size() + that.ubig_value.size()); i++) {
      product.ubig_value.push_back('0');
   }
   ubigint big = *this;
   ubigint little = that;
   if (that.ubig_value.size() > this->ubig_value.size()) {
      big = that;
      little = *this;
   }
   for (std::vector<int>::size_type i = 0; i < big.ubig_value.size(); i++) {
      int carry = 0;
      for (std::vector<int>::size_type j = 0; j < little.ubig_value.size(); j++) {
         int curr_digit = 0;
         curr_digit += (product.ubig_value[i + j] - '0');
         curr_digit += carry + (big.ubig_value[i] -'0') * (little.ubig_value[j] - '0');
         if (curr_digit > 9) {
            carry = curr_digit / 10;
            curr_digit = curr_digit % 10;
         }
         else carry = 0;
         product.ubig_value[i + j] = (curr_digit + '0');
      }
      product.ubig_value[i + little.ubig_value.size()] = (carry + '0');
   }
   product.clear_zeroes();
   return product;
}

void ubigint::multiply_by_2() {
   bool carry = false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      int curr_digit = (ubig_value[i] - '0');
      curr_digit *= 2;
      if (carry) {
         curr_digit += 1;
         carry = false;
      }
      if (curr_digit > 9) {
         curr_digit -= 10;
         carry = true;
      }
      ubig_value[i] = (curr_digit + '0');
   }
   if (carry) ubig_value.push_back('1');
   clear_zeroes();
}

void ubigint::divide_by_2() {
   if (ubig_value.size() == 1 and ubig_value[0] == static_cast<udigit_t>(0)) {
      ubig_value.pop_back();
   }
   else {
      for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
         int curr_digit = (ubig_value[i] - '0');
         if (curr_digit % 2 == 1 and i > 0) {
            ubig_value[i - 1] += static_cast<udigit_t>(5);
         }
         curr_digit /= 2;
         ubig_value[i] = (curr_digit  + '0');
      }
   }
   clear_zeroes();
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {"0"};
   if (divisor == zero or divisor.ubig_value.size() == 0) throw domain_error ("udivide by zero");
   if (dividend == zero or dividend.ubig_value.size() == 0) {
      zero.clear_zeroes();
      return {.quotient = ubigint(zero), .remainder = ubigint(zero)};
   }
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
   quotient.clear_zeroes();
   remainder.clear_zeroes();
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   ubigint result = udivide (*this, that).quotient;
   result.clear_zeroes();
   return result;
}

ubigint ubigint::operator% (const ubigint& that) const {
   ubigint result = udivide (*this, that).remainder;
   result.clear_zeroes();
   return result;
}

bool ubigint::operator== (const ubigint& that) const {
   if (that.ubig_value.size() != ubig_value.size()) return false;
   for (std::vector<int>::size_type i = 0; i < ubig_value.size(); i++) {
      if (that.ubig_value[i] != ubig_value[i]) return false;
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if (ubig_value.size() < that.ubig_value.size()) return true;
   if (ubig_value.size() > that.ubig_value.size()) return false;
   for (int i = ubig_value.size() - 1; i >= 0; i--) {
      if (ubig_value[i] > that.ubig_value[i]) return false;
      if (ubig_value[i] < that.ubig_value[i]) return true;
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   if (that.ubig_value.size() == 0) {
      out << "0";
   }
   else {
      for (int i = that.ubig_value.size() - 1; i >= 0; i--) {
         out << that.ubig_value[i];
      }
   }
   return out;
}

