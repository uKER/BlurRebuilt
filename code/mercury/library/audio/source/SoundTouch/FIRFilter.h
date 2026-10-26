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
// File revision : $Revision: 1.17 $
//
// $Id: FIRFilter.h,v 1.17 2006/02/05 16:44:06 Olli Exp $
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

#ifndef FIRFilter_H
#define FIRFilter_H
//#include "AudioPch.h"
#include "STTypes.h"

namespace soundtouch
{

class FIRFilter 
{
protected:
    // Number of FIR filter taps
    uint length;    
    // Number of FIR filter taps divided by 8
    uint lengthDiv8;

    // Result divider factor in 2^k format
    uint resultDivFactor;

    // Result divider value.
    SAMPLETYPE resultDivider;

    // Memory for filter coefficients
    SAMPLETYPE *filterCoeffs;

    virtual uint evaluateFilterStereo(SAMPLETYPE *dest, 
                                      const SAMPLETYPE *src, 
                                      uint numSamples) const;
    virtual uint evaluateFilterMono(SAMPLETYPE *dest, 
                                    const SAMPLETYPE *src, 
                                    uint numSamples) const;

public:
    FIRFilter();
    virtual ~FIRFilter();

    /// Operator 'new' is overloaded so that it automatically creates a suitable instance 
    /// depending on if we've a MMX-capable CPU available or not.
    void * operator new(size_t s);

    static FIRFilter *newInstance();

    /// Applies the filter to the given sequence of samples. 
    /// Note : The amount of outputted samples is by value of 'filter_length' 
    /// smaller than the amount of input samples.
    ///
    /// \return Number of samples copied to 'dest'.
    uint evaluate(SAMPLETYPE *dest, 
                  const SAMPLETYPE *src, 
                  uint numSamples, 
                  uint numChannels) const;

    uint getLength() const;

    virtual void setCoefficients(const SAMPLETYPE *coeffs, 
                                 uint newLength, 
                                 uint uResultDivFactor);
};


// Optional subclasses that implement CPU-specific optimizations:

#ifdef ALLOW_MMX

    /// Class that implements MMX optimized functions exclusive for 16bit integer samples type.
    class FIRFilterMMX : public FIRFilter
    {
    protected:
        short *filterCoeffsUnalign;
        short *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(short *dest, const short *src, uint numSamples) const;
    public:
        FIRFilterMMX();
        ~FIRFilterMMX();

        virtual void setCoefficients(const short *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif // ALLOW_MMX


#ifdef ALLOW_3DNOW

    /// Class that implements 3DNow! optimized functions exclusive for floating point samples type.
    class FIRFilter3DNow : public FIRFilter
    {
    protected:
        float *filterCoeffsUnalign;
        float *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(float *dest, const float *src, uint numSamples) const;
    public:
        FIRFilter3DNow();
        ~FIRFilter3DNow();
        virtual void setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif  // ALLOW_3DNOW


#ifdef ALLOW_SSE
    /// Class that implements SSE optimized functions exclusive for floating point samples type.
    class FIRFilterSSE : public FIRFilter
    {
    protected:
        float *filterCoeffsUnalign;
        float *filterCoeffsAlign;

        virtual uint evaluateFilterStereo(float *dest, const float *src, uint numSamples) const;
    public:
        FIRFilterSSE();
        ~FIRFilterSSE();

        virtual void setCoefficients(const float *coeffs, uint newLength, uint uResultDivFactor);
    };

#endif // ALLOW_SSE

}

#endif  // FIRFilter_H
