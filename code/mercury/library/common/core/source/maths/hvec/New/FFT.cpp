#include "CorePch.h"
#include <core/maths/hvec/New/IncludeMinimal.h>
#include <core/maths/hvec/New/IncludeMinimal.inl>

//#include <System/DebugLog.h>

namespace Maths
{
/// @addtogroup FFT
//! @{

//static Debug::Logger& g_Log = Debug::GetLogger( "mercury.maths" );

////////////////////////////////////////////////////////////////////////////////
//  FFT
// Sorensen in-place split-radix FFT for real values
// pData: array of F32s:
// re(0),re(1),re(2),...,re(size-1)
// 
// output:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
// normalized by array length
//
// Source: 
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

DEPRECATED void RealFFT_Split0( F32* const __restrict pData, const S32 n )
{
    const F32   PI      = 3.1415926535897932384626433832795;

  long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8;
  F32 t1,t2,t3,t4,t5,t6,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;

  const F32 fRoot2          = 1.41421356237f;
  
  sqrt2=fRoot2;
  n4=n-1;
  
  //data shuffling
      for (i=0,j=0,n2=n/2; i<n4 ; i++){
      if (i<j){
                t1=pData[j];
                pData[j]=pData[i];
                pData[i]=t1;
                }
      k=n2;
      while (k<=j){
                j-=k;
                k>>=1;  
                }
      j+=k;
      }
    
/*----------------------*/
    
    //length two butterflies    
    i0=0;
    id=4;
   do{
       for (; i0<n4; i0+=id){ 
            i1=i0+1;
            t1=pData[i0];
            pData[i0]=t1+pData[i1];
            pData[i1]=t1-pData[i1];
        }
       id<<=1;
       i0=id-2;
       id<<=1;
    } while ( i0<n4 );

   /*----------------------*/
   //L shaped butterflies
n2=2;
for(k=n;k>2;k>>=1){  
    n2<<=1;
    n4=n2>>2;
    n8=n2>>3;
    e = 2*PI/(n2);
    i1=0;
    id=n2<<1;
    do{ 
        for (; i1<n; i1+=id){
            i2=i1+n4;
            i3=i2+n4;
            i4=i3+n4;
            t1=pData[i4]+pData[i3];
            pData[i4]-=pData[i3];
            pData[i3]=pData[i1]-t1;
            pData[i1]+=t1;
            if (n4!=1){
                i0=i1+n8;
                i2+=n8;
                i3+=n8;
                i4+=n8;
                t1=(pData[i3]+pData[i4])/sqrt2;
                t2=(pData[i3]-pData[i4])/sqrt2;
                pData[i4]=pData[i2]-t1;
                pData[i3]=-pData[i2]-t1;
                pData[i2]=pData[i0]-t2;
                pData[i0]+=t2;
            }
         }
         id<<=1;
         i1=id-n2;
         id<<=1;
      } while ( i1<n );
    a=e;
    for (j=2; j<=n8; j++){  
          a3=3*a;
          cc1=cos(a);
          ss1=sin(a);
          cc3=cos(a3);
          ss3=sin(a3);
          a=j*e;
          i=0;
          id=n2<<1;
          do{
           for (; i<n; i+=id){  
              i1=i+j-1;
              i2=i1+n4;
              i3=i2+n4;
              i4=i3+n4;
              i5=i+n4-j+1;
              i6=i5+n4;
              i7=i6+n4;
              i8=i7+n4;
              t1=pData[i3]*cc1+pData[i7]*ss1;
              t2=pData[i7]*cc1-pData[i3]*ss1;
              t3=pData[i4]*cc3+pData[i8]*ss3;
              t4=pData[i8]*cc3-pData[i4]*ss3;
              t5=t1+t3;
              t6=t2+t4;
              t3=t1-t3;
              t4=t2-t4;
              t2=pData[i6]+t6;
              pData[i3]=t6-pData[i6];
              pData[i8]=t2;
              t2=pData[i2]-t3;
              pData[i7]=-pData[i2]-t3;
              pData[i4]=t2;
              t1=pData[i1]+t5;
              pData[i6]=pData[i1]-t5;
              pData[i1]=t1;
              t1=pData[i5]+t4;
              pData[i5]-=t4;
              pData[i2]=t1;
             }
           id<<=1;
           i=id-n2;
           id<<=1;
         } while(i<n);
       }
      }

    //division with array length
 //  for(i=0;i<n;i++) pData[i]/=n;
}

/////////////////////////////////////////////////////////
// Sorensen in-place inverse split-radix FFT for real values
// pData: array of F32s:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
//
// output:
// re(0),re(1),re(2),...,re(size-1)
// NOT normalized by array length
//
// Source:
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
DEPRECATED void IRealFFT_Split0( F32* const __restrict pData, const S32 n )
{
    const F32   pi      = 3.1415926535897932384626433832795;

    long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8,n1;
    F32 t1,t2,t3,t4,t5,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;
    sqrt2=(F32)sqrt(2.0);
    n1=n-1;
    n2=n<<1;
    for(k=n;k>2;k>>=1){
    id=n2;
    n2>>=1;
    n4=n2>>2;
    n8=n2>>3;
    e = 2*pi/(n2);
    i1=0;
    
    do
    {
        for (; i1<n; i1+=id)
        {
            i2=i1+n4;
            i3=i2+n4;
            i4=i3+n4;
            t1=pData[i1]-pData[i3];
            pData[i1]+=pData[i3];
            pData[i2]*=2;
            pData[i3]=t1-2*pData[i4];
            pData[i4]=t1+2*pData[i4];
            
                if (n4!=1)
                {
                    i0=i1+n8;
                    i2+=n8;
                    i3+=n8;
                    i4+=n8;
                    t1=(pData[i2]-pData[i0])/sqrt2;
                    t2=(pData[i4]+pData[i3])/sqrt2;
                    pData[i0]+=pData[i2];
                    pData[i2]=pData[i4]-pData[i3];
                    pData[i3]=2*(-t2-t1);
                    pData[i4]=2*(-t2+t1);
                }
        }

            id<<=1;
            i1=id-n2;
            id<<=1;
    } while ( i1<n1 );


            a=e;

            for (j=2; j<=n8; j++)
            {

                a3=3*a;
                cc1=cos(a);
                ss1=sin(a);
                cc3=cos(a3);
                ss3=sin(a3);
                a=j*e;
                i=0;
                id=n2<<1;
            
                do
                {
                    for (; i<n; i+=id)
                    {
                            i1=i+j-1;
                            i2=i1+n4;
                            i3=i2+n4;
                            i4=i3+n4;
                            i5=i+n4-j+1;
                            i6=i5+n4;
                            i7=i6+n4;
                            i8=i7+n4;
                            t1=pData[i1]-pData[i6];
                            pData[i1]+=pData[i6];
                            t2=pData[i5]-pData[i2];
                            pData[i5]+=pData[i2];
                            t3=pData[i8]+pData[i3];
                            pData[i6]=pData[i8]-pData[i3];
                            t4=pData[i4]+pData[i7];
                            pData[i2]=pData[i4]-pData[i7];
                            t5=t1-t4;
                            t1+=t4;
                            t4=t2-t3;
                            t2+=t3;
                            pData[i3]=t5*cc1+t4*ss1;
                            pData[i7]=-t4*cc1+t5*ss1;
                            pData[i4]=t1*cc3-t2*ss3;
                            pData[i8]=t2*cc3+t1*ss3;
                    }

                    id<<=1;
                    i=id-n2;
                    id<<=1;

                } while(i<n1);
            }
        }
            /*----------------------*/
            i0=0;
            id=4;
            do
            {
            for (; i0<n1; i0+=id)
            {
            i1=i0+1;
            t1=pData[i0];
            pData[i0]=t1+pData[i1];
            pData[i1]=t1-pData[i1];
        }
        id<<=1;
        i0=id-2;
        id<<=1;
    } while ( i0<n1 );
    /*----------------------*/
    //data shuffling
    for (i=0,j=0,n2=n/2; i<n1 ; i++)
    {
        if (i<j){
        t1=pData[j];
        pData[j]=pData[i];
        pData[i]=t1;
    }

    k=n2;   

    while (k<=j)
    {
        j-=k;
        k>>=1;
        }
        j+=k;
    }

        //division with array length
   for(i=0;i<n;i++) pData[i]/=n;
}





class BitReverse
{
private:
    
