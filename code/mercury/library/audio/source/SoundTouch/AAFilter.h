////////////////////////////////////////////////////////////////////////////////
///
/// Sampled sound tempo changer/time stretch algorithm. Changes the sound tempo 
/// while maintaining the original pitch by using a time domain WSOLA-like method 
/// with several performance-increasing tweaks.
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
// File revision : $Revision: 1.10 $
//
// $Id: AAFilter.h,v 1.10 2006/02/05 16:44:06 Olli Exp $
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

#ifndef AAFilter_H
#define AAFilter_H

#include "STTypes.h"

namespace soundtouch
{

class AAFilter
{
protected:
    class FIRFilter *pFIR;

    /// Low-pass filter cut-off frequency, negative = invalid
    double cutoffFreq;

    /// num of filter taps
    uint length;

    /// Calculate the FIR coefficients realizing the given cutoff-frequency
    void calculateCoeffs();
public:
    AAFilter(uint length);

    ~AAFilter();

    /// Sets new anti-alias filter cut-off edge frequency, scaled to sampling 
    /// frequency (nyquist frequency = 0.5). The filter will cut off the 
    /// frequencies than that.
    void setCutoffFreq(double newCutoffFreq);

    /// Sets number of FIR filter taps, i.e. ~filter complexity
    void setLength(uint newLength);

    uint getLength() const;

    /// Applies the filter to the given sequence of samples. 
    /// Note : The amount of outputted samples is by value of 'filter length' 
    /// smaller than the amount of input samples.
    uint evaluate(SAMPLETYPE *dest, 
                  const SAMPLETYPE *src, 
                  uint numSamples, 
                  uint numChannels) const;
};

}

#endif
