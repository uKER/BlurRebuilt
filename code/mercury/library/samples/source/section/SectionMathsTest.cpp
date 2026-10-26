#include "StdAfx.h"


#include <Core/Maths.h>
#include "Samples/Section/SectionMathsTest.h"
#include "Render/Render/GridRender.h"
#include <Core/Threads/Thread.h>

using namespace Maths;

namespace Samples
{

// TODO: Implementations of these for SPU, PPU & SSE!
void StartProfiling( const _ePMCSetup setup );
const S64 EndProfiling( PMCState* const __restrict pState, const bool dump = false );



class Interval
{
public:
    Interval( const F64 lo, const F64 hi ) : m_Lo( lo ) , m_Hi( hi ) {}
    const F64 Lo() const { return m_Lo; }
    const F64 Hi() const { return m_Hi; }
    const F64 m_Lo;
    const F64 m_Hi;
};


void Analyse(
    char* functionName,
    const VF32 (*function)( const VF32 PCREF ),
    F64 (*reference)( F64 ),
    const Interval& interval,
    const S32 samples,
    const bool debug = false )
{
    PMCState pmcstate;
    F32 * const __restrict pX = new F32[ samples + 1 ];
    F32 * const __restrict pA = new F32[ samples + 1 ];
    F32 * const __restrict pB = new F32[ samples + 1 ];
    
    F64 peakError = 0.0;
    F64 sumError = 0.0;
    F64 sumSquaredError = 0.0;
    F64 peakErrorRatio = 0.0;
    F64 sumErrorRatio = 0.0;
    F64 sumSquaredErrorRatio = 0.0;
    
    const F64 interval_lo( interval.Lo() );
    const F64 interval_hi( interval.Hi() );
    const F64 interval_size( interval_hi - interval_lo );
    
    LOGINFO( g_Log, "Analysing %s: samples = %d, sample interval = [ %+f, %+f ]\n", functionName, samples + 1, interval_lo, interval_hi );
    
    for ( S32 i = 0; i <= samples; i++ )
    {
        pX[i] = (F32)( interval_lo + ( ( (F64)i * interval_size ) / (F64)samples ) );
    }
    
    StartProfiling( PMC_SETUP_FPU_PB0T0 );
    {
        
        for ( S32 i = 0; i <= samples; i++ )
        {
            
            pA[i] = (F32)(*reference)( pX[i] );
        }
    }
    S64 t_reference = EndProfiling( &pmcstate );
    
    StartProfiling( PMC_SETUP_VMXMIX_PB0T0 );
    {
        
        for ( S32 i = 0; i <= samples; i++ )
        {
            
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pB[i] );
        }
    }
    S64 t_function = EndProfiling( &pmcstate );
    
    for ( S32 i = 0; i <= samples; i++ )
    {
        const F64 x = pX[i];
        const F64 a = pA[i];
        const F64 b = pB[i];
        
        const F64 error = _finite( a ) && _finite( b ) &&       // Ignore infinites/NaN's
                          ( abs( a ) < 10.0 )                   // Ignore abs error of large values, where error ratio is more appropriate
                          ? Maths::Abs( a - b ) : 0.0;
        
        const F64 errorRatio = _finite( a ) && _finite( b ) &&  // Ignore infinites/NaN's
                               ( abs( a ) > 0.0001 ) &&         // Ignore error ratios tending towards infinity due to close-to-zero a
                               ( abs( b ) > 0.0001 )            // Ignore error ratios tending towards 1.0 due to close-to-zero b
                               ? error / abs( a ) : 0.0;
        
        peakError = Maths::Max( peakError, error );
        sumError += error;
        sumSquaredError += error * error;
        peakErrorRatio = Maths::Max( peakErrorRatio, errorRatio );
        sumErrorRatio += errorRatio;
        sumSquaredErrorRatio += errorRatio * errorRatio;
        
        if ( debug )
            LOGINFO( g_Log, "x = %+15.9f, a = %+15.9f, b = %+15.9f, error = %15.9f, error ratio = %15.9f\n", x, a, b, error, errorRatio );
    }
    
    const F64 meanError = sumError / (F64)( samples + 1 );
    const F64 rmsError = sqrt( sumSquaredError / (F64)( samples + 1 ) );
    const F64 meanErrorRatio = sumErrorRatio / (F64)( samples + 1 );
    const F64 rmsErrorRatio = sqrt( sumSquaredErrorRatio / (F64)( samples + 1 ) );
    
    LOGINFO( g_Log, "peak error       = %15.9f\n", peakError );
    LOGINFO( g_Log, "mean error       = %15.9f\n", meanError );
    LOGINFO( g_Log, "rms error        = %15.9f\n", rmsError );
    LOGINFO( g_Log, "peak error ratio = %15.9f\n", peakErrorRatio );
    LOGINFO( g_Log, "mean error ratio = %15.9f\n", meanErrorRatio );
    LOGINFO( g_Log, "rms error ratio  = %15.9f\n", rmsErrorRatio );
    LOGINFO( g_Log, "reference time   = %15.9f\n", (F64)t_reference / (F64)samples );
    LOGINFO( g_Log, "function time    = %15.9f\n", (F64)t_function / (F64)samples );
    LOGINFO( g_Log, "\n" );
    
    delete [] pX;
    delete [] pA;
    delete [] pB;
}


