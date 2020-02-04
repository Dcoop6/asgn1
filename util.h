// $Id: util.h,v 1.13 2019-10-08 13:55:31-07 - - $

// util -
//    A utility class to provide various services not conveniently
//    included in other modules.

#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

// Convenient type using to allow brevity of code elsewhere.

template <typename iterator>
using range_type = pair<iterator,iterator>;

using wordvec = vector<string>;
using word_range = range_type<decltype(declval<wordvec>().cbegin())>;

// want_echo -
//    We want to echo all of cin to cout if either cin or cout
//    is not a tty.  This helps make batch processing easier by
//    making cout look like a terminal session trace.

bool want_echo();

//
// main -
//    Keep track of execname and exit status.  Must be initialized
//    as the first thing done inside main.  Main should call:
//       main::execname (argv[0]);
//    before anything else.
//

class exec {
   private:
      static string execname_;
      static int status_;
      static void execname (const string& argv0);
      friend int main (int, char**);
   public:
      static void status (int status);
      static const string& execname() {return execname_; }
      static int status() {return status_; }
};


// split -
//    Split a string into a wordvec (as defined above).  Any sequence
//    of chars in the delimiter string is used as a separator.  To
//    Split a pathname, use "/".  To split a shell command, use " ".

wordvec split (const string& line, const string& delimiter);

// complain -
//    Used for starting error messages.  Sets the exit status to
//    EXIT_FAILURE, writes the program name to cerr, and then
//    returns the cerr ostream.  Example:
//       complain() << filename << ": some problem" << endl;

ostream& complain();

// operator<< (vector) -
//    An overloaded template operator which allows vectors to be
//    printed out as a single operator, each element separated from
//    the next with spaces.  The item_t must have an output operator
//    defined for it.

template <typename item_t>
ostream& operator<< (ostream& out, const vector<item_t>& vec) {
   string space = "";
   for (const auto& item: vec) {
      out << space << item;
      space = " ";
   }
   return out;
}

template <typename iterator>
ostream& operator<< (ostream& out, range_type<iterator> range) {
   for (auto itor = range.first; itor != range.second; ++itor) {
      if (itor != range.first) out << " ";
      out << *itor;
   }
   return out;
}

#endif

