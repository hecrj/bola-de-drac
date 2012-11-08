
#ifndef Utils_hh
#define Utils_hh

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <limits>
#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <time.h>

#include "BackTrace.hh"

using namespace std;




/**
 * Defines customized error macros.
 * (somehow adapted from err.h and error.h)
 */

#define myerror_at_line(status, error, file, line, ...) { fprintf(stderr, "Error at %s:%i: ", file, line); fprintf(stderr, __VA_ARGS__); if (error) perror(" errno"); fprintf(stderr, "\n"); exit(status); }
#define myerr(...) myerror_at_line(EXIT_FAILURE, errno, __FILE__, __LINE__, __VA_ARGS__)
#define myerrx(...) myerror_at_line(EXIT_FAILURE, 0, __FILE__, __LINE__, __VA_ARGS__)


/**
 * Hard assert.
 */
#define check(b) { if (not (b)) {BackTrace::print(); myerror_at_line(EXIT_FAILURE, errno, __FILE__, __LINE__, "Assertion `" #b "' failed in check()."); }}


/**
 * We use a macro to specifically indicate when something is unused,
 * thus the compiler doesn't cry when using NDEBUG.
 */

#define _unused(x) ((void)x)


/**
 * We use a macro to specifically indicate when some code is unreachable,
 * thus the compiler doesn't cry when using NDEBUG.
 */
#define _unreachable() { assert(0); myerrx("unreachable code reached."); }



/**
 * Defines an infinite for doubles.
 */
const double infinite = numeric_limits<double>::infinity();

/**
 * Defines maxint, minint for ints.
 */
const int maxint = numeric_limits<int>::max();
const int minint = numeric_limits<int>::min();


/**
 * Conversion: string to int.
 */
inline int s2i (const string& s) {
    istringstream iss(s);
    int i;
    iss >> i;
    return i;
}


/**
 * Conversion: int to string
 */
inline string i2s (int i) {
    ostringstream oss;
    oss << i;
    return oss.str();
}


/**
 * Size as int.
 */
template <typename T>
inline int sze (T& x) {
    return int(x.size());
}


#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

////////////////// implies connective //////////////////////////

#define implies  % (boolean)

class boolean {

public:

  boolean(bool b = false) : val(b) { }
  operator bool() { return val; }
  friend boolean  operator %(bool b1, boolean b2);

private:
  bool val;
};

inline boolean  operator %(bool b1, boolean b2) {
  return not b1 or b2.val;
}

////////////////// print macro //////////////////////////

#ifndef PRINT
#define PRINT_AUX(ARG,SEP)  {string tmpStr(SEP # ARG # SEP); \
                             cerr << boolalpha \
                                  << tmpStr.substr(1, tmpStr.size()-5)    \
                                  << ": " << ARG << endl;}
#define PRINT(ARG)          PRINT_AUX(ARG,"\"")
#endif

/////////////////// random numbers //////////////////////

const int RANDOMIZE_MODULUS    = 2147483647;
const int RANDOMIZE_MULTIPLIER = 48271;
const int RANDOMIZE_DEFAULT    = 123456789;  // Initial seed.
const int RANDOMIZE_MAX        = RANDOMIZE_MODULUS - 2;
      
extern long rnd_seed;

// Sets random seed.
inline void srandomize(int s = RANDOMIZE_DEFAULT) {
  if (s < 0) s = -s;
  rnd_seed = 1 + s % (RANDOMIZE_MODULUS - 1);
}

// Returns pseudo-random number between 0 and RANDOMIZE_MAX
inline int randomize(void) {
  const long Q = RANDOMIZE_MODULUS / RANDOMIZE_MULTIPLIER;
  const long R = RANDOMIZE_MODULUS % RANDOMIZE_MULTIPLIER;
        long t;

  t = RANDOMIZE_MULTIPLIER * (rnd_seed % Q) - R * (rnd_seed / Q);
  if (t > 0) 
    rnd_seed = t;
  else 
    rnd_seed = t + RANDOMIZE_MODULUS;
  return rnd_seed - 1;
}

#endif
