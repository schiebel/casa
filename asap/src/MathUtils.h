//#---------------------------------------------------------------------------
//# MathUtilities.h: General math operations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: MathUtils.h 2619 2012-08-02 08:19:59Z TakeshiNakazato $
//#---------------------------------------------------------------------------
#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <string>
#include <vector>
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/IPosition.h>

namespace mathutil {

// Hanning smoothing
/**
 * Hanning smooth a masked vector
 * @param out the smoothed vector
 * @param outmask  the smoothed mask
 * @param in the input vector
 * @param mask the input mask
 * @param relaxed a weighting scheme
 * @param ignoreOther drop every second channel (NYI)
 */
void hanning(casa::Vector<casa::Float>& out,
	     casa::Vector<casa::Bool>& outmask,
             const casa::Vector<casa::Float>& in,
	     const casa::Vector<casa::Bool>& mask,
             casa::Bool relaxed=casa::False,
             casa::Bool ignoreOther=casa::False);

/**
 * Apply a running median to  a masked vector.
 * Edge solution:  The first and last hwidth channels will be replicated
 * from the first/last value from a full window.
 * @param out the smoothed vector
 * @param outmask  the smoothed mask
 * @param in the input vector
 * @param mask the input mask
 * @param hwidth half-width of the smoothing window
 */
void runningMedian(casa::Vector<casa::Float>& out,
                   casa::Vector<casa::Bool>& outflag,
                   const casa::Vector<casa::Float>& in,
                   const casa::Vector<casa::Bool>& flag,
                   float hwidth);

void polyfit(casa::Vector<casa::Float>& out,
             casa::Vector<casa::Bool>& outmask,
             const casa::Vector<casa::Float>& in,
             const casa::Vector<casa::Bool>& mask,
             float hwidth, int order);

// Generate specified statistic
float statistics(const casa::String& which,
                 const casa::MaskedArray<casa::Float>& data);

// Return a position of min or max value
casa::IPosition minMaxPos(const casa::String& which,
                 const casa::MaskedArray<casa::Float>& data);

// Replace masked value by zero
void replaceMaskByZero(casa::Vector<casa::Float>& data,
                       const casa::Vector<casa::Bool>& mask);

/**
 * Convert casa implementations to stl
 * @param in casa string
 * @return a std vector of std strings
 */
std::vector<std::string> tovectorstring(const casa::Vector<casa::String>& in);

/**
 * convert stl implementations to casa versions
 * @param in
 * @return
 */
casa::Vector<casa::String> toVectorString(const std::vector<std::string>& in);

void doZeroOrderInterpolation(casa::Vector<casa::Float>& data, 
			      std::vector<bool>& mask);

/**
 * RA nomalization: n*2pi rotation if necessary
 **/
void rotateRA( const casa::Vector<casa::Double> &in,
               casa::Vector<casa::Double> &out ) ;
void rotateRA( casa::Vector<casa::Double> &v ) ;

/**
 * tool to record current time stamp
 **/
double gettimeofday_sec() ;

}

#endif
