#ifndef ODE_INL
#define ODE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{
    // -------------------------------------------------------
    // |-- ODE : Ordinary Differential Equations & Solvers --|
    // -------------------------------------------------------
    
    typedef void (*OdeDerivativeFunctionPtr)( const F32 t, const F32* __restrict x, F32* __restrict dxdt );
    
    class C_ODE
    {
    public:
        
        C_ODE( const U32 varCount ) :
            m_VarCount( varCount ),
            m_VarArray( new F32[ varCount ] )
        {
        }
        
        ~C_ODE()
        {
            delete [] m_VarArray;
        }
        
        const U32       VarCount() const        { return m_VarCount; }
        F32*            VarArray() const        { return m_VarArray; }
        
    private:
        
        const U32       m_VarCount;
        F32* const      m_VarArray;
    };
    
    /// Given values for the variables x[0..n-1] and their derivatives dxdt[0..n-1] known at t, use the
    /// fourth-order Runge-Kutta method to advance the solution over an interval deltaT and return the
    /// incremented variables as x[0..n-1]. The user supplies the routine EvalDerivs(t,x,dxdt), which
    /// returns derivatives dxdt at t.
    inline void RungeKutta4(
        F32*                        __restrict x,
        const F32*                  __restrict dxdt,
        const U32                   varCount,
        const F32                   t,
        const F32                   deltaT,
        OdeDerivativeFunctionPtr    EvaluateDerivatives )
    {
        F32* xTemp   = new F32[ varCount ];
        F32* dxTemp1 = new F32[ varCount ];
        F32* dxTemp2 = new F32[ varCount ];
        
        const F32 halfDeltaT = deltaT * 0.5f;
        
        // First step.
        
        for ( U32 i=0; i<varCount; i++ )
        {
            xTemp[i] = x[i] + halfDeltaT * dxdt[i];
        }
        
        // Second step.
        
        (*EvaluateDerivatives)( t + halfDeltaT, xTemp, dxTemp1 );
        
        for ( U32 i=0; i<varCount; i++ )
        {
            xTemp[i] = x[i] + halfDeltaT * dxTemp1[i];
        }
        
        // Third step.
        
        (*EvaluateDerivatives)( t + halfDeltaT, xTemp, dxTemp2 );
        
        for ( U32 i=0; i<varCount; i++ )
        {
            xTemp[i] = x[i] + deltaT * dxTemp2[i];
            dxTemp2[i] += dxTemp1[i];
        }
        
        // Fourth step.
        
        (*EvaluateDerivatives)( t + deltaT, xTemp, dxTemp1 );
        
        const F32 deltaTdiv6 = deltaT / 6.0f;
        
        for ( U32 i=0; i<varCount; i++ )
        {
            // Accumulate increments with proper weights.
            // dxdt    = k1
            // dxTemp2 = k2 + k3
            // dxTemp1 = k4
            
            x[i] = x[i] + ( deltaTdiv6 * ( dxdt[i] + 2.0f * dxTemp2[i] + dxTemp1[i] ) );
        }
        
        delete [] xTemp;
        delete [] dxTemp1;
        delete [] dxTemp2;
    }
    
}; // namespace Maths



#endif