	#if defined ( USING_256_FFT )
    NOALIAS __forceinline static const S32 BitReverse8( const S32 i )
    {
        return ( ( ( ( ( i & 0x001 ) << 7 ) |
                     ( ( i & 0x002 ) << 5 ) ) |
                   ( ( ( i & 0x004 ) << 3 ) |
                     ( ( i & 0x008 ) << 1 ) ) ) |
                 ( ( ( ( i & 0x010 ) >> 1 ) |
                     ( ( i & 0x020 ) >> 3 ) ) |
                   ( ( ( i & 0x040 ) >> 5 ) |
                     ( ( i & 0x080 ) >> 7 ) ) ) );
    }
	#endif

	#if defined ( USING_512_FFT )
    NOALIAS __forceinline static const S32 BitReverse9( const S32 i )
    {
        return ( ( ( ( ( i & 0x001 ) << 8 ) |
                     ( ( i & 0x002 ) << 6 ) ) |
                   ( ( ( i & 0x004 ) << 4 ) |
                     ( ( i & 0x008 ) << 2 ) ) ) |
                 ( ( ( ( i & 0x010 )      ) |
                     ( ( i & 0x020 ) >> 2 ) ) |
                   ( ( ( i & 0x040 ) >> 4 ) |
                     ( ( i & 0x080 ) >> 6 ) ) ) |
                     ( ( i & 0x100 ) >> 8 ) );
    }
	#endif
    