void Analyse(
    char* functionName,
    const VF32 (*function)( const VF32 PCREF, const VF32 PCREF ),
    F64 (*reference)( F64, F64 ),
    const Interval& interval_x,
    const S32 samples_x,
    const Interval& interval_y,
    const S32 samples_y,
    const bool debug = false )
{
    PMCState pmcstate;
    F32 * const __restrict pX = new F32[ samples_x + 1 ];
    F32 * const __restrict pY = new F32[ samples_y + 1 ];
    F32 * const __restrict pA = new F32[ ( samples_x + 1 ) * ( samples_y + 1 ) ];
    F32 * const __restrict pB = new F32[ ( samples_x + 1 ) * ( samples_y + 1 ) ];
    
    F64 peakError = 0.0;
    F64 sumError = 0.0;
    F64 sumSquaredError = 0.0;
    F64 peakErrorRatio = 0.0;
    F64 sumErrorRatio = 0.0;
    F64 sumSquaredErrorRatio = 0.0;
    
    const F64 interval_x_lo( interval_x.Lo() );
    const F64 interval_x_hi( interval_x.Hi() );
    const F64 interval_x_size( interval_x_hi - interval_x_lo );
    
    const F64 interval_y_lo( interval_y.Lo() );
    const F64 interval_y_hi( interval_y.Hi() );
    const F64 interval_y_size( interval_y_hi - interval_y_lo );
    
    LOGINFO( g_Log, "Analysing %s: x samples = %d, x sample interval = [ %+f, %+f ], y samples = %d, y sample interval = [ %+f, %+f ]\n",
        functionName, samples_x + 1, interval_x_lo, interval_x_hi, samples_y + 1, interval_y_lo, interval_y_hi );
    
    for ( S32 i = 0; i <= samples_x; i++ )
    {
        pX[i] = (F32)( interval_x_lo + ( ( (F64)i * interval_x_size ) / (F64)samples_x ) );
    }
    
    for ( S32 i = 0; i <= samples_y; i++ )
    {
        pY[i] = (F32)( interval_y_lo + ( ( (F64)i * interval_y_size ) / (F64)samples_x ) );
    }
    
    StartProfiling( PMC_SETUP_FPU_PB0T0 );
    {
        
        S32 index = 0;
        for ( S32 i = 0; i <= samples_x; i++ )
        {
            const F64 x = pX[i];
            for ( S32 j = 0; j <= samples_y; j++ )
            {
                
                const F64 y = pY[j];
                pA[index++] = (F32)(*reference)( x, y );
            }
            
        }
    }
    S64 t_reference = EndProfiling( &pmcstate );
    
    StartProfiling( PMC_SETUP_VMXMIX_PB0T0 );
    {
        
        S32 index = 0;
        for ( S32 i = 0; i <= samples_x; i++ )
        {
            const VF32 x = Load< VF32 >( &pX[i] );
            for ( S32 j = 0; j <= samples_y; j++ )
            {
                
                const VF32 y = Load< VF32 >( &pY[j] );
                Store( (*function)( x, y ), &pB[index++] );
            }
            
        }
    }
    S64 t_function = EndProfiling( &pmcstate );
    
    S32 index = 0;
    for ( S32 i = 0; i <= samples_x; i++ )
    {
        const F64 x = pX[i];
        for ( S32 j = 0; j <= samples_y; j++ )
        {
            const F64 y = pY[j];
            const F64 a = pA[index];
            const F64 b = pB[index++];
            
            const F64 error = _finite( a ) && _finite( b ) &&       // Ignore infinites/NaN's
                              ( abs( a ) < 10.0 )                   // Ignore abs error of large values, where error ratio is more appropriate
                              ? Maths::Abs( a - b ) : 0.0;
            
            const F64 errorRatio = _finite( a ) && _finite( b ) &&  // Ignore infinites/NaN's
                                   ( abs( a ) > 0.0001 ) &&         // Ignore error ratios tending towards infinity due to close-to-zero a
                                   ( abs( b ) > 0.0001 )            // Ignore error ratios tending towards 1.0 due to close-to-zero b
                                   ? error / abs( a ) : 0.0;
            
            peakError = Maths::Max( peakError, error );
            sumError += error;
            sumSquaredError += error * error;
            peakErrorRatio = Maths::Max( peakErrorRatio, errorRatio );
            sumErrorRatio += errorRatio;
            sumSquaredErrorRatio += errorRatio * errorRatio;
            
            if ( debug )
                LOGINFO( g_Log, "x = %+15.9f, y = %+15.9f, a = %+15.9f, b = %+15.9f, error = %15.9f, error ratio = %15.9f\n", x, y, a, b, error, errorRatio );
        }
    }
    
    const F64 samples = (F64)( samples_x + 1 ) * (F64)( samples_y + 1 );
    const F64 meanError = sumError / (F64)samples;
    const F64 rmsError = sqrt( sumSquaredError / (F64)samples );
    const F64 meanErrorRatio = sumErrorRatio / (F64)samples;
    const F64 rmsErrorRatio = sqrt( sumSquaredErrorRatio / (F64)samples );
    
    LOGINFO( g_Log, "peak error       = %15.9f\n", peakError );
    LOGINFO( g_Log, "mean error       = %15.9f\n", meanError );
    LOGINFO( g_Log, "rms error        = %15.9f\n", rmsError );
    LOGINFO( g_Log, "peak error ratio = %15.9f\n", peakErrorRatio );
    LOGINFO( g_Log, "mean error ratio = %15.9f\n", meanErrorRatio );
    LOGINFO( g_Log, "rms error ratio  = %15.9f\n", rmsErrorRatio );
    LOGINFO( g_Log, "reference time   = %15.9f\n", (F64)t_reference / (F64)samples );
    LOGINFO( g_Log, "function time    = %15.9f\n", (F64)t_function / (F64)samples );
    LOGINFO( g_Log, "\n" );
    
    delete [] pX;
    delete [] pY;
    delete [] pA;
    delete [] pB;
}


