/*
 *
 */


#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
#include <ImfRgba.h>
#include <ImfHeader.h>
#include <stdio.h>
#include <string.h>
#include <boost/math/constants/constants.hpp>
#include <math.h>
#include <ImathBox.h>

#include <iostream>
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

using namespace std;
using namespace Imf;
using namespace Imath;
const double pi = boost::math::constants::pi<double>();
const double e = boost::math::constants::e<double>();