	#if defined ( USING_1024_FFT )
    NOALIAS __forceinline static const S32 BitReverse10( const S32 i )
    {
        return ( ( ( ( ( i & 0x001 ) << 9 ) |
                     ( ( i & 0x002 ) << 7 ) ) |
                   ( ( ( i & 0x004 ) << 5 ) |
                     ( ( i & 0x008 ) << 3 ) ) ) |
                 ( ( ( ( i & 0x010 ) << 1 ) |
                     ( ( i & 0x020 ) >> 1 ) ) |
                   ( ( ( i & 0x040 ) >> 3 ) |
                     ( ( i & 0x080 ) >> 5 ) ) ) |
                   ( ( ( i & 0x100 ) >> 7 ) |
                     ( ( i & 0x200 ) >> 9 ) ) );
    }
	#endif
    
	#if defined ( USING_2048_FFT )
    NOALIAS __forceinline static const S32 BitReverse11( const S32 i )
    {
        return ( ( ( ( ( i & 0x001 ) << 10 ) |
                     ( ( i & 0x002 ) << 8 ) ) |
                   ( ( ( i & 0x004 ) << 6 ) |
                     ( ( i & 0x008 ) << 4 ) ) ) |
                 ( ( ( ( i & 0x010 ) << 2 ) |
                     ( ( i & 0x020 )      ) ) |
                   ( ( ( i & 0x040 ) >> 2 ) |
                     ( ( i & 0x080 ) >> 4 ) ) ) |
                 ( ( ( ( i & 0x100 ) >> 6 ) |
                     ( ( i & 0x200 ) >> 8 ) ) |
                     ( ( i & 0x400 ) >> 10 ) ) );
    }
	#endif
    
public :
    
    BitReverse()
    {
	#if defined ( USING_256_FFT )
        for ( S32 i=0; i<256; i++ )
            m_BitReverseTable8[i] = BitReverse8(i);
	#endif
        
	#if defined ( USING_512_FFT )
        for ( S32 i=0; i<512; i++ )
            m_BitReverseTable9[i] = BitReverse9(i);
	#endif
        
	#if defined ( USING_1024_FFT )
        for ( S32 i=0; i<1024; i++ )
            m_BitReverseTable10[i] = BitReverse10(i);
	#endif
       
	#if defined ( USING_2048_FFT )
        for ( S32 i=0; i<2048; i++ )
            m_BitReverseTable11[i] = BitReverse11(i);
	#endif
    }
    
