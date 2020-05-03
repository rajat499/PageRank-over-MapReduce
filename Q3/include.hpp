/*
 * Including libraries, for avoiding repetitive include.
 */

#ifndef __INCLUDE
#define __INCLUDE

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <random>

#include <vector>
#include <tuple>
#include <map>
#include <set>
#include <functional>

#include <mpi.h>

#include <unordered_map>
#include <unordered_set>

#include <algorithm>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

using namespace std;

#ifdef __linux__

#define      __stderrp stderr
#define      __stdoutp stdout
#define      __stdinp  stdin

#define      uint16_t  u_int16_t
#endif


/*
 * Default file access permissions for new files.
 */
#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#endif