void Analyse(
    char* functionName,
    void (*function)( const VF32 PCREF, VF32* const __restrict, VF32* const __restrict ),
    void (*reference)( F64, F64*, F64* ),
    const Interval& interval,
    const S32 samples,
    const bool debug = false )
{
    PMCState pmcstate;
    F32 * const __restrict pX = new F32[ samples + 1 ];
    F32 * const __restrict pA1 = new F32[ samples + 1 ];
    F32 * const __restrict pA2 = new F32[ samples + 1 ];
    F32 * const __restrict pB1 = new F32[ samples + 1 ];
    F32 * const __restrict pB2 = new F32[ samples + 1 ];
    
    F64 peakError1 = 0.0;
    F64 peakError2 = 0.0;
    F64 sumError1 = 0.0;
    F64 sumError2 = 0.0;
    F64 sumSquaredError1 = 0.0;
    F64 sumSquaredError2 = 0.0;
    F64 peakErrorRatio1 = 0.0;
    F64 peakErrorRatio2 = 0.0;
    F64 sumErrorRatio1 = 0.0;
    F64 sumErrorRatio2 = 0.0;
    F64 sumSquaredErrorRatio1 = 0.0;
    F64 sumSquaredErrorRatio2 = 0.0;
    
    const F64 interval_lo( interval.Lo() );
    const F64 interval_hi( interval.Hi() );
    const F64 interval_size( interval_hi - interval_lo );
    
    LOGINFO( g_Log, "Analysing %s: samples = %d, sample interval = [ %+f, %+f ]\n", functionName, samples + 1, interval_lo, interval_hi );
    
    for ( S32 i = 0; i <= samples; i++ )
    {
        pX[i] = (F32)( interval_lo + ( ( (F64)i * interval_size ) / (F64)samples ) );
    }
    
    StartProfiling( PMC_SETUP_FPU_PB0T0 );
    {
        
        for ( S32 i = 0; i <= samples; i++ )
        {
            
            F64 result1;
            F64 result2;
            (*reference)( pX[i], &result1, &result2 );
            pA1[i] = (F32)result1;
            pA2[i] = (F32)result2;
        }
    }
    S64 t_reference = EndProfiling( &pmcstate );
    
    StartProfiling( PMC_SETUP_VMXMIX_PB0T0 );
    {
        
        for ( S32 i = 0; i <= samples; i++ )
        {
            
            VF32 result1;
            VF32 result2;
            (*function)( Load< VF32 >( &pX[i] ), &result1, &result2 );
            Store( result1, &pB1[i] );
            Store( result2, &pB2[i] );
        }
    }
    S64 t_function = EndProfiling( &pmcstate );
    
    for ( S32 i = 0; i <= samples; i++ )
    {
        const F64 x = pX[i];
        const F64 a1 = pA1[i];
        const F64 a2 = pA2[i];
        const F64 b1 = pB1[i];
        const F64 b2 = pB2[i];
        
        const F64 error1 = _finite( a1 ) && _finite( b1 ) &&        // Ignore infinites/NaN's
                           ( abs( a1 ) < 10.0 )                     // Ignore abs error of large values, where error ratio is more appropriate
                           ? Maths::Abs( a1 - b1 ) : 0.0;
        
        const F64 error2 = _finite( a2 ) && _finite( b2 ) &&        // Ignore infinites/NaN's
                           ( abs( a2 ) < 10.0 )                     // Ignore abs error of large values, where error ratio is more appropriate
                           ? Maths::Abs( a2 - b2 ) : 0.0;
        
        const F64 errorRatio1 = _finite( a1 ) && _finite( b1 ) &&   // Ignore infinites/NaN's
                                ( abs( a1 ) > 0.0001 ) &&           // Ignore error ratios tending towards infinity due to close-to-zero a
                                ( abs( b1 ) > 0.0001 )              // Ignore error ratios tending towards 1.0 due to close-to-zero b
                                ? error1 / abs( a1 ) : 0.0;
        
        const F64 errorRatio2 = _finite( a2 ) && _finite( b2 ) &&   // Ignore infinites/NaN's
                                ( abs( a2 ) > 0.0001 ) &&           // Ignore error ratios tending towards infinity due to close-to-zero a
                                ( abs( b2 ) > 0.0001 )              // Ignore error ratios tending towards 1.0 due to close-to-zero b
                                ? error2 / abs( a2 ) : 0.0;
        
        peakError1 = Maths::Max( peakError1, error1 );
        peakError2 = Maths::Max( peakError2, error2 );
        sumError1 += error1;
        sumError2 += error2;
        sumSquaredError1 += error1 * error1;
        sumSquaredError2 += error2 * error2;
        peakErrorRatio1 = Maths::Max( peakErrorRatio1, errorRatio1 );
        peakErrorRatio2 = Maths::Max( peakErrorRatio2, errorRatio2 );
        sumErrorRatio1 += errorRatio1;
        sumErrorRatio2 += errorRatio2;
        sumSquaredErrorRatio1 += errorRatio1 * errorRatio1;
        sumSquaredErrorRatio2 += errorRatio2 * errorRatio2;
        
        if ( debug )
        {
            LOGINFO( g_Log, "x1 = %+15.9f, a1 = %+15.9f, b1 = %+15.9f, error1 = %15.9f, error ratio1 = %15.9f\n", x, a1, b1, error1, errorRatio1 );
            LOGINFO( g_Log, "x2 = %+15.9f, a2 = %+15.9f, b2 = %+15.9f, error2 = %15.9f, error ratio2 = %15.9f\n", x, a2, b2, error2, errorRatio2 );
        }
    }
    
    const F64 meanError1 = sumError1 / (F64)( samples + 1 );
    const F64 meanError2 = sumError2 / (F64)( samples + 1 );
    const F64 rmsError1 = sqrt( sumSquaredError1 / (F64)( samples + 1 ) );
    const F64 rmsError2 = sqrt( sumSquaredError2 / (F64)( samples + 1 ) );
    const F64 meanErrorRatio1 = sumErrorRatio1 / (F64)( samples + 1 );
    const F64 meanErrorRatio2 = sumErrorRatio2 / (F64)( samples + 1 );
    const F64 rmsErrorRatio1 = sqrt( sumSquaredErrorRatio1 / (F64)( samples + 1 ) );
    const F64 rmsErrorRatio2 = sqrt( sumSquaredErrorRatio2 / (F64)( samples + 1 ) );
    
    LOGINFO( g_Log, "peak error1       = %15.9f\n", peakError1 );
    LOGINFO( g_Log, "peak error2       = %15.9f\n", peakError2 );
    LOGINFO( g_Log, "mean error1       = %15.9f\n", meanError1 );
    LOGINFO( g_Log, "mean error2       = %15.9f\n", meanError2 );
    LOGINFO( g_Log, "rms error1        = %15.9f\n", rmsError1 );
    LOGINFO( g_Log, "rms error2        = %15.9f\n", rmsError2 );
    LOGINFO( g_Log, "peak error ratio1 = %15.9f\n", peakErrorRatio1 );
    LOGINFO( g_Log, "peak error ratio2 = %15.9f\n", peakErrorRatio2 );
    LOGINFO( g_Log, "mean error ratio1 = %15.9f\n", meanErrorRatio1 );
    LOGINFO( g_Log, "mean error ratio2 = %15.9f\n", meanErrorRatio2 );
    LOGINFO( g_Log, "rms error ratio1  = %15.9f\n", rmsErrorRatio1 );
    LOGINFO( g_Log, "rms error ratio2  = %15.9f\n", rmsErrorRatio2 );
    LOGINFO( g_Log, "reference time    = %15.9f\n", (F64)t_reference / (F64)samples );
    LOGINFO( g_Log, "function time     = %15.9f\n", (F64)t_function / (F64)samples );
    LOGINFO( g_Log, "\n" );
    
    delete [] pX;
    delete [] pA1;
    delete [] pA2;
    delete [] pB1;
    delete [] pB2;
}


