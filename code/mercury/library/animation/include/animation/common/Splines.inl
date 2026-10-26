#ifndef ANIMATION_CORE_SPLINES_INL
#define ANIMATION_CORE_SPLINES_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{

//
// CubicBSpline
//

template <typename TPoint, typename TKnot>
TPoint C_CubicBSpline::Evaluate(const TKnot knots[6], const TPoint controlPoints[4], TKnot t)
{
	TKnot coeffs[4];
	EvaluateCoefficients<TKnot>(knots, t, coeffs);

	return coeffs[0] * controlPoints[0] +
		   coeffs[1] * controlPoints[1] +
		   coeffs[2] * controlPoints[2] +
		   coeffs[3] * controlPoints[3];
}


template <typename TKnot>
void C_CubicBSpline::EvaluateCoefficients(const TKnot knots[6], TKnot t, TKnot outCoeffs[4])
{
    TKnot coeff0 = Maths::Constants<TKnot>::One();
    TKnot coeff1;// = Maths::Constants<TKnot>::Zero();
    TKnot coeff2;// = Maths::Constants<TKnot>::Zero();
    TKnot coeff3;// = Maths::Constants<TKnot>::Zero();

    static const S32 kMidKnot = 2;
    
    TKnot tMinusTk       = t - knots[kMidKnot];
    TKnot tMinusTkMinus1 = t - knots[kMidKnot-1];
    TKnot tMinusTkMinus2 = t - knots[kMidKnot-2];
    
    TKnot tkPlus1MinusT  = knots[kMidKnot+1] - t;
    TKnot tkPlus2MinusT  = knots[kMidKnot+2] - t;
    TKnot tkPlus3MinusT  = knots[kMidKnot+3] - t;

	assert(tMinusTk >= 0.f);
	assert(tMinusTkMinus1 >= 0.f);
	assert(tMinusTkMinus2 >= 0.f);
	assert(tkPlus1MinusT >= 0.f);
	assert(tkPlus2MinusT >= 0.f);
	assert(tkPlus3MinusT >= 0.f);
        
    // p = 1
    {
       TKnot inputCoeff = coeff0 / (tMinusTk + tkPlus1MinusT);
       coeff0 = inputCoeff * tMinusTk;
       coeff1 = inputCoeff * tkPlus1MinusT;
    }
    
    // p = 2
    {
        TKnot prevRightTerm; // = Maths::Constants<TKnot>::Zero();

        // k = 0
        {
            TKnot inputCoeff = coeff0 / (tMinusTk + tkPlus2MinusT);
            coeff0 = inputCoeff * tMinusTk;
            prevRightTerm = inputCoeff * tkPlus2MinusT;
        }
        
        // k = 1
        {
            TKnot inputCoeff = coeff1 / (tMinusTkMinus1 + tkPlus1MinusT);
            coeff1 = prevRightTerm + inputCoeff * tMinusTkMinus1;
            coeff2 = inputCoeff * tkPlus1MinusT;
        }
    }
    
    
    // p = 3
    {
        TKnot prevRightTerm; // = Maths::Constants<TKnot>::Zero();
        
        // k = 0
        {
            TKnot inputCoeff = coeff0 / (tMinusTk + tkPlus3MinusT);
            coeff0 = inputCoeff * tMinusTk;
            prevRightTerm = inputCoeff * tkPlus3MinusT;
        }
        
        // k = 1
        {
            TKnot inputCoeff = coeff1 / (tMinusTkMinus1 + tkPlus2MinusT);
            coeff1 = prevRightTerm + inputCoeff * tMinusTkMinus1;
            prevRightTerm = inputCoeff * tkPlus2MinusT;
        }
        
        // k = 2
        {
            TKnot inputCoeff = coeff2 / (tMinusTkMinus2 + tkPlus1MinusT);
            coeff2 = prevRightTerm + inputCoeff * tMinusTkMinus2;
            coeff3 = inputCoeff * tkPlus1MinusT;
        }
    }
    
	outCoeffs[3] = coeff0;
	outCoeffs[2] = coeff1;
	outCoeffs[1] = coeff2;
	outCoeffs[0] = coeff3;
}

//
// QuadraticBSpline
//

template <typename TPoint, typename TKnot>
TPoint C_QuadraticBSpline::Evaluate(const TKnot knots[4], const TPoint controlPoints[3], TKnot t)
{
	TKnot coeffs[3];
	EvaluateCoefficients(knots, t, coeffs);
	return coeffs[0] * controlPoints[0] +
		   coeffs[1] * controlPoints[1] +
		   coeffs[2] * controlPoints[2];
}


template <typename TKnot>
void C_QuadraticBSpline::EvaluateCoefficients(const TKnot knots[4], TKnot t, TKnot outCoeffs[3])
{
    TKnot coeff0 = Maths::Constants<TKnot>::One();
    TKnot coeff1;// = Maths::Constants<TKnot>::Zero();
    TKnot coeff2;// = Maths::Constants<TKnot>::Zero();

    static const S32 kMidKnot = 1;
    
    TKnot tMinusTk       = t - knots[kMidKnot];
    TKnot tMinusTkMinus1 = t - knots[kMidKnot-1];
    
    TKnot tkPlus1MinusT  = knots[kMidKnot+1] - t;
    TKnot tkPlus2MinusT  = knots[kMidKnot+2] - t;
        
    // p = 1
    {
		TKnot invDenom = Maths::Reciprocal(tMinusTk + tkPlus1MinusT);
		TKnot inputCoeff = coeff0 * invDenom;
		coeff0 = inputCoeff * tMinusTk;
		coeff1 = inputCoeff * tkPlus1MinusT;
    }
    
    // p = 2
    {
        TKnot prevRightTerm; // = Maths::Constants<TKnot>::Zero();

        // k = 0
        {
			TKnot invDenom = Maths::Reciprocal(tMinusTk + tkPlus2MinusT);
            TKnot inputCoeff = coeff0 * invDenom;
            coeff0 = inputCoeff * tMinusTk;
            prevRightTerm = inputCoeff * tkPlus2MinusT;
        }
        
        // k = 1
        {
			TKnot invDenom = Maths::Reciprocal(tMinusTkMinus1 + tkPlus1MinusT);
            TKnot inputCoeff = coeff1 * invDenom;
            coeff1 = prevRightTerm + inputCoeff * tMinusTkMinus1;
            coeff2 = inputCoeff * tkPlus1MinusT;
        }
    }
    
	outCoeffs[2] = coeff0;
	outCoeffs[1] = coeff1;
	outCoeffs[0] = coeff2;
}

//
// LinearBSpline
//

template <typename TPoint, typename TKnot>
TPoint C_LinearBSpline::Evaluate(const TKnot knots[2], const TPoint controlPoints[2], TKnot t)
{
	TKnot coeffs[2];
	EvaluateCoefficients(knots, t, coeffs);
	return coeffs[0] * controlPoints[0] +
		   coeffs[1] * controlPoints[1];
}


template <typename TKnot>
void C_LinearBSpline::EvaluateCoefficients(const TKnot knots[2], TKnot t, TKnot outCoeffs[2])
{
    static const S32 kMidKnot = 0;
    
    TKnot tMinusTk       = t - knots[kMidKnot];
    TKnot tkPlus1MinusT  = knots[kMidKnot+1] - t;
        
    // p = 1
    {
	   TKnot inputCoeff = Maths::Reciprocal(tMinusTk + tkPlus1MinusT);
       outCoeffs[1] = inputCoeff * tMinusTk;
       outCoeffs[0] = inputCoeff * tkPlus1MinusT;
    }
   
}


} // end Animation

#endif

