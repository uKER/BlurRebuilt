////////////////////////////////////////////////////////////////////////////////
///
/// General FIR digital filter routines with MMX optimization. 
///
/// Note : MMX optimized functions reside in a separate, platform-specific file, 
/// e.g. 'mmx_win.cpp' or 'mmx_gcc.cpp'
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.16 $
//
// $Id: FIRFilter.cpp,v 1.16 2006/02/05 16:44:06 Olli Exp $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouchZ audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is licensed for commercial use of Bizarre Creations ltd
//  as defined in the executed license agreement. Please see document
//  "SoundTouchZ-license-Bizarre Creations.pdf" for further details.
//
////////////////////////////////////////////////////////////////////////////////
#include "AudioPch.h"
#if defined(PLATFORM_PS3) && CELL_SDK_VERSION >= 0x240001
#include <sys/memory.h>
#else
#include <memory.h>
#endif
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdexcept>
#include "FIRFilter.h"
//#include "cpu_detect.h"

using namespace soundtouch;

/*****************************************************************************
 *
 * Implementation of the class 'FIRFilter'
 *
 *****************************************************************************/

FIRFilter::FIRFilter()
{
    resultDivFactor = 0;
    length = 0;
    lengthDiv8 = 0;
    filterCoeffs = NULL;
}


FIRFilter::~FIRFilter()
{
    delete[] filterCoeffs;
}

// Usual C-version of the filter routine for stereo sound
uint FIRFilter::evaluateFilterStereo(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples) const
{
    uint i, j, end;
    LONG_SAMPLETYPE suml, sumr;
#ifdef FLOAT_SAMPLES
    // when using floating point samples, use a scaler instead of a divider
    // because division is much slower operation than multiplying.
    double dScaler = 1.0 / (double)resultDivider;
#endif

    assert(length != 0);

    end = 2 * (numSamples - length);

    for (j = 0; j < end; j += 2) 
    {
        const SAMPLETYPE *ptr;

        suml = sumr = 0;
        ptr = src + j;

        for (i = 0; i < length; i += 4) 
        {
            // loop is unrolled by factor of 4 here for efficiency
            suml += ptr[2 * i + 0] * filterCoeffs[i + 0] +
                    ptr[2 * i + 2] * filterCoeffs[i + 1] +
                    ptr[2 * i + 4] * filterCoeffs[i + 2] +
                    ptr[2 * i + 6] * filterCoeffs[i + 3];
            sumr += ptr[2 * i + 1] * filterCoeffs[i + 0] +
                    ptr[2 * i + 3] * filterCoeffs[i + 1] +
                    ptr[2 * i + 5] * filterCoeffs[i + 2] +
                    ptr[2 * i + 7] * filterCoeffs[i + 3];
        }

#ifdef INTEGER_SAMPLES
        suml >>= resultDivFactor;
        sumr >>= resultDivFactor;
        // saturate to 16 bit integer limits
        suml = (suml < -32768) ? -32768 : (suml > 32767) ? 32767 : suml;
        // saturate to 16 bit integer limits
        sumr = (sumr < -32768) ? -32768 : (sumr > 32767) ? 32767 : sumr;
#else
        suml *= dScaler;
        sumr *= dScaler;
#endif // INTEGER_SAMPLES
        dest[j] = (SAMPLETYPE)suml;
        dest[j + 1] = (SAMPLETYPE)sumr;
    }
    return numSamples - length;
}




// Usual C-version of the filter routine for mono sound
uint FIRFilter::evaluateFilterMono(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples) const
{
    uint i, j, end;
    LONG_SAMPLETYPE sum;
#ifdef FLOAT_SAMPLES
    // when using floating point samples, use a scaler instead of a divider
    // because division is much slower operation than multiplying.
    double dScaler = 1.0 / (double)resultDivider;
#endif


    assert(length != 0);

    end = numSamples - length;
    for (j = 0; j < end; j ++) 
    {
        sum = 0;
        for (i = 0; i < length; i += 4) 
        {
            // loop is unrolled by factor of 4 here for efficiency
            sum += src[i + 0] * filterCoeffs[i + 0] + 
                   src[i + 1] * filterCoeffs[i + 1] + 
                   src[i + 2] * filterCoeffs[i + 2] + 
                   src[i + 3] * filterCoeffs[i + 3];
        }
#ifdef INTEGER_SAMPLES
        sum >>= resultDivFactor;
        // saturate to 16 bit integer limits
        sum = (sum < -32768) ? -32768 : (sum > 32767) ? 32767 : sum;
#else
        sum *= dScaler;
#endif // INTEGER_SAMPLES
        dest[j] = (SAMPLETYPE)sum;
        src ++;
    }
    return end;
}


// Set filter coeffiecients and length.
//
// Throws an exception if filter length isn't divisible by 8
void FIRFilter::setCoefficients(const SAMPLETYPE *coeffs, uint newLength, uint uResultDivFactor)
{
    assert(newLength > 0);
// temp, getting ps3 version to compile, throw's should be removed
#ifndef PLATFORM_PS3
    if (newLength % 8) throw std::runtime_error("FIR filter length not divisible by 8");
#endif

    lengthDiv8 = newLength / 8;
    length = lengthDiv8 * 8;
    assert(length == newLength);
	uint power2 = 2;

    resultDivFactor = uResultDivFactor;
    resultDivider = (SAMPLETYPE)pow((double)power2, (int)resultDivFactor);

    delete[] filterCoeffs;
    filterCoeffs = new SAMPLETYPE[length];
    memcpy(filterCoeffs, coeffs, length * sizeof(SAMPLETYPE));
}


uint FIRFilter::getLength() const
{
    return length;
}



// Applies the filter to the given sequence of samples. 
//
// Note : The amount of outputted samples is by value of 'filter_length' 
// smaller than the amount of input samples.
uint FIRFilter::evaluate(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples, uint numChannels) const
{
    assert(numChannels == 1 || numChannels == 2);

    assert(length > 0);
    assert(lengthDiv8 * 8 == length);
    if (numSamples < length) return 0;
    assert(resultDivFactor >= 0);
    if (numChannels == 2) 
    {
        return evaluateFilterStereo(dest, src, numSamples);
    } else {
        return evaluateFilterMono(dest, src, numSamples);
    }
}



// Operator 'new' is overloaded so that it automatically creates a suitable instance 
// depending on if we've a MMX-capable CPU available or not.
void * FIRFilter::operator new(size_t s)
{
    // Notice! don't use "new FIRFilter" directly, use "newInstance" to create a new instance instead!
// temp, getting ps3 version to compile, throw's should be removed
#ifndef PLATFORM_PS3
    throw std::runtime_error("Don't use 'new FIRFilter', use 'newInstance' member instead!");
#endif
    return (void*)0;
}


FIRFilter * FIRFilter::newInstance()
{
    {
        // ISA optimizations not supported, use plain C version
        return ::new FIRFilter;
    }
}