void Profile(
    char* functionName,
    const VF32 (*function)( const VF32 PCREF ),
    const Interval& interval,
    const S32 samples,
    const bool debug = false )
{
    PMCState pmcstate;
    const S32 sample_size = ( ( samples + 31 ) & ~31 ) * sizeof( F32 );
    F32 * const __restrict pX = (F32*)_aligned_malloc( sample_size + 128, 128 );
    F32 * const __restrict pA = (F32*)_aligned_malloc( sample_size + 128, 128 );
    
    const F64 interval_lo( interval.Lo() );
    const F64 interval_hi( interval.Hi() );
    const F64 interval_size( interval_hi - interval_lo );
    
    LOGINFO( g_Log, "Profiling %s: samples = %d, sample interval = [ %+f, %+f ]\n", functionName, samples, interval_lo, interval_hi );
    
    // Generate input arguments
    
    for ( S32 i = 0; i < samples; i++ )
    {
        pX[i] = (F32)( interval_lo + ( ( (F64)i * interval_size ) / (F64)samples ) );
    }
    
    // Warmup
    
    StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    S64 t_warmup = EndProfiling( &pmcstate );
    
    // Profile PMC_SETUP_OVERVIEW_PB0T0
    
    StartProfiling( PMC_SETUP_OVERVIEW_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile1 = EndProfiling( &pmcstate /*, true */ );
    
    const U64 count_l1_i_cache_miss_cycles  = pmcstate.pmc[0];
    const U64 count_pipeline_flush          = pmcstate.pmc[1];
    const U64 count_is2_stall_cycles        = pmcstate.pmc[2];
    const U64 count_is2_depend_stall_cycles = pmcstate.pmc[3];
    const U64 count_instr_commit            = pmcstate.pmc[4];
    const U64 count_l1_d_cache_misses       = pmcstate.pmc[5];
    const U64 count_load_hit_stores         = pmcstate.pmc[6];
    const U64 count_d_erat_misses           = pmcstate.pmc[7];
    const U64 count_c0_l2_data_misses       = pmcstate.pmc[8];
    const U64 count_c0_l2_data_hits         = pmcstate.pmc[9];
    const U64 count_c1_l2_data_hits         = pmcstate.pmc[10];
    const U64 count_c2_l2_data_hits         = pmcstate.pmc[11];
    const U64 count_all_rc_busy_cycles      = pmcstate.pmc[12];
    const U64 count_rc_dispatches           = pmcstate.pmc[13];
    const U64 count_c1_l2_data_misses       = pmcstate.pmc[14];
    const U64 count_c2_l2_data_misses       = pmcstate.pmc[15];
    
    // Profile PMC_SETUP_OVERVIEW_PB0
    
    StartProfiling( PMC_SETUP_OVERVIEW_PB0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile2 = EndProfiling( &pmcstate );
    
    const U64 count_t0_instr_commit           = pmcstate.pmc[0];
    const U64 count_t0_l1_i_cache_miss_cycles = pmcstate.pmc[1];
    const U64 count_t0_l1_d_cache_misses      = pmcstate.pmc[2];
    const U64 count_t0_pipeline_flush         = pmcstate.pmc[3];
    const U64 count_t1_instr_commit           = pmcstate.pmc[4];
    const U64 count_t1_l1_i_cache_miss_cycles = pmcstate.pmc[5];
    const U64 count_t1_l1_d_cache_misses      = pmcstate.pmc[6];
    const U64 count_t1_pipeline_flush         = pmcstate.pmc[7];
    
    // Profile PMC_SETUP_INSTRUCTIONMIX_PB0T0
    
    StartProfiling( PMC_SETUP_INSTRUCTIONMIX_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile3 = EndProfiling( &pmcstate );
    
    const U64 count_branch_commit     = pmcstate.pmc[0];
    const U64 count_branch_flushes    = pmcstate.pmc[1];
    const U64 count_int_instr_commit  = pmcstate.pmc[2];
    const U64 count_fp_instr_commit   = pmcstate.pmc[3];
    const U64 count_l2_store_requests = pmcstate.pmc[4];
    const U64 count_loads             = pmcstate.pmc[5];
    
    // Profile PMC_SETUP_VMXMIX_PB0T0
    
    StartProfiling( PMC_SETUP_VMXMIX_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile4 = EndProfiling( &pmcstate );
    
    const U64 count_viq_stall_cycles    = pmcstate.pmc[1];
    const U64 count_vmx_simple_commit   = pmcstate.pmc[2];
    const U64 count_vmx_float_commit    = pmcstate.pmc[3];
    const U64 count_vmx_permute_commit  = pmcstate.pmc[4];
    const U64 count_vmx_load_commit     = pmcstate.pmc[5];
    const U64 count_vmx_store_commit    = pmcstate.pmc[6];
    const U64 count_vmx_estimate_commit = pmcstate.pmc[7];
    
    // Profile PMC_SETUP_VMXFPU_PB0T0
    
    StartProfiling( PMC_SETUP_VMXFPU_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile5 = EndProfiling( &pmcstate );
    
    const U64 count_viq_depend_stall_cycles = pmcstate.pmc[1];
    const U64 count_viq_other_stall_cycles  = pmcstate.pmc[2];
    const U64 count_viq_div_sqrt_stalls     = pmcstate.pmc[3];
    const U64 count_viq_denorm_stalls       = pmcstate.pmc[4];
    const U64 count_viq_nan_stalls          = pmcstate.pmc[5];
    const U64 count_viq_dot_products        = pmcstate.pmc[6];
    const U64 count_vmx_abnormal_results    = pmcstate.pmc[7];
    
    // Profile PMC_SETUP_FPU_PB0T0
    
    StartProfiling( PMC_SETUP_FPU_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile6 = EndProfiling( &pmcstate );
    
    const U64 count_fp_load_commit = pmcstate.pmc[6];
    
    // Profile PMC_SETUP_FLUSHREASONS_PB0T0
    
    StartProfiling( PMC_SETUP_FLUSHREASONS_PB0T0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile7 = EndProfiling( &pmcstate );
    
    const U64 count_i_erat_misses            = pmcstate.pmc[0];
    const U64 count_microcode_instr_commit   = pmcstate.pmc[2];
    const U64 count_misaligned_flushes       = pmcstate.pmc[4];
    const U64 count_store_queue_full_flushes = pmcstate.pmc[5];
    
    // Profile PMC_SETUP_PREFETCH
    
    StartProfiling( PMC_SETUP_PREFETCH );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile8 = EndProfiling( &pmcstate );
    
    const U64 count_c0_dbct_l2_hits    = pmcstate.pmc[1];
    const U64 count_c1_dbct_l2_hits    = pmcstate.pmc[2];
    const U64 count_c2_dbct_l2_hits    = pmcstate.pmc[3];
    const U64 count_c0_t0_dcbt_sends   = pmcstate.pmc[4];
    const U64 count_c0_t0_dcbt_l1_hits = pmcstate.pmc[5];
    const U64 count_c0_t1_dcbt_sends   = pmcstate.pmc[6];
    const U64 count_c0_t1_dcbt_l1_hits = pmcstate.pmc[7];
    const U64 count_c1_t0_dcbt_sends   = pmcstate.pmc[8];
    const U64 count_c1_t0_dcbt_l1_hits = pmcstate.pmc[9];
    const U64 count_c1_t1_dcbt_sends   = pmcstate.pmc[10];
    const U64 count_c1_t1_dcbt_l1_hits = pmcstate.pmc[11];
    const U64 count_c2_t0_dcbt_sends   = pmcstate.pmc[12];
    const U64 count_c2_t0_dcbt_l1_hits = pmcstate.pmc[13];
    const U64 count_c2_t1_dcbt_sends   = pmcstate.pmc[14];
    const U64 count_c2_t1_dcbt_l1_hits = pmcstate.pmc[15];
    
    // Profile PMC_SETUP_MEMORY_LATENCY_PB0
    
    StartProfiling( PMC_SETUP_MEMORY_LATENCY_PB0 );
    {
        for ( S32 i = 0; i < samples; i++ )
        {
            Store( (*function)( Load< VF32 >( &pX[i] ) ), &pA[i] );
        }
    }
    const S64 t_profile9 = EndProfiling( &pmcstate );
    
    const U64 count_c0_l2_store_miss_no_dcbz  = pmcstate.pmc[0];
    const U64 count_c0_l2_store_miss          = pmcstate.pmc[1];
    const U64 count_c0_l2_load_miss           = pmcstate.pmc[2];
    const U64 count_c0_l2_load_miss_no_ifetch = pmcstate.pmc[3];
    const U64 count_c0_l2_i_miss              = pmcstate.pmc[8];
    const U64 count_c0_l2_i_miss_prefetch     = pmcstate.pmc[9];
    const U64 count_c0_l2_dcbt_miss           = pmcstate.pmc[10];
    const U64 count_c0_l2_dcbt_miss2          = pmcstate.pmc[11];
    
    // Output
    
    const F64 averageProfileTime( ( (F64)t_profile1 + (F64)t_profile2 + (F64)t_profile3 + 
                                    (F64)t_profile4 + (F64)t_profile5 + (F64)t_profile6 +
                                    (F64)t_profile7 + (F64)t_profile8 + (F64)t_profile9 ) / ( (F64)samples * 9.0 ) );
    
    LOGINFO( g_Log, "warmup time               = %15.9f\n", (F64)t_warmup / (F64)samples );
    LOGINFO( g_Log, "profile 1 time            = %15.9f\n", (F64)t_profile1 / (F64)samples );
    LOGINFO( g_Log, "profile 2 time            = %15.9f\n", (F64)t_profile2 / (F64)samples );
    LOGINFO( g_Log, "profile 3 time            = %15.9f\n", (F64)t_profile3 / (F64)samples );
    LOGINFO( g_Log, "profile 4 time            = %15.9f\n", (F64)t_profile4 / (F64)samples );
    LOGINFO( g_Log, "profile 5 time            = %15.9f\n", (F64)t_profile5 / (F64)samples );
    LOGINFO( g_Log, "profile 6 time            = %15.9f\n", (F64)t_profile6 / (F64)samples );
    LOGINFO( g_Log, "profile 7 time            = %15.9f\n", (F64)t_profile7 / (F64)samples );
    LOGINFO( g_Log, "profile 8 time            = %15.9f\n", (F64)t_profile8 / (F64)samples );
    LOGINFO( g_Log, "profile 9 time            = %15.9f\n", (F64)t_profile9 / (F64)samples );
    LOGINFO( g_Log, "average profile time      = %15.9f\n", averageProfileTime );
    LOGINFO( g_Log, "instructions per cycle    = %9.3f\n", (F64)count_instr_commit / (F64)t_profile1 );
    LOGINFO( g_Log, "t0 instructions per cycle = %9.3f\n", (F64)count_t0_instr_commit / (F64)t_profile2 );
    LOGINFO( g_Log, "t1 instructions per cycle = %9.3f\n", (F64)count_t1_instr_commit / (F64)t_profile2 );
    LOGINFO( g_Log, "\n" );
    
    if ( 0 )
    {
        LOGINFO( g_Log, "c0_l2_data_hits           = %10ld\n", count_c0_l2_data_hits    );
        LOGINFO( g_Log, "c0_l2_data_misses         = %10ld\n", count_c0_l2_data_misses  );
        LOGINFO( g_Log, "c1_l2_data_hits           = %10ld\n", count_c1_l2_data_hits    );
        LOGINFO( g_Log, "c1_l2_data_misses         = %10ld\n", count_c1_l2_data_misses  );
        LOGINFO( g_Log, "c2_l2_data_hits           = %10ld\n", count_c2_l2_data_hits    );
        LOGINFO( g_Log, "c2_l2_data_misses         = %10ld\n", count_c2_l2_data_misses  );
        LOGINFO( g_Log, "rc_dispatches             = %10ld\n", count_rc_dispatches      );
        LOGINFO( g_Log, "all_rc_busy_cycles        = %10ld\n", count_all_rc_busy_cycles );
        LOGINFO( g_Log, "\n" );
    }
    
    if ( 1 )
    {
        LOGINFO( g_Log, "t0_instr_commit           = %10ld\n", count_t0_instr_commit           );
        LOGINFO( g_Log, "t0_l1_i_cache_miss_cycles = %10ld\n", count_t0_l1_i_cache_miss_cycles );
        LOGINFO( g_Log, "t0_l1_d_cache_misses      = %10ld\n", count_t0_l1_d_cache_misses      );
        LOGINFO( g_Log, "t0_pipeline_flush         = %10ld\n", count_t0_pipeline_flush         );
        LOGINFO( g_Log, "t1_instr_commit           = %10ld\n", count_t1_instr_commit           );
        LOGINFO( g_Log, "t1_l1_i_cache_miss_cycles = %10ld\n", count_t1_l1_i_cache_miss_cycles );
        LOGINFO( g_Log, "t1_l1_d_cache_misses      = %10ld\n", count_t1_l1_d_cache_misses      );
        LOGINFO( g_Log, "t1_pipeline_flush         = %10ld\n", count_t1_pipeline_flush         );
        LOGINFO( g_Log, "\n" );
    }
    
    LOGINFO( g_Log, "instr_commit              = %10ld\n", count_instr_commit           );
    LOGINFO( g_Log, "branch_commit             = %10ld\n", count_branch_commit          );
    LOGINFO( g_Log, "int_instr_commit          = %10ld\n", count_int_instr_commit       );
    LOGINFO( g_Log, "fp_instr_commit           = %10ld\n", count_fp_instr_commit        );
    LOGINFO( g_Log, "fp_load_commit            = %10ld\n", count_fp_load_commit         );
    LOGINFO( g_Log, "microcode_instr_commit    = %10ld\n", count_microcode_instr_commit );
    LOGINFO( g_Log, "l1_i_cache_miss_cycles    = %10ld\n", count_l1_i_cache_miss_cycles );
    LOGINFO( g_Log, "i_erat_misses             = %10ld\n", count_i_erat_misses          );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "loads                     = %10ld\n", count_loads              );
    LOGINFO( g_Log, "l1_d_cache_misses         = %10ld\n", count_l1_d_cache_misses  );
    LOGINFO( g_Log, "c0_l2_data_hits           = %10ld\n", count_c0_l2_data_hits    );
    LOGINFO( g_Log, "c0_l2_data_misses         = %10ld\n", count_c0_l2_data_misses  );
    LOGINFO( g_Log, "l2_store_requests         = %10ld\n", count_l2_store_requests  );
    LOGINFO( g_Log, "d_erat_misses             = %10ld\n", count_d_erat_misses      );
    LOGINFO( g_Log, "rc_dispatches             = %10ld\n", count_rc_dispatches      );
    LOGINFO( g_Log, "all_rc_busy_cycles        = %10ld\n", count_all_rc_busy_cycles );
    LOGINFO( g_Log, "c0_dcbt_sends             = %10ld\n", count_c0_t0_dcbt_sends   );
    LOGINFO( g_Log, "c0_dcbt_l1_hits           = %10ld\n", count_c0_t0_dcbt_l1_hits );
    LOGINFO( g_Log, "c0_dbct_l2_hits           = %10ld\n", count_c0_dbct_l2_hits    );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "is2_stall_cycles          = %10ld\n", count_is2_stall_cycles         );
    LOGINFO( g_Log, "is2_depend_stall_cycles   = %10ld\n", count_is2_depend_stall_cycles  );
    LOGINFO( g_Log, "load_hit_stores           = %10ld\n", count_load_hit_stores          );
    LOGINFO( g_Log, "pipeline_flush            = %10ld\n", count_pipeline_flush           );
    LOGINFO( g_Log, "branch_flushes            = %10ld\n", count_branch_flushes           );
    LOGINFO( g_Log, "misaligned_flushes        = %10ld\n", count_misaligned_flushes       );
    LOGINFO( g_Log, "store_queue_full_flushes  = %10ld\n", count_store_queue_full_flushes );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "viq_stall_cycles          = %10ld\n", count_viq_stall_cycles        );
    LOGINFO( g_Log, "viq_depend_stall_cycles   = %10ld\n", count_viq_depend_stall_cycles );
    LOGINFO( g_Log, "viq_other_stall_cycles    = %10ld\n", count_viq_other_stall_cycles  );
    LOGINFO( g_Log, "viq_div_sqrt_stalls       = %10ld\n", count_viq_div_sqrt_stalls     );
    LOGINFO( g_Log, "viq_denorm_stalls         = %10ld\n", count_viq_denorm_stalls       );
    LOGINFO( g_Log, "viq_nan_stalls            = %10ld\n", count_viq_nan_stalls          );
    LOGINFO( g_Log, "viq_dot_products          = %10ld\n", count_viq_dot_products        );
    LOGINFO( g_Log, "vmx_simple_commit         = %10ld\n", count_vmx_simple_commit       );
    LOGINFO( g_Log, "vmx_float_commit          = %10ld\n", count_vmx_float_commit        );
    LOGINFO( g_Log, "vmx_permute_commit        = %10ld\n", count_vmx_permute_commit      );
    LOGINFO( g_Log, "vmx_load_commit           = %10ld\n", count_vmx_load_commit         );
    LOGINFO( g_Log, "vmx_store_commit          = %10ld\n", count_vmx_store_commit        );
    LOGINFO( g_Log, "vmx_estimate_commit       = %10ld\n", count_vmx_estimate_commit     );
    LOGINFO( g_Log, "vmx_abnormal_results      = %10ld\n", count_vmx_abnormal_results    );
    LOGINFO( g_Log, "\n" );
    
    LOGINFO( g_Log, "c0_l2_store_miss          = %10ld\n", count_c0_l2_store_miss          );
    LOGINFO( g_Log, "c0_l2_store_miss_no_dcbz  = %10ld\n", count_c0_l2_store_miss_no_dcbz  );
    LOGINFO( g_Log, "c0_l2_load_miss           = %10ld\n", count_c0_l2_load_miss           );
    LOGINFO( g_Log, "c0_l2_load_miss_no_ifetch = %10ld\n", count_c0_l2_load_miss_no_ifetch );
    LOGINFO( g_Log, "c0_l2_i_miss              = %10ld\n", count_c0_l2_i_miss              );
    LOGINFO( g_Log, "c0_l2_i_miss_prefetch     = %10ld\n", count_c0_l2_i_miss_prefetch     );
    LOGINFO( g_Log, "c0_l2_dcbt_miss           = %10ld\n", count_c0_l2_dcbt_miss           );
    LOGINFO( g_Log, "c0_l2_dcbt_miss2          = %10ld\n", count_c0_l2_dcbt_miss2          );
    LOGINFO( g_Log, "\n" );
    
    if ( 0 )
    {
        LOGINFO( g_Log, "c0_t0_dcbt_sends          = %10ld\n", count_c0_t0_dcbt_sends   );
        LOGINFO( g_Log, "c0_t1_dcbt_sends          = %10ld\n", count_c0_t1_dcbt_sends   );
        LOGINFO( g_Log, "c0_t0_dcbt_l1_hits        = %10ld\n", count_c0_t0_dcbt_l1_hits );
        LOGINFO( g_Log, "c0_t1_dcbt_l1_hits        = %10ld\n", count_c0_t1_dcbt_l1_hits );
        LOGINFO( g_Log, "c0_dbct_l2_hits           = %10ld\n", count_c0_dbct_l2_hits    );
        LOGINFO( g_Log, "\n" );
        
        LOGINFO( g_Log, "c1_t0_dcbt_sends          = %10ld\n", count_c1_t0_dcbt_sends   );
        LOGINFO( g_Log, "c1_t1_dcbt_sends          = %10ld\n", count_c1_t1_dcbt_sends   );
        LOGINFO( g_Log, "c1_t0_dcbt_l1_hits        = %10ld\n", count_c1_t0_dcbt_l1_hits );
        LOGINFO( g_Log, "c1_t1_dcbt_l1_hits        = %10ld\n", count_c1_t1_dcbt_l1_hits );
        LOGINFO( g_Log, "c1_dbct_l2_hits           = %10ld\n", count_c1_dbct_l2_hits    );
        LOGINFO( g_Log, "\n" );
        
        LOGINFO( g_Log, "c2_t0_dcbt_sends          = %10ld\n", count_c2_t0_dcbt_sends   );
        LOGINFO( g_Log, "c2_t1_dcbt_sends          = %10ld\n", count_c2_t1_dcbt_sends   );
        LOGINFO( g_Log, "c2_t0_dcbt_l1_hits        = %10ld\n", count_c2_t0_dcbt_l1_hits );
        LOGINFO( g_Log, "c2_t1_dcbt_l1_hits        = %10ld\n", count_c2_t1_dcbt_l1_hits );
        LOGINFO( g_Log, "c2_dbct_l2_hits           = %10ld\n", count_c2_dbct_l2_hits    );
        LOGINFO( g_Log, "\n" );
    }
    
    _aligned_free( pX );
    _aligned_free( pA );
}


class IdleThread : public MT::Thread
{
public:
    
    IdleThread( const S32 cpu )
        : m_Cpu( cpu )
        , m_Terminate( false )
    {
        Start();
    }
    
    ~IdleThread()
    {
        m_Terminate = true;
        Join();
    }
    
    virtual void ThreadMain()
    {
        
        IdleThread* pThis = this;
        
        const S32 zero = 0;
        while ( !m_Terminate )
        {
        }
    }
    
    S32             m_Cpu;
    volatile bool   m_Terminate;
};


F64 RcpRef( F64 x )         { return 1.0 / x; }
F64 DivRef( F64 x, F64 y )  { return x / y; }
F64 RcpSqrtRef( F64 x )     { return 1.0 / sqrt( x ); }

void sincos( F64 x, F64* pSin, F64* pCos ) { *pSin = sin( x ); *pCos = cos( x ); }

void NewMathsTest()
{
    IdleThread thread1( 1 );
    
    //SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
    
    const F64 PI( 3.1415926535897932384626433832795 );
    
    for ( F32 x=1.0f; x!=0.0f; x*=0.5f )
    {
        const F32 rcp_x = GetF32( Rcp( MakeVF32( x ) ) );
        const F32 rcp_sqrt_x = GetF32( RcpSqrt( MakeVF32( x ) ) );
        if ( !_finite( rcp_x ) )
            __debugbreak();
        if ( !_finite( rcp_sqrt_x ) )
            __debugbreak();
    }
    
    for ( F32 x=1.0f; _finite(x); x*=2.0f )
    {
        const F32 rcp_x = GetF32( Rcp( MakeVF32( x ) ) );
        const F32 rcp_sqrt_x = GetF32( RcpSqrt( MakeVF32( x ) ) );
        if ( !_finite( rcp_x ) )
            __debugbreak();
        if ( !_finite( rcp_sqrt_x ) )
            __debugbreak();
    }
    
    Analyse( "Rcp", &Rcp, &RcpRef, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Div", &Div, &DivRef, Interval( -10.0, 10.0 ), 100, Interval( -10.0, 10.0 ), 1000 );
    
    Analyse( "RcpSqrt", &RcpSqrt, &RcpSqrtRef, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Sqrt", &Sqrt, &sqrt, Interval( -10.0, 10.0 ), 100000 );
    
    Analyse( "Sin", &Sin, &sin, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    Analyse( "Cos", &Cos, &cos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    Analyse( "Tan", &Tan, &tan, Interval( -PI, PI ), 100000 );
    Analyse( "SinCos", &SinCos, &sincos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    
    Analyse( "ArcSin", &ArcSin, &asin, Interval( -1.0, 1.0 ), 100000 );
    Analyse( "ArcCos", &ArcCos, &acos, Interval( -1.0, 1.0 ), 100000 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10.0, 10.0 ), 100000 );
    Analyse( "ArcTan", &ArcTan, &atan2, Interval( -10.0, 10.0 ), 100, Interval( -10.0, 10.0 ), 1000 );
    
    //Analyse( "ArcTan", &ArcTan, &atan2, Interval( -1.0, 1.0 ), 10, Interval( -1.0, 1.0 ), 10, true );
    
    Analyse( "ArcTan", &ArcTan, &atan, Interval( 0.0, 0.05 ), 100000 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1.0, 1.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10.0, 10.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100.0, 100.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000.0, 1000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000.0, 10000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100000.0, 100000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000000.0, 1000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000000.0, 10000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -100000000.0, 100000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -1000000000.0, 1000000000.0 ), 1 );
    Analyse( "ArcTan", &ArcTan, &atan, Interval( -10000000000.0, 10000000000.0 ), 1 );
    
    Analyse( "Log", &Log, &log, Interval( 0.0, 1.0 ), 100000 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10.0 ), 100000 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 10000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 100000000.0 ), 1 /*, true */ );
    Analyse( "Log", &Log, &log, Interval( 0.0, 1000000000.0 ), 1 /*, true */ );
    
    Analyse( "Exp", &Exp, &exp, Interval( -0.1, 0.1 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -1.0, 1.0 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -10.0, 10.0 ), 100000 /*, true */ );
    Analyse( "Exp", &Exp, &exp, Interval( -88.0, 88.0 ), 100000 /*, true */ );
    
    Analyse( "Pow", &Pow, &pow, Interval( -2.0, 2.0 ), 100, Interval( -2.0, 2.0 ), 1000 );
    //Analyse( "Pow", &Pow, &pow, Interval( -2.0, 2.0 ), 8, Interval( -2.0, 2.0 ), 8, true );
    
    Profile( "Log", &Log, Interval( 0.0, 1.0 ), 100000 );
    
    Profile( "Cos", &Cos, Interval( -PI * 2.0, PI * 2.0 ), 100000 );
    
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
    
    __debugbreak();
}




C_SectionMathsTest::C_SectionMathsTest( const Util::C_String& name ) : C_Section( name )
{
    NewMathsTest();
}

C_SectionMathsTest::~C_SectionMathsTest()
{
}

void C_SectionMathsTest::OnRender( const C_Input& input, const C_Timer& timer )
{
	GPU_EVENT( 0xFFFFFFFF, "RenderSectionMathsTest" );
    
	Display::Device()->Clear( Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225) );
    
	Render::DrawGrid();
    
	Util::C_String str;
	str.Printf( "Maths Test Section : %.2f fps", timer.GetFrameRate() );
	Display::FontRender()->Draw( str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255) );
}

}; // namespace Samples