	#if defined ( USING_256_FFT )
    S16 m_BitReverseTable8[256];
	#endif
	#if defined ( USING_512_FFT )
    S16 m_BitReverseTable9[512];
	#endif
	#if defined ( USING_1024_FFT )
    S16 m_BitReverseTable10[1024];
	#endif
	#if defined ( USING_2048_FFT )
    S16 m_BitReverseTable11[2048];
	#endif
};

BitReverse g_BitReverse;


////////////////////////////////////////////////////////////////////////////////
//  FFT
// Sorensen in-place split-radix FFT for real values
// pData: array of F32s:
// re(0),re(1),re(2),...,re(size-1)
// 
// output:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
// original audio comment "normalized by array length" ???
// NOT normalized by array length
//
// Source: 
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
////////////////////////////////////////////////////////////////////////////////
void RealFFT_Split( F32* const __restrict pData, const S32 n )
{
    
    const F32 rcpSqrt2 = 0.70710678118654752440084436210485f;
    
    #if defined( _DEBUG )  || defined  USING_CLIB_SINCOS
    const F32 half  = 0.5f;
    const F32 three = 3.0f;
    const F32 twoPI = 6.283185307179586476925286766559f;
    #else
    const VF32 half( Const<VF32>::Half() );
    const VF32 three( Const<VF32>::Three() );
    const VF32 twoPI( Const<VF32>::TwoPi() );
    #endif
    
    S16 const * __restrict pBitReverseTable;
    switch ( n )
    {
		#if defined ( USING_256_FFT )
        case 256  : pBitReverseTable = g_BitReverse.m_BitReverseTable8;  break;
		#endif
		#if defined ( USING_512_FFT )
        case 512  : pBitReverseTable = g_BitReverse.m_BitReverseTable9;  break;
		#endif
		#if defined ( USING_1024_FFT )
        case 1024 : pBitReverseTable = g_BitReverse.m_BitReverseTable10; break;
		#endif
		#if defined ( USING_2048_FFT )
        case 2048 : pBitReverseTable = g_BitReverse.m_BitReverseTable11; break;
		#endif
        default :
        {
			printf("BufferSize: %d",n);
            assert( 0 && "Invalid buffer size!\n" );
            return;
        }
    }
    
    
    ALIGN16 F32 buffer[2048];
    
    // data shuffling
    {
        const S32 n_div_2 = n >> 1; // n / 2;
        for ( S32 i=0, j=0; i<n; i+=4 )
        {
            
            const S32 i0( i+0 );
            const S32 i1( i+1 );
            const S32 i2( i+2 );
            const S32 i3( i+3 );
            const S32 j0( pBitReverseTable[i0] );
            const S32 j1( pBitReverseTable[i1] );
            const S32 j2( pBitReverseTable[i2] );
            const S32 j3( pBitReverseTable[i3] );
            const F32 d0( pData[i0] );
            const F32 d1( pData[i1] );
            const F32 d2( pData[i2] );
            const F32 d3( pData[i3] );
            const F32 d4( pData[j0] );
            const F32 d5( pData[j1] );
            const F32 d6( pData[j2] );
            const F32 d7( pData[j3] );
            ReadWriteBarrier();
            buffer[j0] = d0;
            buffer[j1] = d1;
            buffer[j2] = d2;
            buffer[j3] = d3;
            buffer[i0] = d4;
            buffer[i1] = d5;
            buffer[i2] = d6;
            buffer[i3] = d7;
        }
        
        for ( S32 i=0; i<n; i+=32 )
        {
            const Vec4F32 data0( Load<Vec4F32>( &buffer[i+0] ) );
            const Vec4F32 data1( Load<Vec4F32>( &buffer[i+4] ) );
            const Vec4F32 data2( Load<Vec4F32>( &buffer[i+8] ) );
            const Vec4F32 data3( Load<Vec4F32>( &buffer[i+12] ) );
            const Vec4F32 data4( Load<Vec4F32>( &buffer[i+16] ) );
            const Vec4F32 data5( Load<Vec4F32>( &buffer[i+20] ) );
            const Vec4F32 data6( Load<Vec4F32>( &buffer[i+24] ) );
            const Vec4F32 data7( Load<Vec4F32>( &buffer[i+28] ) );
            ReadWriteBarrier();
            Store( data0, &pData[i+0] );
            Store( data1, &pData[i+4] );
            Store( data2, &pData[i+8] );
            Store( data3, &pData[i+12] );
            Store( data4, &pData[i+16] );
            Store( data5, &pData[i+20] );
            Store( data6, &pData[i+24] );
            Store( data7, &pData[i+28] );
        }
    }
    
    /*----------------------*/
    
    //length two butterflies    
    {
        const S32 n_minus_1 = n - 1;
        
        S32 i0 = 0;
        S32 id = 4;
        do
        {
            for ( ; i0<n_minus_1; i0+=id )
            { 
                const S32 i1 = i0 + 1;
                const F32 d0( pData[i0] );
                const F32 d1( pData[i1] );
                const F32 r0( d0 + d1 );
                const F32 r1( d0 - d1 );
                ReadWriteBarrier();
                pData[i0]= r0;
                pData[i1]= r1;
            }
            i0 = ( id << 1 ) - 2;
            id = id << 2;
        } while ( i0 < n_minus_1 );
    }
    
    /*----------------------*/
    //L shaped butterflies
    S32 n2 = 2;
    
    #if defined( _DEBUG )  || defined  USING_CLIB_SINCOS
    F32 rcp_n2 = half;
    #else
    VF32 rcp_n2 = half;
    #endif
    
    for ( S32 k=n; k>2; k>>=1 )
    {  
        n2 <<= 1;
        const S32 n4 = n2 >> 2;
        const S32 n8 = n2 >> 3;
        const S32 n4x2( n4 + n4 );
        const S32 n4x3( n4x2 + n4 );
        S32 i0 = 0;
        S32 id = n2 << 1;
        do
        { 
            for ( ; i0<n; i0+=id )
            {
                const S32 i1 = i0 + n4;
                const S32 i2 = i0 + n4x2; // i1 + n4;
                const S32 i3 = i0 + n4x3; // i2 + n4;
                const F32 d0( pData[i0] );
                const F32 d1( pData[i1] );
                const F32 d2( pData[i2] );
                const F32 d3( pData[i3] );
                const F32 t0( d2 + d3 );
                const F32 r0( d0 + t0 );
                const F32 r2( d0 - t0 );
                const F32 r3( d3 - d2 );
                ReadWriteBarrier();
                pData[i0] = r0;
                pData[i2] = r2;
                pData[i3] = r3;
                
                if ( n4 != 1 )
                {
                    const S32 i4( i0 + n8 );
                    const S32 i5( i1 + n8 );
                    const S32 i6( i2 + n8 );
                    const S32 i7( i3 + n8 );
                    const F32 d4( pData[i4] );
                    const F32 d5( pData[i5] );
                    const F32 d6( pData[i6] );
                    const F32 d7( pData[i7] );
                    const F32 t0( ( d6 - d7 ) * rcpSqrt2 );
                    const F32 t1( ( d6 + d7 ) * rcpSqrt2 );
                    const F32 r4( d4 + t0 );
                    const F32 r5( d4 - t0 );
                    const F32 r6( -d5 - t1 );
                    const F32 r7( d5 - t1 );
                    ReadWriteBarrier();
                    pData[i4] = r4;
                    pData[i5] = r5;
                    pData[i6] = r6;
                    pData[i7] = r7;
                }
            }
            i0 = ( id << 1 ) - n2;
            id = id << 2;
        } while ( i0 < n );
        
        rcp_n2 *= half;
        
#if defined( _DEBUG ) || defined  USING_CLIB_SINCOS
        const F32 e = twoPI * rcp_n2;
        F32 a = e;
        #else
        const VF32 e( twoPI * rcp_n2 );
        const Vec4F32 ve( MakeVec4F32( e, e, e * three, e * three ) );
        Vec4F32 va( ve );
        #endif
        
        for ( S32 j=2; j<=n8; j++ )
        {
            #if defined( _DEBUG )  || defined  USING_CLIB_SINCOS // Use the standard library trig functions in debug since they'll run faster
            const F32 a3 = a * three;
            const VF32 s1 = MakeVF32( sin(a) );
            const VF32 c1 = MakeVF32( cos(a) );
            const VF32 s3 = MakeVF32( sin(a3) );
            const VF32 c3 = MakeVF32( cos(a3) );
            #else
            const Vector4 sc( MakeVector4( SinCos( va ) ) );
            const VF32 s1( X(sc) );
            const VF32 c1( Y(sc) );
            const VF32 s3( Z(sc) );
            const VF32 c3( W(sc) );
            #endif
            
            #if defined( _DEBUG )  || defined  USING_CLIB_SINCOS
            a += e;
            #else
            va += ve;
            #endif
            
            S32 i = 0;
            S32 id = n2 << 1;
            do
            {
                const S32 n4x2( n4 + n4 );
                const S32 n4x3( n4x2 + n4 );
                for ( ; i<n; i+=id )
                {
                    const S32 j_minus_1( j - 1 );
                    const S32 i0 = i + j_minus_1;
                    const S32 i4 = i + n4 - j_minus_1;
                    const S32 i1 = i0 + n4;
                    const S32 i2 = i0 + n4x2;
                    const S32 i3 = i0 + n4x3;
                    const S32 i5 = i4 + n4;
                    const S32 i6 = i4 + n4x2;
                    const S32 i7 = i4 + n4x3;
                    const VF32 d0( Load<VF32>( &pData[i0] ) );
                    const VF32 d1( Load<VF32>( &pData[i1] ) );
                    const VF32 d2( Load<VF32>( &pData[i2] ) );
                    const VF32 d3( Load<VF32>( &pData[i3] ) );
                    const VF32 d4( Load<VF32>( &pData[i4] ) );
                    const VF32 d5( Load<VF32>( &pData[i5] ) );
                    const VF32 d6( Load<VF32>( &pData[i6] ) );
                    const VF32 d7( Load<VF32>( &pData[i7] ) );
                    const VF32 t0( MulAdd( d2, c1, d6 * s1 ) );
                    const VF32 t1( MulSub( d6, c1, d2 * s1 ) );
                    const VF32 t2( MulAdd( d3, c3, d7 * s3 ) );
                    const VF32 t3( MulSub( d7, c3, d3 * s3 ) );
                    const VF32 t4( t0 + t2 );
                    const VF32 t5( t1 + t3 );
                    const VF32 t6( t0 - t2 );
                    const VF32 t7( t1 - t3 );
                    const VF32 r0( d0 + t4 );
                    const VF32 r1( d4 + t7 );
                    const VF32 r2( t5 - d5 );
                    const VF32 r3( d1 - t6 );
                    const VF32 r4( d4 - t7 );
                    const VF32 r5( d0 - t4 );
                    const VF32 r6( -d1 - t6 );
                    const VF32 r7( d5 + t5 );
                    ReadWriteBarrier();
                    Store( r0, &pData[i0] );
                    Store( r1, &pData[i1] );
                    Store( r2, &pData[i2] );
                    Store( r3, &pData[i3] );
                    Store( r4, &pData[i4] );
                    Store( r5, &pData[i5] );
                    Store( r6, &pData[i6] );
                    Store( r7, &pData[i7] );
                }
                i = ( id << 1 ) - n2;
                id = id << 2;
            } while ( i < n );
        }
    }
}


/////////////////////////////////////////////////////////
// Sorensen in-place inverse split-radix FFT for real values
// pData: array of F32s:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
//
// output:
// re(0),re(1),re(2),...,re(size-1)
// original audio comment: "NOT normalized by array length" ???
// normalized by array length
//
// Source:
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
/////////////////////////////////////////////////////////
void IRealFFT_Split( F32* const __restrict pData, const S32 n )
{
    const VF32 twoPI( Const<VF32>::TwoPi() );
    const F32 sqrt2 = 1.4142135623730950488016887242097f;
    const F32 two = 2.0f;
    
    const VF32 three( Const<VF32>::Three() );
    const VF32 rcp_n( Rcp( MakeVF32( (F32)n ) ) );
    
    const S32 n_minus_1 = n - 1;
    
    S32 n2 = n << 1;
    VF32 rcp_n2( rcp_n );
    
    for ( S32 k=n; k>2; k>>=1 )
    {
        S32 id = n2;
        n2 >>= 1;
        const S32 n4 = n2 >> 2;
        const S32 n8 = n2 >> 3;
        const S32 n4x2( n4 + n4 );
        const S32 n4x3( n4x2 + n4 );
        
        S32 i0 = 0;
        do
        {
            for ( ; i0<n; i0+=id )
            {
                const S32 i1 = i0 + n4;
                const S32 i2 = i0 + n4x2;
                const S32 i3 = i0 + n4x3;
                const F32 d0( pData[i0] );
                const F32 d1( pData[i1] );
                const F32 d2( pData[i2] );
                const F32 d3( pData[i3] );
                const F32 t0( d0 - d2 );
                const F32 t1( d3 + d3 );
                const F32 r0( d0 + d2 );
                const F32 r1( d1 + d1 );
                const F32 r2( t0 - t1 );
                const F32 r3( t0 + t1 );
                ReadWriteBarrier();
                pData[i0] = r0;
                pData[i1] = r1;
                pData[i2] = r2;
                pData[i3] = r3;
                
                if ( n4 != 1 )
                {
                    const S32 i4 = i0 + n8;
                    const S32 i5 = i1 + n8;
                    const S32 i6 = i2 + n8;
                    const S32 i7 = i3 + n8;
                    const F32 d4( pData[i4] );
                    const F32 d5( pData[i5] );
                    const F32 d6( pData[i6] );
                    const F32 d7( pData[i7] );
                    const F32 t0( ( d5 - d4 ) * sqrt2 );
                    const F32 t1( ( d7 + d6 ) * sqrt2 );
                    const F32 r4( d4 + d5 );
                    const F32 r5( d7 - d6 );
                    const F32 r6( -t1 - t0 );
                    const F32 r7(  t0 - t1 );
                    ReadWriteBarrier();
                    pData[i4] = r4;
                    pData[i5] = r5;
                    pData[i6] = r6;
                    pData[i7] = r7;
                }
            }
            
            i0 = ( id << 1 ) - n2;
            id <<= 2;
            
        } while ( i0 < n_minus_1 );
        
        const VF32 e( twoPI * rcp_n2 );
        rcp_n2 += rcp_n2;
        
        Vec4F32 ve( MakeVec4F32( e, e, e * three, e * three ) );
        Vec4F32 va( ve );
        
        for ( S32 j=2; j<=n8; j++, va+=ve )
        {
            const Vector4 sc( MakeVector4( SinCos( va ) ) );
            const VF32 s1( X(sc) );
            const VF32 c1( Y(sc) );
            const VF32 s3( Z(sc) );
            const VF32 c3( W(sc) );
            
            S32 i = 0;
            S32 id = n2 << 1;
            do
            {
                const S32 n4x2( n4 + n4 );
                const S32 n4x3( n4x2 + n4 );
                for ( ; i<n; i+=id )
                {
                    const S32 j_minus_1( j - 1 );
                    const S32 i0 = i + j_minus_1;
                    const S32 i4 = i + n4 - j_minus_1;
                    const S32 i1 = i0 + n4;
                    const S32 i2 = i0 + n4x2;
                    const S32 i3 = i0 + n4x3;
                    const S32 i5 = i4 + n4;
                    const S32 i6 = i4 + n4x2;
                    const S32 i7 = i4 + n4x3;
                    const VF32 d0( Load<VF32>( &pData[i0] ) );
                    const VF32 d1( Load<VF32>( &pData[i1] ) );
                    const VF32 d2( Load<VF32>( &pData[i2] ) );
                    const VF32 d3( Load<VF32>( &pData[i3] ) );
                    const VF32 d4( Load<VF32>( &pData[i4] ) );
                    const VF32 d5( Load<VF32>( &pData[i5] ) );
                    const VF32 d6( Load<VF32>( &pData[i6] ) );
                    const VF32 d7( Load<VF32>( &pData[i7] ) );
                    const VF32 t0( d0 - d5 );
                    const VF32 t1( d4 - d1 );
                    const VF32 t2( d7 + d2 );
                    const VF32 t3( d3 + d6 );
                    const VF32 t4( t0 - t3 );
                    const VF32 t5( t0 + t3 );
                    const VF32 t6( t1 - t2 );
                    const VF32 t7( t1 + t2 );
                    const VF32 r0( d0 + d5 );
                    const VF32 r1( d3 - d6 );
                    const VF32 r2( MulAdd( t4, c1, t6 * s1 ) );
                    const VF32 r3( MulSub( t5, c3, t7 * s3 ) );
                    const VF32 r4( d4 + d1 );
                    const VF32 r5( d7 - d2 );
                    const VF32 r6( MulSub( t4, s1, t6 * c1 ) );
                    const VF32 r7( MulAdd( t7, c3, t5 * s3 ) );
                    ReadWriteBarrier();
                    Store( r0, &pData[i0] );
                    Store( r1, &pData[i1] );
                    Store( r2, &pData[i2] );
                    Store( r3, &pData[i3] );
                    Store( r4, &pData[i4] );
                    Store( r5, &pData[i5] );
                    Store( r6, &pData[i6] );
                    Store( r7, &pData[i7] );
                }
                
                i = ( id << 1 ) - n2;
                id <<= 2;
                
            } while( i < n_minus_1 );
        }
    }
    
    /*----------------------*/
    S32 i0 = 0;
    S32 id = 4;
    do
    {
        for ( ; i0<n_minus_1; i0+=id )
        {
            const S32 i1 = i0 + 1;
            const F32 d0( pData[i0] );
            const F32 d1( pData[i1] );
            const F32 r0( d0 + d1 );
            const F32 r1( d0 - d1 );
            ReadWriteBarrier();
            pData[i0] = r0;
            pData[i1] = r1;
        }
        i0 = ( id << 1 ) - 2;
        id <<= 2;
    } while ( i0 < n_minus_1 );
    
    /*----------------------*/
    //data shuffling and division by array length
    
    S16 const * __restrict pBitReverseTable;
    switch ( n )
    {
		#if defined ( USING_256_FFT )
        case 256  : pBitReverseTable = g_BitReverse.m_BitReverseTable8;  break;
		#endif
		#if defined ( USING_512_FFT )
        case 512  : pBitReverseTable = g_BitReverse.m_BitReverseTable9;  break;
		#endif
		#if defined ( USING_1024_FFT )
        case 1024 : pBitReverseTable = g_BitReverse.m_BitReverseTable10; break;
		#endif
		#if defined ( USING_2048_FFT )
        case 2048 : pBitReverseTable = g_BitReverse.m_BitReverseTable11; break;
		#endif
        default :
        {
			printf("BufferSize: %d",n);
            assert( 0 && "Invalid buffer size!\n" );
            return;
        }
    }
    
    
    ALIGN16 F32 buffer[2048];
    
    {
        const S32 n_div_2 = n >> 1;
        for ( S32 i=0, j=0; i<n; i+=4 )
        {
            
            const S32 i0( i+0 );
            const S32 i1( i+1 );
            const S32 i2( i+2 );
            const S32 i3( i+3 );
            const S32 j0( pBitReverseTable[i0] );
            const S32 j1( pBitReverseTable[i1] );
            const S32 j2( pBitReverseTable[i2] );
            const S32 j3( pBitReverseTable[i3] );
            const F32 d0( pData[i0] );
            const F32 d1( pData[i1] );
            const F32 d2( pData[i2] );
            const F32 d3( pData[i3] );
            const F32 d4( pData[j0] );
            const F32 d5( pData[j1] );
            const F32 d6( pData[j2] );
            const F32 d7( pData[j3] );
            ReadWriteBarrier();
            buffer[j0] = d0;
            buffer[j1] = d1;
            buffer[j2] = d2;
            buffer[j3] = d3;
            buffer[i0] = d4;
            buffer[i1] = d5;
            buffer[i2] = d6;
            buffer[i3] = d7;
        }
        
        for ( S32 i=0; i<n; i+=32 )
        {
            const Vec4F32 data0( Load<Vec4F32>( &buffer[i+0]  ) * rcp_n );
            const Vec4F32 data1( Load<Vec4F32>( &buffer[i+4]  ) * rcp_n );
            const Vec4F32 data2( Load<Vec4F32>( &buffer[i+8]  ) * rcp_n );
            const Vec4F32 data3( Load<Vec4F32>( &buffer[i+12] ) * rcp_n );
            const Vec4F32 data4( Load<Vec4F32>( &buffer[i+16] ) * rcp_n );
            const Vec4F32 data5( Load<Vec4F32>( &buffer[i+20] ) * rcp_n );
            const Vec4F32 data6( Load<Vec4F32>( &buffer[i+24] ) * rcp_n );
            const Vec4F32 data7( Load<Vec4F32>( &buffer[i+28] ) * rcp_n );
            ReadWriteBarrier();
            Store( data0, &pData[i+0] );
            Store( data1, &pData[i+4] );
            Store( data2, &pData[i+8] );
            Store( data3, &pData[i+12] );
            Store( data4, &pData[i+16] );
            Store( data5, &pData[i+20] );
            Store( data6, &pData[i+24] );
            Store( data7, &pData[i+28] );
        }
    }
}



//! @}
}; // namespace Maths


