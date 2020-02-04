// $Id: debug.cpp,v 1.15 2020-01-22 14:21:55-08 - - $

#include <climits>
#include <iostream>
#include <vector>

using namespace std;

#include "debug.h"
#include "util.h"

debugflags::flagset_ debugflags::flags_ {};

void debugflags::setflags (const string& initflags) {
   for (const unsigned char flag: initflags) {
      if (flag == '@') flags_.set();
                  else flags_.set (flag, true);
   }
}

// getflag -
//    Check to see if a certain flag is on.

bool debugflags::getflag (char flag) {
   // WARNING: Don't TRACE this function or the stack will blow up.
   return flags_.test (static_cast<unsigned char> (flag));
}

void debugflags::where (char flag, const char* file, int line,
                        const char* pretty_function) {
   cout << "DEBUG(" << flag << ") "
        << file << "[" << line << "] " << endl
        << "... " << pretty_function << endl;
}

