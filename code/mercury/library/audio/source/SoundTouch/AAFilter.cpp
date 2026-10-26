////////////////////////////////////////////////////////////////////////////////
///
/// FIR low-pass (anti-alias) filter with filter coefficient design routine and
/// MMX optimization. 
/// 
/// Anti-alias filter is used to prevent folding of high frequencies when 
/// transposing the sample rate with interpolation.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.9 $
//
// $Id: AAFilter.cpp,v 1.9 2006/02/05 16:44:06 Olli Exp $
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
#include "AAFilter.h"
#include "FIRFilter.h"

using namespace soundtouch;

#define PI        3.141592655357989
#define TWOPI    (2 * PI)

/*****************************************************************************
 *
 * Implementation of the class 'AAFilter'
 *
 *****************************************************************************/

AAFilter::AAFilter(const uint length)
{
    pFIR = FIRFilter::newInstance();
    cutoffFreq = 0.5;
    setLength(length);
}



AAFilter::~AAFilter()
{
    delete pFIR;
}



// Sets new anti-alias filter cut-off edge frequency, scaled to
// sampling frequency (nyquist frequency = 0.5).
// The filter will cut frequencies higher than the given frequency.
void AAFilter::setCutoffFreq(const double newCutoffFreq)
{
    cutoffFreq = newCutoffFreq;
    calculateCoeffs();
}



// Sets number of FIR filter taps
void AAFilter::setLength(const uint newLength)
{
    length = newLength;
    calculateCoeffs();
}



// Calculates coefficients for a low-pass FIR filter using Hamming window
void AAFilter::calculateCoeffs()
{
    uint i;
    double cntTemp, temp, tempCoeff,h, w;
    double fc2, wc;
    double scaleCoeff, sum;
    double *work;
    SAMPLETYPE *coeffs;

    assert(length > 0);
    assert(length % 4 == 0);
    assert(cutoffFreq >= 0);
    assert(cutoffFreq <= 0.5);

    work = new double[length];
    coeffs = new SAMPLETYPE[length];

    fc2 = 2.0 * cutoffFreq; 
    wc = PI * fc2;
    tempCoeff = TWOPI / (double)length;

    sum = 0;
    for (i = 0; i < length; i ++) 
    {
        cntTemp = (double)i - (double)(length / 2);

        temp = cntTemp * wc;
        if (temp != 0) 
        {
            h = fc2 * sin(temp) / temp;                     // sinc function
        } 
        else 
        {
            h = 1.0;
        }
        w = 0.54 + 0.46 * cos(tempCoeff * cntTemp);       // hamming window

        temp = w * h;
        work[i] = temp;

        // calc net sum of coefficients 
        sum += temp;
    }

    // ensure the sum of coefficients is larger than zero
    assert(sum > 0);

    // ensure we've really designed a lowpass filter...
    assert(work[length/2] > 0);
    assert(work[length/2 + 1] > -1e-6);
    assert(work[length/2 - 1] > -1e-6);

    // Calculate a scaling coefficient in such a way that the result can be
    // divided by 16384
    scaleCoeff = 16384.0f / sum;

    for (i = 0; i < length; i ++) 
    {
        // scale & round to nearest integer
        temp = work[i] * scaleCoeff;
        temp += (temp >= 0) ? 0.5 : -0.5;
        // ensure no overfloods
        assert(temp >= -32768 && temp <= 32767);
        coeffs[i] = (SAMPLETYPE)temp;
    }

    // Set coefficients. Use divide factor 14 => divide result by 2^14 = 16384
    pFIR->setCoefficients(coeffs, length, 14);

    delete[] work;
    delete[] coeffs;
}


// Applies the filter to the given sequence of samples. 
// Note : The amount of outputted samples is by value of 'filter length' 
// smaller than the amount of input samples.
uint AAFilter::evaluate(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples, uint numChannels) const
{
    return pFIR->evaluate(dest, src, numSamples, numChannels);
}


uint AAFilter::getLength() const
{
    return pFIR->getLength();
}
