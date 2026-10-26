#ifndef _FRAME_H_
#define _FRAME_H_

#include <math.h>
#include "../BatchFrameAdd.h"
#if defined (AUDIO_USE_NEW_MATHS)
#include <core/maths.h>
#include <core/maths/hvec/new/includecomplete.h>
#include <core/maths/hvec/new/includecomplete.inl>
#endif // (AUDIO_USE_NEW_MATHS)
#include <core/maths/hvec/simd.h>


namespace Audio
{
    namespace Frame
    {
#if defined( PLATFORM_PC )
		inline void ZeroDenormals( F32* pFrame )
        {
            for( U32 uCount = 0; uCount < Audio::Core::uFrameLength; uCount++ )
            {
#if 0
				//	remove denormal
				static const F32 fTinyVal = 1e-18;
				pFrame[ uCount ] += fTinyVal;
				pFrame[ uCount ] -= fTinyVal;
#else
				//	test and remove denormal
				U32 uBits = Maths::IntFloatToBits( pFrame[ uCount ] );
				const U32 uAbsMant = uBits & 0x007FFFFF;
				const U32 uBiasedExp = uBits & 0x7F800000;
				if( uBiasedExp == 0 && uAbsMant != 0 )
					pFrame[ uCount ] = 0.f;
#endif
            }
        }
#else // PLATFORM_PC
		inline void ZeroDenormals( F32* pFrame ) {}
#endif // PLATFORM_PC

#ifndef MINIMAL_SPU_FRAME_FUNCTIONS

		inline bool IsValid( const F32* pFrame )
        {
            for( U32 uCount = 0; uCount < Audio::Core::uFrameLength; uCount++ )
            {
                if( !Audio::Maths::IsFinite( pFrame[ uCount ] ) )
                    return false;
            }
            return true;
        }

		inline bool IsZero( F32 const * const __restrict pFrame )
        {
            for( U32 uCount = 0; uCount < Audio::Core::uFrameLength; uCount++ )
            {
				if( pFrame[ uCount ] <= Audio::Maths::fEpsilon )
                    return false;
            }
            return true;
        }

#endif


#if defined( PLATFORM_PC )
        inline void DoAdd(F32* pTo, const F32* pFromA, const F32* pFromB)
        {
            for (int i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = pFromA[i] + pFromB[i];
			
        }
        inline void AddBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB)
        {
			DoAdd( pTo, pFromA, pFromB );
        }

#endif // PLATFORM_PC

#if defined( PLATFORM_SPU )
		inline void DoAdd( F32* pTo, const F32* pFromA, const F32* pFromB )
        {
            vector float* pvTo = (vector float*)pTo;
            vector float const* pvFromA = (vector float*)pFromA;
            vector float const* pvFromB = (vector float*)pFromB;

            for( U32 i = 0; i < 4; i++ )
            {
				register const vector float vFromA00 = pvFromA[0];
				register const vector float vFromA01 = pvFromA[1];
				register const vector float vFromA02 = pvFromA[2];
				register const vector float vFromA03 = pvFromA[3];
				register const vector float vFromA04 = pvFromA[4];
				register const vector float vFromA05 = pvFromA[5];
				register const vector float vFromA06 = pvFromA[6];
				register const vector float vFromA07 = pvFromA[7];
				register const vector float vFromA08 = pvFromA[8];
				register const vector float vFromA09 = pvFromA[9];
				register const vector float vFromA10 = pvFromA[10];
				register const vector float vFromA11 = pvFromA[11];
				register const vector float vFromA12 = pvFromA[12];
				register const vector float vFromA13 = pvFromA[13];
				register const vector float vFromA14 = pvFromA[14];
				register const vector float vFromA15 = pvFromA[15];
                pvFromA += 16;

				register const vector float vFromB00 = pvFromB[0];
				register const vector float vFromB01 = pvFromB[1];
				register const vector float vFromB02 = pvFromB[2];
				register const vector float vFromB03 = pvFromB[3];
				register const vector float vFromB04 = pvFromB[4];
				register const vector float vFromB05 = pvFromB[5];
				register const vector float vFromB06 = pvFromB[6];
				register const vector float vFromB07 = pvFromB[7];
				register const vector float vFromB08 = pvFromB[8];
				register const vector float vFromB09 = pvFromB[9];
				register const vector float vFromB10 = pvFromB[10];
				register const vector float vFromB11 = pvFromB[11];
				register const vector float vFromB12 = pvFromB[12];
				register const vector float vFromB13 = pvFromB[13];
				register const vector float vFromB14 = pvFromB[14];
				register const vector float vFromB15 = pvFromB[15];
                pvFromB += 16;

				register vector float vTemp00 = spu_add(vFromA00, vFromB00);
				register vector float vTemp01 = spu_add(vFromA01, vFromB01);
				register vector float vTemp02 = spu_add(vFromA02, vFromB02);
				register vector float vTemp03 = spu_add(vFromA03, vFromB03);
				register vector float vTemp04 = spu_add(vFromA04, vFromB04);
				register vector float vTemp05 = spu_add(vFromA05, vFromB05);
				register vector float vTemp06 = spu_add(vFromA06, vFromB06);
				register vector float vTemp07 = spu_add(vFromA07, vFromB07);
				register vector float vTemp08 = spu_add(vFromA08, vFromB08);
				register vector float vTemp09 = spu_add(vFromA09, vFromB09);
				register vector float vTemp10 = spu_add(vFromA10, vFromB10);
				register vector float vTemp11 = spu_add(vFromA11, vFromB11);
				register vector float vTemp12 = spu_add(vFromA12, vFromB12);
				register vector float vTemp13 = spu_add(vFromA13, vFromB13);
				register vector float vTemp14 = spu_add(vFromA14, vFromB14);
				register vector float vTemp15 = spu_add(vFromA15, vFromB15);
                
				pvTo[0]  = vTemp00;
				pvTo[1]  = vTemp01;
				pvTo[2]  = vTemp02;
				pvTo[3]  = vTemp03;
				pvTo[4]  = vTemp04;
				pvTo[5]  = vTemp05;
				pvTo[6]  = vTemp06;
				pvTo[7]  = vTemp07;
				pvTo[8]  = vTemp08;
				pvTo[9]  = vTemp09;
				pvTo[10] = vTemp10;
				pvTo[11] = vTemp11;
				pvTo[12] = vTemp12;
				pvTo[13] = vTemp13;
				pvTo[14] = vTemp14;
				pvTo[15] = vTemp15;
                pvTo += 16;
            }
		}
		inline void AddBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB )
		{
			DoAdd( pTo, pFromA, pFromB );
		}
#endif // PLATFORM_SPU

#if defined( PLATFORM_PS3 )
		inline void DoAdd( F32* pTo, const F32* pFromA, const F32* pFromB )
        {
			vector float* pvTo = (vector float*)pTo;
			vector float const* pvFromA = (vector float*)pFromA;
			vector float const* pvFromB = (vector float*)pFromB;

			for( U32 i = 0; i < 4; i++ )
			{
				register const vector float vFromA00 = pvFromA[0];
				register const vector float vFromA01 = pvFromA[1];
				register const vector float vFromA02 = pvFromA[2];
				register const vector float vFromA03 = pvFromA[3];
				register const vector float vFromA04 = pvFromA[4];
				register const vector float vFromA05 = pvFromA[5];
				register const vector float vFromA06 = pvFromA[6];
				register const vector float vFromA07 = pvFromA[7];
				register const vector float vFromA08 = pvFromA[8];
				register const vector float vFromA09 = pvFromA[9];
				register const vector float vFromA10 = pvFromA[10];
				register const vector float vFromA11 = pvFromA[11];
				register const vector float vFromA12 = pvFromA[12];
				register const vector float vFromA13 = pvFromA[13];
				register const vector float vFromA14 = pvFromA[14];
				register const vector float vFromA15 = pvFromA[15];
				pvFromA += 16;

				register const vector float vFromB00 = pvFromB[0];
				register const vector float vFromB01 = pvFromB[1];
				register const vector float vFromB02 = pvFromB[2];
				register const vector float vFromB03 = pvFromB[3];
				register const vector float vFromB04 = pvFromB[4];
				register const vector float vFromB05 = pvFromB[5];
				register const vector float vFromB06 = pvFromB[6];
				register const vector float vFromB07 = pvFromB[7];
				register const vector float vFromB08 = pvFromB[8];
				register const vector float vFromB09 = pvFromB[9];
				register const vector float vFromB10 = pvFromB[10];
				register const vector float vFromB11 = pvFromB[11];
				register const vector float vFromB12 = pvFromB[12];
				register const vector float vFromB13 = pvFromB[13];
				register const vector float vFromB14 = pvFromB[14];
				register const vector float vFromB15 = pvFromB[15];
				pvFromB += 16;

				register vector float vTemp00 = vec_add(vFromA00, vFromB00);
				register vector float vTemp01 = vec_add(vFromA01, vFromB01);
				register vector float vTemp02 = vec_add(vFromA02, vFromB02);
				register vector float vTemp03 = vec_add(vFromA03, vFromB03);
				register vector float vTemp04 = vec_add(vFromA04, vFromB04);
				register vector float vTemp05 = vec_add(vFromA05, vFromB05);
				register vector float vTemp06 = vec_add(vFromA06, vFromB06);
				register vector float vTemp07 = vec_add(vFromA07, vFromB07);
				register vector float vTemp08 = vec_add(vFromA08, vFromB08);
				register vector float vTemp09 = vec_add(vFromA09, vFromB09);
				register vector float vTemp10 = vec_add(vFromA10, vFromB10);
				register vector float vTemp11 = vec_add(vFromA11, vFromB11);
				register vector float vTemp12 = vec_add(vFromA12, vFromB12);
				register vector float vTemp13 = vec_add(vFromA13, vFromB13);
				register vector float vTemp14 = vec_add(vFromA14, vFromB14);
				register vector float vTemp15 = vec_add(vFromA15, vFromB15);
                
				pvTo[0]  = vTemp00;
				pvTo[1]  = vTemp01;
				pvTo[2]  = vTemp02;
				pvTo[3]  = vTemp03;
				pvTo[4]  = vTemp04;
				pvTo[5]  = vTemp05;
				pvTo[6]  = vTemp06;
				pvTo[7]  = vTemp07;
				pvTo[8]  = vTemp08;
				pvTo[9]  = vTemp09;
				pvTo[10] = vTemp10;
				pvTo[11] = vTemp11;
				pvTo[12] = vTemp12;
				pvTo[13] = vTemp13;
				pvTo[14] = vTemp14;
				pvTo[15] = vTemp15;
				pvTo += 16;
			}
		}
		inline void AddBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB )
		{
			pBatch->Add( pTo, pFromA, pFromB );
		}
#endif // PLATFORM_PS3


#if defined( PLATFORM_PC )
        inline void DoMultiply(F32* pTo, const F32* pFromA, const F32* pFromB)
        {
            for (int i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = pFromA[i] * pFromB[i];
			
        }
        inline void MultiplyBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB)
        {
			DoMultiply( pTo, pFromA, pFromB );
        }

#endif // PLATFORM_PC

#if defined( PLATFORM_SPU )
		inline void DoMultiply( F32* pTo, const F32* pFromA, const F32* pFromB )
        {
            vector float* pvTo = (vector float*)pTo;
            vector float const* pvFromA = (vector float*)pFromA;
            vector float const* pvFromB = (vector float*)pFromB;

            for( U32 i = 0; i < 4; i++ )
            {
				register const vector float vFromA00 = pvFromA[0];
				register const vector float vFromA01 = pvFromA[1];
				register const vector float vFromA02 = pvFromA[2];
				register const vector float vFromA03 = pvFromA[3];
				register const vector float vFromA04 = pvFromA[4];
				register const vector float vFromA05 = pvFromA[5];
				register const vector float vFromA06 = pvFromA[6];
				register const vector float vFromA07 = pvFromA[7];
				register const vector float vFromA08 = pvFromA[8];
				register const vector float vFromA09 = pvFromA[9];
				register const vector float vFromA10 = pvFromA[10];
				register const vector float vFromA11 = pvFromA[11];
				register const vector float vFromA12 = pvFromA[12];
				register const vector float vFromA13 = pvFromA[13];
				register const vector float vFromA14 = pvFromA[14];
				register const vector float vFromA15 = pvFromA[15];
                pvFromA += 16;

				register const vector float vFromB00 = pvFromB[0];
				register const vector float vFromB01 = pvFromB[1];
				register const vector float vFromB02 = pvFromB[2];
				register const vector float vFromB03 = pvFromB[3];
				register const vector float vFromB04 = pvFromB[4];
				register const vector float vFromB05 = pvFromB[5];
				register const vector float vFromB06 = pvFromB[6];
				register const vector float vFromB07 = pvFromB[7];
				register const vector float vFromB08 = pvFromB[8];
				register const vector float vFromB09 = pvFromB[9];
				register const vector float vFromB10 = pvFromB[10];
				register const vector float vFromB11 = pvFromB[11];
				register const vector float vFromB12 = pvFromB[12];
				register const vector float vFromB13 = pvFromB[13];
				register const vector float vFromB14 = pvFromB[14];
				register const vector float vFromB15 = pvFromB[15];
                pvFromB += 16;

				register vector float vTemp00 = spu_mul(vFromA00, vFromB00);
				register vector float vTemp01 = spu_mul(vFromA01, vFromB01);
				register vector float vTemp02 = spu_mul(vFromA02, vFromB02);
				register vector float vTemp03 = spu_mul(vFromA03, vFromB03);
				register vector float vTemp04 = spu_mul(vFromA04, vFromB04);
				register vector float vTemp05 = spu_mul(vFromA05, vFromB05);
				register vector float vTemp06 = spu_mul(vFromA06, vFromB06);
				register vector float vTemp07 = spu_mul(vFromA07, vFromB07);
				register vector float vTemp08 = spu_mul(vFromA08, vFromB08);
				register vector float vTemp09 = spu_mul(vFromA09, vFromB09);
				register vector float vTemp10 = spu_mul(vFromA10, vFromB10);
				register vector float vTemp11 = spu_mul(vFromA11, vFromB11);
				register vector float vTemp12 = spu_mul(vFromA12, vFromB12);
				register vector float vTemp13 = spu_mul(vFromA13, vFromB13);
				register vector float vTemp14 = spu_mul(vFromA14, vFromB14);
				register vector float vTemp15 = spu_mul(vFromA15, vFromB15);
                
				pvTo[0]  = vTemp00;
				pvTo[1]  = vTemp01;
				pvTo[2]  = vTemp02;
				pvTo[3]  = vTemp03;
				pvTo[4]  = vTemp04;
				pvTo[5]  = vTemp05;
				pvTo[6]  = vTemp06;
				pvTo[7]  = vTemp07;
				pvTo[8]  = vTemp08;
				pvTo[9]  = vTemp09;
				pvTo[10] = vTemp10;
				pvTo[11] = vTemp11;
				pvTo[12] = vTemp12;
				pvTo[13] = vTemp13;
				pvTo[14] = vTemp14;
				pvTo[15] = vTemp15;
                pvTo += 16;
            }
		}
		inline void MultiplyBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB )
		{
			DoMultiply( pTo, pFromA, pFromB );
		}
#endif // PLATFORM_SPU

#if defined( PLATFORM_PS3 )
		inline void DoMultiply( F32* pTo, const F32* pFromA, const F32* pFromB )
        {
			vector float* pvTo = (vector float*)pTo;
			vector float const* pvFromA = (vector float*)pFromA;
			vector float const* pvFromB = (vector float*)pFromB;

			for( U32 i = 0; i < 4; i++ )
			{
				register const vector float vFromA00 = pvFromA[0];
				register const vector float vFromA01 = pvFromA[1];
				register const vector float vFromA02 = pvFromA[2];
				register const vector float vFromA03 = pvFromA[3];
				register const vector float vFromA04 = pvFromA[4];
				register const vector float vFromA05 = pvFromA[5];
				register const vector float vFromA06 = pvFromA[6];
				register const vector float vFromA07 = pvFromA[7];
				register const vector float vFromA08 = pvFromA[8];
				register const vector float vFromA09 = pvFromA[9];
				register const vector float vFromA10 = pvFromA[10];
				register const vector float vFromA11 = pvFromA[11];
				register const vector float vFromA12 = pvFromA[12];
				register const vector float vFromA13 = pvFromA[13];
				register const vector float vFromA14 = pvFromA[14];
				register const vector float vFromA15 = pvFromA[15];
				pvFromA += 16;

				register const vector float vFromB00 = pvFromB[0];
				register const vector float vFromB01 = pvFromB[1];
				register const vector float vFromB02 = pvFromB[2];
				register const vector float vFromB03 = pvFromB[3];
				register const vector float vFromB04 = pvFromB[4];
				register const vector float vFromB05 = pvFromB[5];
				register const vector float vFromB06 = pvFromB[6];
				register const vector float vFromB07 = pvFromB[7];
				register const vector float vFromB08 = pvFromB[8];
				register const vector float vFromB09 = pvFromB[9];
				register const vector float vFromB10 = pvFromB[10];
				register const vector float vFromB11 = pvFromB[11];
				register const vector float vFromB12 = pvFromB[12];
				register const vector float vFromB13 = pvFromB[13];
				register const vector float vFromB14 = pvFromB[14];
				register const vector float vFromB15 = pvFromB[15];
				pvFromB += 16;

				register vector float vTemp00 = ::Maths::vec_mul(vFromA00, vFromB00);
				register vector float vTemp01 = ::Maths::vec_mul(vFromA01, vFromB01);
				register vector float vTemp02 = ::Maths::vec_mul(vFromA02, vFromB02);
				register vector float vTemp03 = ::Maths::vec_mul(vFromA03, vFromB03);
				register vector float vTemp04 = ::Maths::vec_mul(vFromA04, vFromB04);
				register vector float vTemp05 = ::Maths::vec_mul(vFromA05, vFromB05);
				register vector float vTemp06 = ::Maths::vec_mul(vFromA06, vFromB06);
				register vector float vTemp07 = ::Maths::vec_mul(vFromA07, vFromB07);
				register vector float vTemp08 = ::Maths::vec_mul(vFromA08, vFromB08);
				register vector float vTemp09 = ::Maths::vec_mul(vFromA09, vFromB09);
				register vector float vTemp10 = ::Maths::vec_mul(vFromA10, vFromB10);
				register vector float vTemp11 = ::Maths::vec_mul(vFromA11, vFromB11);
				register vector float vTemp12 = ::Maths::vec_mul(vFromA12, vFromB12);
				register vector float vTemp13 = ::Maths::vec_mul(vFromA13, vFromB13);
				register vector float vTemp14 = ::Maths::vec_mul(vFromA14, vFromB14);
				register vector float vTemp15 = ::Maths::vec_mul(vFromA15, vFromB15);
                
				pvTo[0]  = vTemp00;
				pvTo[1]  = vTemp01;
				pvTo[2]  = vTemp02;
				pvTo[3]  = vTemp03;
				pvTo[4]  = vTemp04;
				pvTo[5]  = vTemp05;
				pvTo[6]  = vTemp06;
				pvTo[7]  = vTemp07;
				pvTo[8]  = vTemp08;
				pvTo[9]  = vTemp09;
				pvTo[10] = vTemp10;
				pvTo[11] = vTemp11;
				pvTo[12] = vTemp12;
				pvTo[13] = vTemp13;
				pvTo[14] = vTemp14;
				pvTo[15] = vTemp15;
				pvTo += 16;
			}
		}
		inline void MultiplyBatch( Audio::BatchFrame::cBatch* pBatch, F32* pTo, const F32* pFromA, const F32* pFromB )
		{
			//pBatch->Multiply( pTo, pFromA, pFromB );
		}
#endif // PLATFORM_PS3



#if defined (AUDIO_USE_NEW_MATHS)
        inline void Zero( F32* const __restrict pTo )
        {
            memset( pTo, 0, 1024 );
        }
        
        inline void Zero( F32* const __restrict pTo, U32 channelCount )
        {
			for ( U32 iChannel = 0; iChannel < channelCount; iChannel++ )
			{
				Zero( pTo + ( iChannel * Audio::Core::uFrameLength ) );
			}
        }

#ifndef MINIMAL_SPU_FRAME_FUNCTIONS
        inline void Copy( F32* __restrict pTo, F32 const * __restrict pFrom )
        {
            Assert( pTo != pFrom );
            
            
            for ( U32 i=0; i<4; i++ )
            {
				const ::Maths::VF32 vFrom_00 = ::Maths::Load( &pFrom[0x00] );
                const ::Maths::VF32 vFrom_01 = ::Maths::Load( &pFrom[0x04] );
                const ::Maths::VF32 vFrom_02 = ::Maths::Load( &pFrom[0x08] );
                const ::Maths::VF32 vFrom_03 = ::Maths::Load( &pFrom[0x0C] );
                const ::Maths::VF32 vFrom_04 = ::Maths::Load( &pFrom[0x10] );
                const ::Maths::VF32 vFrom_05 = ::Maths::Load( &pFrom[0x14] );
                const ::Maths::VF32 vFrom_06 = ::Maths::Load( &pFrom[0x18] );
                const ::Maths::VF32 vFrom_07 = ::Maths::Load( &pFrom[0x1C] );
                const ::Maths::VF32 vFrom_08 = ::Maths::Load( &pFrom[0x20] );
                const ::Maths::VF32 vFrom_09 = ::Maths::Load( &pFrom[0x24] );
                const ::Maths::VF32 vFrom_10 = ::Maths::Load( &pFrom[0x28] );
                const ::Maths::VF32 vFrom_11 = ::Maths::Load( &pFrom[0x2C] );
                const ::Maths::VF32 vFrom_12 = ::Maths::Load( &pFrom[0x30] );
                const ::Maths::VF32 vFrom_13 = ::Maths::Load( &pFrom[0x34] );
                const ::Maths::VF32 vFrom_14 = ::Maths::Load( &pFrom[0x38] );
                const ::Maths::VF32 vFrom_15 = ::Maths::Load( &pFrom[0x3C] );
                pFrom += 0x40;
                ReadWriteBarrier();
				::Maths::Store( vFrom_00, &pTo[0x00] );
                ::Maths::Store( vFrom_01, &pTo[0x04] );
                ::Maths::Store( vFrom_02, &pTo[0x08] );
                ::Maths::Store( vFrom_03, &pTo[0x0C] );
                ::Maths::Store( vFrom_04, &pTo[0x10] );
                ::Maths::Store( vFrom_05, &pTo[0x14] );
                ::Maths::Store( vFrom_06, &pTo[0x18] );
                ::Maths::Store( vFrom_07, &pTo[0x1C] );
                ::Maths::Store( vFrom_08, &pTo[0x20] );
                ::Maths::Store( vFrom_09, &pTo[0x24] );
                ::Maths::Store( vFrom_10, &pTo[0x28] );
                ::Maths::Store( vFrom_11, &pTo[0x2C] );
                ::Maths::Store( vFrom_12, &pTo[0x30] );
                ::Maths::Store( vFrom_13, &pTo[0x34] );
                ::Maths::Store( vFrom_14, &pTo[0x38] );
                ::Maths::Store( vFrom_15, &pTo[0x3C] );
                pTo += 0x40;
            }
        }
#endif
#endif // (AUDIO_USE_NEW_MATHS)



#ifdef PLATFORM_PC

#if !defined (AUDIO_USE_NEW_MATHS)
        inline void Zero(F32* pTo)
        {
			//for (int i=0; i<Audio::Core::uFrameLength; i++)
			//	pTo[i] = 0.f;
			ZeroMemory(pTo, sizeof(float) * Audio::Core::uFrameLength);
        }
        inline void Zero(F32* pTo, U32 channelCount)
        {
			for (U32 i=0; i < channelCount * Audio::Core::uFrameLength; i++)
				pTo[i] = 0.f;
			//ZeroMemory(pTo, sizeof(float) * Audio::Core::uFrameLength * channelCount);
        }
        inline void Copy(F32* pTo, const F32* pFrom)
        {
			for (int i=0; i < Audio::Core::uFrameLength; i++)
				pTo[i] = pFrom[i];
			//memcpy(pTo, pFrom, sizeof(float) * Audio::Core::uFrameLength);
        }
#endif // (AUDIO_USE_NEW_MATHS)
        inline void Gain(F32* pTo, const F32* pFrom, F32 fGain)
        {
            for (int i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = fGain * pFrom[i];
        }
				////////////////////////////////////////////
		//various gain routines for different sized arrays
		/////////////////////////////////////////////
		  inline void Gain128( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain( pTo, pFrom, vGain );
            ////SubGain( &pTo[64], &pFrom[64], vGain );

        }

				//various gain routines for different sized arrays
		////inline void SubGain32( F32* pTo, const F32* pFrom, __vector4& vGain )
      ////  {
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            ////register const __vector4 vFrom01 = pvFrom[1];
            ////register const __vector4 vFrom02 = pvFrom[2];
            ////register const __vector4 vFrom03 = pvFrom[3];
            ////register const __vector4 vFrom04 = pvFrom[4];
            ////register const __vector4 vFrom05 = pvFrom[5];
            ////register const __vector4 vFrom06 = pvFrom[6];
            ////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            ////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            ////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            ////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            ////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            ////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            ////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            ////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            ////pvTo[1]  = vTemp01;
            ////pvTo[2]  = vTemp02;
            ////pvTo[3]  = vTemp03;
            ////pvTo[4]  = vTemp04;
            ////pvTo[5]  = vTemp05;
            ////pvTo[6]  = vTemp06;
            ////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
       //// }

        inline void Gain32( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain32( pTo, pFrom, vGain );
            //////SubGain32( &pTo[64], &pFrom[64], vGain );
            //////SubGain32( &pTo[128], &pFrom[128], vGain );
            //////SubGain32( &pTo[192], &pFrom[192], vGain );
        }

		////inline void SubGain4( F32* pTo, const F32* pFrom, __vector4& vGain )
       //// {
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            //////register const __vector4 vFrom01 = pvFrom[1];
            //////register const __vector4 vFrom02 = pvFrom[2];
            //////register const __vector4 vFrom03 = pvFrom[3];
            //////register const __vector4 vFrom04 = pvFrom[4];
            //////register const __vector4 vFrom05 = pvFrom[5];
            //////register const __vector4 vFrom06 = pvFrom[6];
            //////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            //////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            //////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            //////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            //////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            //////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            //////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            //////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            //////pvTo[1]  = vTemp01;
            //////pvTo[2]  = vTemp02;
            //////pvTo[3]  = vTemp03;
            //////pvTo[4]  = vTemp04;
            //////pvTo[5]  = vTemp05;
            //////pvTo[6]  = vTemp06;
            //////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
      ////  }

        inline void Gain4( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain4( pTo, pFrom, vGain );
            //////SubGain4( &pTo[64], &pFrom[64], vGain );
            //////SubGain4( &pTo[128], &pFrom[128], vGain );
            //////SubGain4( &pTo[192], &pFrom[192], vGain );
        }
        inline void AddGain(F32* pTo, const F32* pFromA, const F32* pFromB, F32 fGainB)
		{
            for (int i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = pFromA[i] + fGainB * pFromB[i];
		}
        inline void GainAddGain(F32* pTo, const F32* pFromA, F32 fGainA, const F32* pFromB, F32 fGainB)
        {
            for (int i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = fGainA * pFromA[i] + fGainB * pFromB[i];
        }
        inline void GainRamp(F32* pTo, const F32* pFrom, F32 fGain1, F32 fGain2,U32 frameLength = Audio::Core::uFrameLength)
        {
			const F32 fStep = (fGain2 - fGain1) / (F32)Audio::Core::uFrameLength;
			F32 fSampleStep(0.f);

			for (U32 uSample = 0; uSample < Audio::Core::uFrameLength; uSample++, fSampleStep+=fStep)
				*pTo++ = *pFrom++ * (fGain1 + fSampleStep);
		}
        inline void AddGainRamp(F32* pTo, const F32* pFromA, const F32* pFromB, F32 fGainB1, F32 fGainB2)
        {
			const F32 fStep = (fGainB2 - fGainB1) / (F32)Audio::Core::uFrameLength;
			F32 fSampleStep(0.f);

			for (U32 uSample=0; uSample < Audio::Core::uFrameLength; uSample++, fSampleStep+=fStep)
				*pTo++ = *pFromA++ + *pFromB++ * (fGainB1 + fSampleStep);
		}
        inline void Prefetch(const F32* pFrom)
		{
		}

        //static F32* g_pTempAddr = NULL;
        //inline F32* TempAlloc(U32 uNumFrames)
        //{
        //    Assert(g_pTempAddr == NULL);

        //    U32 uMask = Core::uFrameLength - 1;
        //    U32 uSize = uNumFrames * Core::uFrameLength;
        //    g_pTempAddr = new F32[uSize + Core::uFrameLength];
        //    U32 uAddr = (((U32)g_pTempAddr + uMask) & ~uMask);
        //    return (F32*)uAddr;
        //}

        //inline void TempFree(F32* pFrame)
        //{
        //    Assert(g_pTempAddr != NULL);

        //    delete [] g_pTempAddr;
        //    g_pTempAddr = NULL;
        //}

        inline void ContinuousSRC(F32* pTo, F32* pFrom, U32 uLength, F32 fFirst)
        {
   	        F32* afDataIn = pFrom;
   	        F32* afDataOut = pTo;

            S32 sIndex = 0;
            F64 fFraction = 0.0;

            F64 fStep = Core::fFrameLength / (F32)uLength;

            for (U32 uCount = 0; uCount < uLength; uCount++)
            {
                F32 fSampleA = (sIndex == 0) ? fFirst : afDataIn[sIndex - 1];
                F32 fSampleB = afDataIn[sIndex];
                F32 fDiff = fSampleB - fSampleA;
                F32 fDelta = fDiff * (F32)fFraction;
                F32 fValue = fSampleA + fDelta;

                afDataOut[uCount] = fValue;

                fFraction += fStep;
                if (fFraction >= 1.0)
                {
                    S32 sInc = (S32)fFraction;
                    fFraction -= (F64)sInc;
                    sIndex += sInc;
                }
            }
        }

#endif // PLATFORM_PC

#if defined (PLATFORM_PS3)

#if !defined (AUDIO_USE_NEW_MATHS)
        inline void Zero(F32* pTo)
        {
			memset(pTo, 0, sizeof(float) * Audio::Core::uFrameLength);
		}

		inline void Zero(F32* pTo, U32 channelCount)
        {
			memset(pTo, 0, sizeof(float) * Audio::Core::uFrameLength * channelCount);
        }

		inline void Copy(F32* pTo, const F32* pFrom)
        {
			memcpy(pTo, pFrom, sizeof(float) * Audio::Core::uFrameLength);
        }
#endif // (AUDIO_USE_NEW_MATHS)

        inline void Gain(F32* pTo, const F32* pFrom, F32 fGain)
        {
            for (U32 i=0; i < Audio::Core::uFrameLength; i++)
                pTo[i] = fGain * pFrom[i];
        }

		
			////////////////////////////////////////////
		//various gain routines for different sized arrays
		/////////////////////////////////////////////
		  inline void Gain128( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain( pTo, pFrom, vGain );
            ////SubGain( &pTo[64], &pFrom[64], vGain );

        }

				//various gain routines for different sized arrays
		////inline void SubGain32( F32* pTo, const F32* pFrom, __vector4& vGain )
        ////{
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            ////register const __vector4 vFrom01 = pvFrom[1];
            ////register const __vector4 vFrom02 = pvFrom[2];
            ////register const __vector4 vFrom03 = pvFrom[3];
            ////register const __vector4 vFrom04 = pvFrom[4];
            ////register const __vector4 vFrom05 = pvFrom[5];
            ////register const __vector4 vFrom06 = pvFrom[6];
            ////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            ////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            ////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            ////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            ////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            ////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            ////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            ////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            ////pvTo[1]  = vTemp01;
            ////pvTo[2]  = vTemp02;
            ////pvTo[3]  = vTemp03;
            ////pvTo[4]  = vTemp04;
            ////pvTo[5]  = vTemp05;
            ////pvTo[6]  = vTemp06;
            ////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
       //// }

        inline void Gain32( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain32( pTo, pFrom, vGain );
            //////SubGain32( &pTo[64], &pFrom[64], vGain );
            //////SubGain32( &pTo[128], &pFrom[128], vGain );
            //////SubGain32( &pTo[192], &pFrom[192], vGain );
        }

		////inline void SubGain4( F32* pTo, const F32* pFrom, __vector4& vGain )
       //// {
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            //////register const __vector4 vFrom01 = pvFrom[1];
            //////register const __vector4 vFrom02 = pvFrom[2];
            //////register const __vector4 vFrom03 = pvFrom[3];
            //////register const __vector4 vFrom04 = pvFrom[4];
            //////register const __vector4 vFrom05 = pvFrom[5];
            //////register const __vector4 vFrom06 = pvFrom[6];
            //////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            //////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            //////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            //////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            //////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            //////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            //////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            //////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            //////pvTo[1]  = vTemp01;
            //////pvTo[2]  = vTemp02;
            //////pvTo[3]  = vTemp03;
            //////pvTo[4]  = vTemp04;
            //////pvTo[5]  = vTemp05;
            //////pvTo[6]  = vTemp06;
            //////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
       //// }

        inline void Gain4( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain4( pTo, pFrom, vGain );
            //////SubGain4( &pTo[64], &pFrom[64], vGain );
            //////SubGain4( &pTo[128], &pFrom[128], vGain );
            //////SubGain4( &pTo[192], &pFrom[192], vGain );
        }

		inline void AddGain(F32* pTo, const F32* pFromA, const F32* pFromB, F32 fGainB)
        {
            for (U32 i=0; i<256; i++)
                pTo[i] = pFromA[i] + fGainB * pFromB[i];
        }
        inline void GainAddGain(F32* pTo, const F32* pFromA, F32 fGainA, const F32* pFromB, F32 fGainB)
        {
            for (unsigned short i=0; i<256; i++)
                pTo[i] = fGainA * pFromA[i] + fGainB * pFromB[i];
        }
        inline void GainRamp(F32* pTo, const F32* pFrom, F32 fGain1, F32 fGain2,U32 frameLength = Audio::Core::uFrameLength)
        {
			const F32 fStep = (fGain2 - fGain1) / (F32)Audio::Core::uFrameLength;
			F32 fSampleStep(0.f);

			for (U32 uSample = 0; uSample < Audio::Core::uFrameLength; uSample++, fSampleStep+=fStep)
				*pTo++ = *pFrom++ * (fGain1 + fSampleStep);
		}
        inline void AddGainRamp(F32* pTo, const F32* pFromA, const F32* pFromB, F32 fGainB1, F32 fGainB2)
        {
			const F32 fStep = (fGainB2 - fGainB1) / (F32)Audio::Core::uFrameLength;
			F32 fSampleStep(0.f);

			for (U32 uSample=0; uSample < Audio::Core::uFrameLength; uSample++, fSampleStep+=fStep)
				*pTo++ = *pFromA++ + *pFromB++ * (fGainB1 + fSampleStep);
		}
        inline void Prefetch(const F32* pFrom)
		{
		}
        inline void ContinuousSRC(F32* pTo, F32* pFrom, U32 uLength, F32 fFirst)
        {
   	        F32* afDataIn = pFrom;
   	        F32* afDataOut = pTo;

            S32 sIndex = 0;
            F64 fFraction = 0.0;

            F64 fStep = Core::fFrameLength / (F32)uLength;

            for (U32 uCount = 0; uCount < uLength; uCount++)
            {
                F32 fSampleA = (sIndex == 0) ? fFirst : afDataIn[sIndex - 1];
                F32 fSampleB = afDataIn[sIndex];
                F32 fDiff = fSampleB - fSampleA;
                F32 fDelta = fDiff * (F32)fFraction;
                F32 fValue = fSampleA + fDelta;

                afDataOut[uCount] = fValue;

                fFraction += fStep;
                if (fFraction >= 1.0)
                {
                    S32 sInc = (S32)fFraction;
                    fFraction -= (F64)sInc;
                    sIndex += sInc;
                }
            }
        }

#endif // SN_TARGET_PS3

#if defined (PLATFORM_SPU)

#if !defined (AUDIO_USE_NEW_MATHS)
        inline void Zero(F32* pTo)
        {
			memset(pTo, 0, sizeof(float) * Audio::Core::uFrameLength);
		}

		inline void Zero(F32* pTo, U32 channelCount)
        {
			memset(pTo, 0, sizeof(float) * Audio::Core::uFrameLength * channelCount);
        }

		inline void Copy(F32* pTo, const F32* pFrom)
        {
			memcpy(pTo, pFrom, sizeof(float) * Audio::Core::uFrameLength);
        }
#endif // (AUDIO_USE_NEW_MATHS)

		inline void SubGain( F32* pTo, const F32* pFrom, vector float& vGain )
        {
            vector float* pvTo = (vector float*)pTo;
            vector float const* pvFrom = (vector float*)pFrom;

            register const vector float vFrom00 = pvFrom[0];
            register const vector float vFrom01 = pvFrom[1];
            register const vector float vFrom02 = pvFrom[2];
            register const vector float vFrom03 = pvFrom[3];
            register const vector float vFrom04 = pvFrom[4];
            register const vector float vFrom05 = pvFrom[5];
            register const vector float vFrom06 = pvFrom[6];
            register const vector float vFrom07 = pvFrom[7];
            register const vector float vFrom08 = pvFrom[8];
            register const vector float vFrom09 = pvFrom[9];
            register const vector float vFrom10 = pvFrom[10];
            register const vector float vFrom11 = pvFrom[11];
            register const vector float vFrom12 = pvFrom[12];
            register const vector float vFrom13 = pvFrom[13];
            register const vector float vFrom14 = pvFrom[14];
            register const vector float vFrom15 = pvFrom[15];

			register vector float vTemp00 = spu_mul(vFrom00, vGain);
            register vector float vTemp01 = spu_mul(vFrom01, vGain);
            register vector float vTemp02 = spu_mul(vFrom02, vGain);
            register vector float vTemp03 = spu_mul(vFrom03, vGain);
            register vector float vTemp04 = spu_mul(vFrom04, vGain);
            register vector float vTemp05 = spu_mul(vFrom05, vGain);
            register vector float vTemp06 = spu_mul(vFrom06, vGain);
            register vector float vTemp07 = spu_mul(vFrom07, vGain);
            register vector float vTemp08 = spu_mul(vFrom08, vGain);
            register vector float vTemp09 = spu_mul(vFrom09, vGain);
            register vector float vTemp10 = spu_mul(vFrom10, vGain);
            register vector float vTemp11 = spu_mul(vFrom11, vGain);
            register vector float vTemp12 = spu_mul(vFrom12, vGain);
            register vector float vTemp13 = spu_mul(vFrom13, vGain);
            register vector float vTemp14 = spu_mul(vFrom14, vGain);
            register vector float vTemp15 = spu_mul(vFrom15, vGain);

			pvTo[0]  = vTemp00;
            pvTo[1]  = vTemp01;
            pvTo[2]  = vTemp02;
            pvTo[3]  = vTemp03;
            pvTo[4]  = vTemp04;
            pvTo[5]  = vTemp05;
            pvTo[6]  = vTemp06;
            pvTo[7]  = vTemp07;
            pvTo[8]  = vTemp08;
            pvTo[9]  = vTemp09;
            pvTo[10] = vTemp10;
            pvTo[11] = vTemp11;
            pvTo[12] = vTemp12;
            pvTo[13] = vTemp13;
            pvTo[14] = vTemp14;
            pvTo[15] = vTemp15;
        }

        inline void Gain( F32* pTo, const F32* pFrom, F32 fGain )
        {
			register vector float vGain = Maths::make_vector4( fGain, fGain, fGain, fGain );

            SubGain( pTo, pFrom, vGain );
            SubGain( &pTo[64], &pFrom[64], vGain );
            SubGain( &pTo[128], &pFrom[128], vGain );
            SubGain( &pTo[192], &pFrom[192], vGain );
        }

			////////////////////////////////////////////
		//various gain routines for different sized arrays
		/////////////////////////////////////////////
		  inline void Gain128( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain( pTo, pFrom, vGain );
            ////SubGain( &pTo[64], &pFrom[64], vGain );

        }

				//various gain routines for different sized arrays
		////inline void SubGain32( F32* pTo, const F32* pFrom, __vector4& vGain )
        ////{
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            ////register const __vector4 vFrom01 = pvFrom[1];
            ////register const __vector4 vFrom02 = pvFrom[2];
            ////register const __vector4 vFrom03 = pvFrom[3];
            ////register const __vector4 vFrom04 = pvFrom[4];
            ////register const __vector4 vFrom05 = pvFrom[5];
            ////register const __vector4 vFrom06 = pvFrom[6];
            ////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            ////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            ////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            ////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            ////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            ////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            ////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            ////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            ////pvTo[1]  = vTemp01;
            ////pvTo[2]  = vTemp02;
            ////pvTo[3]  = vTemp03;
            ////pvTo[4]  = vTemp04;
            ////pvTo[5]  = vTemp05;
            ////pvTo[6]  = vTemp06;
            ////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
       //// }

        inline void Gain32( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain32( pTo, pFrom, vGain );
            //////SubGain32( &pTo[64], &pFrom[64], vGain );
            //////SubGain32( &pTo[128], &pFrom[128], vGain );
            //////SubGain32( &pTo[192], &pFrom[192], vGain );
        }

		////inline void SubGain4( F32* pTo, const F32* pFrom, __vector4& vGain )
       //// {
            ////__vector4* /*__restrict*/ pvTo = (__vector4*)pTo;
            ////__vector4 const* /*__restrict*/ pvFrom = (__vector4*)pFrom;

            ////register const __vector4 vFrom00 = pvFrom[0];
            //////register const __vector4 vFrom01 = pvFrom[1];
            //////register const __vector4 vFrom02 = pvFrom[2];
            //////register const __vector4 vFrom03 = pvFrom[3];
            //////register const __vector4 vFrom04 = pvFrom[4];
            //////register const __vector4 vFrom05 = pvFrom[5];
            //////register const __vector4 vFrom06 = pvFrom[6];
            //////register const __vector4 vFrom07 = pvFrom[7];
            //////register const __vector4 vFrom08 = pvFrom[8];
            //////register const __vector4 vFrom09 = pvFrom[9];
            //////register const __vector4 vFrom10 = pvFrom[10];
            //////register const __vector4 vFrom11 = pvFrom[11];
            //////register const __vector4 vFrom12 = pvFrom[12];
            //////register const __vector4 vFrom13 = pvFrom[13];
            //////register const __vector4 vFrom14 = pvFrom[14];
            //////register const __vector4 vFrom15 = pvFrom[15];
            ////ReadWriteBarrier();

            ////register __vector4 vTemp00 = __vmulfp(vFrom00, vGain);
            //////register __vector4 vTemp01 = __vmulfp(vFrom01, vGain);
            //////register __vector4 vTemp02 = __vmulfp(vFrom02, vGain);
            //////register __vector4 vTemp03 = __vmulfp(vFrom03, vGain);
            //////register __vector4 vTemp04 = __vmulfp(vFrom04, vGain);
            //////register __vector4 vTemp05 = __vmulfp(vFrom05, vGain);
            //////register __vector4 vTemp06 = __vmulfp(vFrom06, vGain);
            //////register __vector4 vTemp07 = __vmulfp(vFrom07, vGain);
            //////register __vector4 vTemp08 = __vmulfp(vFrom08, vGain);
            //////register __vector4 vTemp09 = __vmulfp(vFrom09, vGain);
            //////register __vector4 vTemp10 = __vmulfp(vFrom10, vGain);
            //////register __vector4 vTemp11 = __vmulfp(vFrom11, vGain);
            //////register __vector4 vTemp12 = __vmulfp(vFrom12, vGain);
            //////register __vector4 vTemp13 = __vmulfp(vFrom13, vGain);
            //////register __vector4 vTemp14 = __vmulfp(vFrom14, vGain);
            //////register __vector4 vTemp15 = __vmulfp(vFrom15, vGain);
            ////
            ////pvTo[0]  = vTemp00;
            //////pvTo[1]  = vTemp01;
            //////pvTo[2]  = vTemp02;
            //////pvTo[3]  = vTemp03;
            //////pvTo[4]  = vTemp04;
            //////pvTo[5]  = vTemp05;
            //////pvTo[6]  = vTemp06;
            //////pvTo[7]  = vTemp07;
            //////pvTo[8]  = vTemp08;
            //////pvTo[9]  = vTemp09;
            //////pvTo[10] = vTemp10;
            //////pvTo[11] = vTemp11;
            //////pvTo[12] = vTemp12;
            //////pvTo[13] = vTemp13;
            //////pvTo[14] = vTemp14;
            //////pvTo[15] = vTemp15;
       //// }

        inline void Gain4( F32* pTo, const F32* pFrom, F32 fGain )
        {
            ////register __vector4 vGain = { fGain, fGain, fGain, fGain };

            ////__dcbt( 0x000, pFrom );
            ////__dcbt( 0x080, pFrom );
            ////__dcbt( 0x100, pFrom );
            ////__dcbt( 0x180, pFrom );
            ////__dcbt( 0x200, pFrom );
            ////__dcbt( 0x280, pFrom );
            ////__dcbt( 0x300, pFrom );
            ////__dcbt( 0x380, pFrom );
            ////
            ////#if defined( AUDIO_MATHS_VECTOR_OPTIMISE_FRAME ) // JASOND
            ////if ( pTo != pFrom )
            ////{
            ////    //if ( (U32)pTo & 0x7F )
            ////    //    DebugBreak();
            ////    void* __restrict pDst = (void*)pTo; // JASOND: Address for dcbz must be restricted or it will spill all registers onto the stack!
            ////    __dcbz( 0x000, pDst );
            ////    __dcbz( 0x080, pDst );
            ////    __dcbz( 0x100, pDst );
            ////    __dcbz( 0x180, pDst );
            ////    __dcbz( 0x200, pDst );
            ////    __dcbz( 0x280, pDst );
            ////    __dcbz( 0x300, pDst );
            ////    __dcbz( 0x380, pDst );
            ////}
            ////#endif
            ////
            ////SubGain4( pTo, pFrom, vGain );
            //////SubGain4( &pTo[64], &pFrom[64], vGain );
            //////SubGain4( &pTo[128], &pFrom[128], vGain );
            //////SubGain4( &pTo[192], &pFrom[192], vGain );
        }

        inline void SubAddGain( F32* pTo, const F32* pFromA, const F32* pFromB, const vector float& vGainB )
        {
            vector float* pvTo = (vector float*)pTo;
            vector float const* pvFromA = (vector float*)pFromA;
            vector float const* pvFromB = (vector float*)pFromB;

            register const vector float vFromA00 = pvFromA[0];
            register const vector float vFromA01 = pvFromA[1];
            register const vector float vFromA02 = pvFromA[2];
            register const vector float vFromA03 = pvFromA[3];
            register const vector float vFromA04 = pvFromA[4];
            register const vector float vFromA05 = pvFromA[5];
            register const vector float vFromA06 = pvFromA[6];
            register const vector float vFromA07 = pvFromA[7];
            register const vector float vFromA08 = pvFromA[8];
            register const vector float vFromA09 = pvFromA[9];
            register const vector float vFromA10 = pvFromA[10];
            register const vector float vFromA11 = pvFromA[11];
            register const vector float vFromA12 = pvFromA[12];
            register const vector float vFromA13 = pvFromA[13];
            register const vector float vFromA14 = pvFromA[14];
            register const vector float vFromA15 = pvFromA[15];

            register const vector float vFromB00 = pvFromB[0];
            register const vector float vFromB01 = pvFromB[1];
            register const vector float vFromB02 = pvFromB[2];
            register const vector float vFromB03 = pvFromB[3];
            register const vector float vFromB04 = pvFromB[4];
            register const vector float vFromB05 = pvFromB[5];
            register const vector float vFromB06 = pvFromB[6];
            register const vector float vFromB07 = pvFromB[7];
            register const vector float vFromB08 = pvFromB[8];
            register const vector float vFromB09 = pvFromB[9];
            register const vector float vFromB10 = pvFromB[10];
            register const vector float vFromB11 = pvFromB[11];
            register const vector float vFromB12 = pvFromB[12];
            register const vector float vFromB13 = pvFromB[13];
            register const vector float vFromB14 = pvFromB[14];
            register const vector float vFromB15 = pvFromB[15];

            register vector float vTemp00 = spu_madd(vFromB00, vGainB, vFromA00);
            register vector float vTemp01 = spu_madd(vFromB01, vGainB, vFromA01);
            register vector float vTemp02 = spu_madd(vFromB02, vGainB, vFromA02);
            register vector float vTemp03 = spu_madd(vFromB03, vGainB, vFromA03);
            register vector float vTemp04 = spu_madd(vFromB04, vGainB, vFromA04);
            register vector float vTemp05 = spu_madd(vFromB05, vGainB, vFromA05);
            register vector float vTemp06 = spu_madd(vFromB06, vGainB, vFromA06);
            register vector float vTemp07 = spu_madd(vFromB07, vGainB, vFromA07);
            register vector float vTemp08 = spu_madd(vFromB08, vGainB, vFromA08);
            register vector float vTemp09 = spu_madd(vFromB09, vGainB, vFromA09);
            register vector float vTemp10 = spu_madd(vFromB10, vGainB, vFromA10);
            register vector float vTemp11 = spu_madd(vFromB11, vGainB, vFromA11);
            register vector float vTemp12 = spu_madd(vFromB12, vGainB, vFromA12);
            register vector float vTemp13 = spu_madd(vFromB13, vGainB, vFromA13);
            register vector float vTemp14 = spu_madd(vFromB14, vGainB, vFromA14);
            register vector float vTemp15 = spu_madd(vFromB15, vGainB, vFromA15);

            pvTo[0]  = vTemp00;
            pvTo[1]  = vTemp01;
            pvTo[2]  = vTemp02;
            pvTo[3]  = vTemp03;
            pvTo[4]  = vTemp04;
            pvTo[5]  = vTemp05;
            pvTo[6]  = vTemp06;
            pvTo[7]  = vTemp07;
            pvTo[8]  = vTemp08;
            pvTo[9]  = vTemp09;
            pvTo[10] = vTemp10;
            pvTo[11] = vTemp11;
            pvTo[12] = vTemp12;
            pvTo[13] = vTemp13;
            pvTo[14] = vTemp14;
            pvTo[15] = vTemp15;
        }

        inline void AddGain( F32* pTo, const F32* pFromA, const F32* pFromB, const F32& fGainB )
        {
            const vector float vGainB = { fGainB, fGainB, fGainB, fGainB };
            SubAddGain( pTo, pFromA, pFromB, vGainB );
            SubAddGain( &pTo[64], &pFromA[64], &pFromB[64], vGainB );
            SubAddGain( &pTo[128], &pFromA[128], &pFromB[128], vGainB );
            SubAddGain( &pTo[192], &pFromA[192], &pFromB[192], vGainB );
        }

        inline void SubGainAddGain( F32* pTo, const F32* pFromA, const vector float& vGainA, const F32* pFromB, const vector float& vGainB )
        {
            vector float* pvTo = (vector float*)pTo;
            vector float const* pvFromA = (vector float*)pFromA;
            vector float const* pvFromB = (vector float*)pFromB;

            register const vector float vFromA00 = pvFromA[0];
            register const vector float vFromA01 = pvFromA[1];
            register const vector float vFromA02 = pvFromA[2];
            register const vector float vFromA03 = pvFromA[3];
            register const vector float vFromA04 = pvFromA[4];
            register const vector float vFromA05 = pvFromA[5];
            register const vector float vFromA06 = pvFromA[6];
            register const vector float vFromA07 = pvFromA[7];
            register const vector float vFromA08 = pvFromA[8];
            register const vector float vFromA09 = pvFromA[9];
            register const vector float vFromA10 = pvFromA[10];
            register const vector float vFromA11 = pvFromA[11];
            register const vector float vFromA12 = pvFromA[12];
            register const vector float vFromA13 = pvFromA[13];
            register const vector float vFromA14 = pvFromA[14];
            register const vector float vFromA15 = pvFromA[15];

            register const vector float vFromB00 = pvFromB[0];
            register const vector float vFromB01 = pvFromB[1];
            register const vector float vFromB02 = pvFromB[2];
            register const vector float vFromB03 = pvFromB[3];
            register const vector float vFromB04 = pvFromB[4];
            register const vector float vFromB05 = pvFromB[5];
            register const vector float vFromB06 = pvFromB[6];
            register const vector float vFromB07 = pvFromB[7];
            register const vector float vFromB08 = pvFromB[8];
            register const vector float vFromB09 = pvFromB[9];
            register const vector float vFromB10 = pvFromB[10];
            register const vector float vFromB11 = pvFromB[11];
            register const vector float vFromB12 = pvFromB[12];
            register const vector float vFromB13 = pvFromB[13];
            register const vector float vFromB14 = pvFromB[14];
            register const vector float vFromB15 = pvFromB[15];

            register vector float vTemp00 = spu_mul(vFromB00, vGainB);
            register vector float vTemp01 = spu_mul(vFromB01, vGainB);
            register vector float vTemp02 = spu_mul(vFromB02, vGainB);
            register vector float vTemp03 = spu_mul(vFromB03, vGainB);
            register vector float vTemp04 = spu_mul(vFromB04, vGainB);
            register vector float vTemp05 = spu_mul(vFromB05, vGainB);
            register vector float vTemp06 = spu_mul(vFromB06, vGainB);
            register vector float vTemp07 = spu_mul(vFromB07, vGainB);
            register vector float vTemp08 = spu_mul(vFromB08, vGainB);
            register vector float vTemp09 = spu_mul(vFromB09, vGainB);
            register vector float vTemp10 = spu_mul(vFromB10, vGainB);
            register vector float vTemp11 = spu_mul(vFromB11, vGainB);
            register vector float vTemp12 = spu_mul(vFromB12, vGainB);
            register vector float vTemp13 = spu_mul(vFromB13, vGainB);
            register vector float vTemp14 = spu_mul(vFromB14, vGainB);
            register vector float vTemp15 = spu_mul(vFromB15, vGainB);

            vTemp00 = spu_madd(vFromA00, vGainA, vTemp00);
            vTemp01 = spu_madd(vFromA01, vGainA, vTemp01);
            vTemp02 = spu_madd(vFromA02, vGainA, vTemp02);
            vTemp03 = spu_madd(vFromA03, vGainA, vTemp03);
            vTemp04 = spu_madd(vFromA04, vGainA, vTemp04);
            vTemp05 = spu_madd(vFromA05, vGainA, vTemp05);
            vTemp06 = spu_madd(vFromA06, vGainA, vTemp06);
            vTemp07 = spu_madd(vFromA07, vGainA, vTemp07);
            vTemp08 = spu_madd(vFromA08, vGainA, vTemp08);
            vTemp09 = spu_madd(vFromA09, vGainA, vTemp09);
            vTemp10 = spu_madd(vFromA10, vGainA, vTemp10);
            vTemp11 = spu_madd(vFromA11, vGainA, vTemp11);
            vTemp12 = spu_madd(vFromA12, vGainA, vTemp12);
            vTemp13 = spu_madd(vFromA13, vGainA, vTemp13);
            vTemp14 = spu_madd(vFromA14, vGainA, vTemp14);
            vTemp15 = spu_madd(vFromA15, vGainA, vTemp15);

            pvTo[0]  = vTemp00;
            pvTo[1]  = vTemp01;
            pvTo[2]  = vTemp02;
            pvTo[3]  = vTemp03;
            pvTo[4]  = vTemp04;
            pvTo[5]  = vTemp05;
            pvTo[6]  = vTemp06;
            pvTo[7]  = vTemp07;
            pvTo[8]  = vTemp08;
            pvTo[9]  = vTemp09;
            pvTo[10] = vTemp10;
            pvTo[11] = vTemp11;
            pvTo[12] = vTemp12;
            pvTo[13] = vTemp13;
            pvTo[14] = vTemp14;
            pvTo[15] = vTemp15;
        }

        inline void GainAddGain( F32* pTo, const F32* pFromA, F32 fGainA, const F32* pFromB, F32 fGainB )
        {
            const vector float vGainA = { fGainA, fGainA, fGainA, fGainA };
            const vector float vGainB = { fGainB, fGainB, fGainB, fGainB };

            SubGainAddGain( pTo, pFromA, vGainA, pFromB, vGainB );
            SubGainAddGain( &pTo[64], &pFromA[64], vGainA, &pFromB[64], vGainB );
            SubGainAddGain( &pTo[128], &pFromA[128], vGainA, &pFromB[128], vGainB );
            SubGainAddGain( &pTo[192], &pFromA[192], vGainA, &pFromB[192], vGainB );
        }

#ifndef MINIMAL_SPU_FRAME_FUNCTIONS
		inline void GainRamp(F32* pTo, const F32* pFrom, F32 fGain1, F32 fGain2, U32 frameLength = Audio::Core::uFrameLength)
        {
			//F32 fStep = (fGain2 - fGain1) / (F32)Audio::Core::uFrameLength;

			//for (U16 uSample = 0; uSample < Audio::Core::uFrameLength; uSample++)
			//	pTo[uSample] = pFrom[uSample] * (fGain1 + (F32)uSample * fStep);

            using namespace Audio::Maths;
            		    
		    const Vector4 vGain1 = MakeVector4( fGain1 );
		    const Vector4 vGain2 = MakeVector4( fGain2 );
		    
			const Vector4 vStep = ( vGain2 - vGain1 ) * MakeVector4( (F32)Audio::Core::fRcpFrameLength );
		    
	        const Vector4 vThirtyTwo = MakeVector4( g_VectorThirtyTwo );
	        
            Vector4 vSampleIndex_00_01_02_03 = MakeVector4( g_VectorFloat_00_01_02_03 );
            Vector4 vSampleIndex_04_05_06_07 = MakeVector4( g_VectorFloat_04_05_06_07 );
            Vector4 vSampleIndex_08_09_0A_0B = MakeVector4( g_VectorFloat_08_09_0A_0B );
            Vector4 vSampleIndex_0C_0D_0E_0F = MakeVector4( g_VectorFloat_0C_0D_0E_0F );
            Vector4 vSampleIndex_10_11_12_13 = MakeVector4( g_VectorFloat_10_11_12_13 );
            Vector4 vSampleIndex_14_15_16_17 = MakeVector4( g_VectorFloat_14_15_16_17 );
            Vector4 vSampleIndex_18_19_1A_1B = MakeVector4( g_VectorFloat_18_19_1A_1B );
            Vector4 vSampleIndex_1C_1D_1E_1F = MakeVector4( g_VectorFloat_1C_1D_1E_1F );
		    
			for ( U32 uSample = 0; uSample < Audio::Core::uFrameLength; uSample+=32 )
			{
				const Vector4 vFrom_00_01_02_03 = LoadAligned( &pFrom[uSample+0x00] );
				const Vector4 vFrom_04_05_06_07 = LoadAligned( &pFrom[uSample+0x04] );
				const Vector4 vFrom_08_09_0A_0B = LoadAligned( &pFrom[uSample+0x08] );
				const Vector4 vFrom_0C_0D_0E_0F = LoadAligned( &pFrom[uSample+0x0C] );
				const Vector4 vFrom_10_11_12_13 = LoadAligned( &pFrom[uSample+0x10] );
				const Vector4 vFrom_14_15_16_17 = LoadAligned( &pFrom[uSample+0x14] );
				const Vector4 vFrom_18_19_1A_1B = LoadAligned( &pFrom[uSample+0x18] );
				const Vector4 vFrom_1C_1D_1E_1F = LoadAligned( &pFrom[uSample+0x1C] );
				
				const Vector4 vRatio_00_01_02_03 = vGain1 + ( vSampleIndex_00_01_02_03 * vStep );
				const Vector4 vRatio_04_05_06_07 = vGain1 + ( vSampleIndex_04_05_06_07 * vStep );
				const Vector4 vRatio_08_09_0A_0B = vGain1 + ( vSampleIndex_08_09_0A_0B * vStep );
				const Vector4 vRatio_0C_0D_0E_0F = vGain1 + ( vSampleIndex_0C_0D_0E_0F * vStep );
				const Vector4 vRatio_10_11_12_13 = vGain1 + ( vSampleIndex_10_11_12_13 * vStep );
				const Vector4 vRatio_14_15_16_17 = vGain1 + ( vSampleIndex_14_15_16_17 * vStep );
				const Vector4 vRatio_18_19_1A_1B = vGain1 + ( vSampleIndex_18_19_1A_1B * vStep );
				const Vector4 vRatio_1C_1D_1E_1F = vGain1 + ( vSampleIndex_1C_1D_1E_1F * vStep );
				
                vSampleIndex_00_01_02_03 += vThirtyTwo;
                vSampleIndex_04_05_06_07 += vThirtyTwo;
                vSampleIndex_08_09_0A_0B += vThirtyTwo;
                vSampleIndex_0C_0D_0E_0F += vThirtyTwo;
                vSampleIndex_10_11_12_13 += vThirtyTwo;
                vSampleIndex_14_15_16_17 += vThirtyTwo;
                vSampleIndex_18_19_1A_1B += vThirtyTwo;
                vSampleIndex_1C_1D_1E_1F += vThirtyTwo;
				
				const Vector4 vOut_00_01_02_03 = vFrom_00_01_02_03 * vRatio_00_01_02_03;
				const Vector4 vOut_04_05_06_07 = vFrom_04_05_06_07 * vRatio_04_05_06_07;
				const Vector4 vOut_08_09_0A_0B = vFrom_08_09_0A_0B * vRatio_08_09_0A_0B;
				const Vector4 vOut_0C_0D_0E_0F = vFrom_0C_0D_0E_0F * vRatio_0C_0D_0E_0F;
				const Vector4 vOut_10_11_12_13 = vFrom_10_11_12_13 * vRatio_10_11_12_13;
				const Vector4 vOut_14_15_16_17 = vFrom_14_15_16_17 * vRatio_14_15_16_17;
				const Vector4 vOut_18_19_1A_1B = vFrom_18_19_1A_1B * vRatio_18_19_1A_1B;
				const Vector4 vOut_1C_1D_1E_1F = vFrom_1C_1D_1E_1F * vRatio_1C_1D_1E_1F;
				
				StoreAligned( vOut_00_01_02_03, &pTo[uSample+0x00] );
				StoreAligned( vOut_04_05_06_07, &pTo[uSample+0x04] );
				StoreAligned( vOut_08_09_0A_0B, &pTo[uSample+0x08] );
				StoreAligned( vOut_0C_0D_0E_0F, &pTo[uSample+0x0C] );
				StoreAligned( vOut_10_11_12_13, &pTo[uSample+0x10] );
				StoreAligned( vOut_14_15_16_17, &pTo[uSample+0x14] );
				StoreAligned( vOut_18_19_1A_1B, &pTo[uSample+0x18] );
				StoreAligned( vOut_1C_1D_1E_1F, &pTo[uSample+0x1C] );
		    }   
		}
        inline void AddGainRamp(F32* pTo, const F32* pFromA, const F32* pFromB, F32 fGainB1, F32 fGainB2)
        {
			//F32 fStep = (fGainB2 - fGainB1) / (F32)Audio::Core::uFrameLength;

			//for (U32 uSample = 0; uSample < Audio::Core::uFrameLength; uSample++)
			//	pTo[uSample] = pFromA[uSample] + pFromB[uSample] * (fGainB1 + (F32)uSample * fStep);

            using namespace Audio::Maths;
            		    
		    const Vector4 vGainB1 = MakeVector4( fGainB1 );
		    const Vector4 vGainB2 = MakeVector4( fGainB2 );
		    
			const Vector4 vStep = ( vGainB2 - vGainB1 ) * MakeVector4( (F32)Audio::Core::fRcpFrameLength );
		    
	        const Vector4 vThirtyTwo = MakeVector4( g_VectorThirtyTwo );
	        
            Vector4 vSampleIndex_00_01_02_03 = MakeVector4( g_VectorFloat_00_01_02_03 );
            Vector4 vSampleIndex_04_05_06_07 = MakeVector4( g_VectorFloat_04_05_06_07 );
            Vector4 vSampleIndex_08_09_0A_0B = MakeVector4( g_VectorFloat_08_09_0A_0B );
            Vector4 vSampleIndex_0C_0D_0E_0F = MakeVector4( g_VectorFloat_0C_0D_0E_0F );
            Vector4 vSampleIndex_10_11_12_13 = MakeVector4( g_VectorFloat_10_11_12_13 );
            Vector4 vSampleIndex_14_15_16_17 = MakeVector4( g_VectorFloat_14_15_16_17 );
            Vector4 vSampleIndex_18_19_1A_1B = MakeVector4( g_VectorFloat_18_19_1A_1B );
            Vector4 vSampleIndex_1C_1D_1E_1F = MakeVector4( g_VectorFloat_1C_1D_1E_1F );
		    
			for ( U32 uSample = 0; uSample < Audio::Core::uFrameLength; uSample+=32 )
			{
				const Vector4 vFromA_00_01_02_03 = LoadAligned( &pFromA[uSample+0x00] );
				const Vector4 vFromA_04_05_06_07 = LoadAligned( &pFromA[uSample+0x04] );
				const Vector4 vFromA_08_09_0A_0B = LoadAligned( &pFromA[uSample+0x08] );
				const Vector4 vFromA_0C_0D_0E_0F = LoadAligned( &pFromA[uSample+0x0C] );
				const Vector4 vFromA_10_11_12_13 = LoadAligned( &pFromA[uSample+0x10] );
				const Vector4 vFromA_14_15_16_17 = LoadAligned( &pFromA[uSample+0x14] );
				const Vector4 vFromA_18_19_1A_1B = LoadAligned( &pFromA[uSample+0x18] );
				const Vector4 vFromA_1C_1D_1E_1F = LoadAligned( &pFromA[uSample+0x1C] );
				
				const Vector4 vFromB_00_01_02_03 = LoadAligned( &pFromB[uSample+0x00] );
				const Vector4 vFromB_04_05_06_07 = LoadAligned( &pFromB[uSample+0x04] );
				const Vector4 vFromB_08_09_0A_0B = LoadAligned( &pFromB[uSample+0x08] );
				const Vector4 vFromB_0C_0D_0E_0F = LoadAligned( &pFromB[uSample+0x0C] );
				const Vector4 vFromB_10_11_12_13 = LoadAligned( &pFromB[uSample+0x10] );
				const Vector4 vFromB_14_15_16_17 = LoadAligned( &pFromB[uSample+0x14] );
				const Vector4 vFromB_18_19_1A_1B = LoadAligned( &pFromB[uSample+0x18] );
				const Vector4 vFromB_1C_1D_1E_1F = LoadAligned( &pFromB[uSample+0x1C] );
				
				const Vector4 vRatio_00_01_02_03 = vGainB1 + ( vSampleIndex_00_01_02_03 * vStep );
				const Vector4 vRatio_04_05_06_07 = vGainB1 + ( vSampleIndex_04_05_06_07 * vStep );
				const Vector4 vRatio_08_09_0A_0B = vGainB1 + ( vSampleIndex_08_09_0A_0B * vStep );
				const Vector4 vRatio_0C_0D_0E_0F = vGainB1 + ( vSampleIndex_0C_0D_0E_0F * vStep );
				const Vector4 vRatio_10_11_12_13 = vGainB1 + ( vSampleIndex_10_11_12_13 * vStep );
				const Vector4 vRatio_14_15_16_17 = vGainB1 + ( vSampleIndex_14_15_16_17 * vStep );
				const Vector4 vRatio_18_19_1A_1B = vGainB1 + ( vSampleIndex_18_19_1A_1B * vStep );
				const Vector4 vRatio_1C_1D_1E_1F = vGainB1 + ( vSampleIndex_1C_1D_1E_1F * vStep );
				
                vSampleIndex_00_01_02_03 += vThirtyTwo;
                vSampleIndex_04_05_06_07 += vThirtyTwo;
                vSampleIndex_08_09_0A_0B += vThirtyTwo;
                vSampleIndex_0C_0D_0E_0F += vThirtyTwo;
                vSampleIndex_10_11_12_13 += vThirtyTwo;
                vSampleIndex_14_15_16_17 += vThirtyTwo;
                vSampleIndex_18_19_1A_1B += vThirtyTwo;
                vSampleIndex_1C_1D_1E_1F += vThirtyTwo;
				
				const Vector4 vOut_00_01_02_03 = vFromA_00_01_02_03 + ( vFromB_00_01_02_03 * vRatio_00_01_02_03 );
				const Vector4 vOut_04_05_06_07 = vFromA_04_05_06_07 + ( vFromB_04_05_06_07 * vRatio_04_05_06_07 );
				const Vector4 vOut_08_09_0A_0B = vFromA_08_09_0A_0B + ( vFromB_08_09_0A_0B * vRatio_08_09_0A_0B );
				const Vector4 vOut_0C_0D_0E_0F = vFromA_0C_0D_0E_0F + ( vFromB_0C_0D_0E_0F * vRatio_0C_0D_0E_0F );
				const Vector4 vOut_10_11_12_13 = vFromA_10_11_12_13 + ( vFromB_10_11_12_13 * vRatio_10_11_12_13 );
				const Vector4 vOut_14_15_16_17 = vFromA_14_15_16_17 + ( vFromB_14_15_16_17 * vRatio_14_15_16_17 );
				const Vector4 vOut_18_19_1A_1B = vFromA_18_19_1A_1B + ( vFromB_18_19_1A_1B * vRatio_18_19_1A_1B );
				const Vector4 vOut_1C_1D_1E_1F = vFromA_1C_1D_1E_1F + ( vFromB_1C_1D_1E_1F * vRatio_1C_1D_1E_1F );
				
				StoreAligned( vOut_00_01_02_03, &pTo[uSample+0x00] );
				StoreAligned( vOut_04_05_06_07, &pTo[uSample+0x04] );
				StoreAligned( vOut_08_09_0A_0B, &pTo[uSample+0x08] );
				StoreAligned( vOut_0C_0D_0E_0F, &pTo[uSample+0x0C] );
				StoreAligned( vOut_10_11_12_13, &pTo[uSample+0x10] );
				StoreAligned( vOut_14_15_16_17, &pTo[uSample+0x14] );
				StoreAligned( vOut_18_19_1A_1B, &pTo[uSample+0x18] );
				StoreAligned( vOut_1C_1D_1E_1F, &pTo[uSample+0x1C] );
		    }   
		}

#endif
        inline void Prefetch(const F32* pFrom) {}

        static F32* g_pTempAddr = NULL;
#ifndef MINIMAL_SPU_FRAME_FUNCTIONS
        inline F32* TempAlloc(U32 uNumFrames)
        {
            Assert(g_pTempAddr == NULL);

            U32 uMask = Core::uFrameLength - 1;
            U32 uSize = uNumFrames * Core::uFrameLength;
            g_pTempAddr = new F32[uSize + Core::uFrameLength];
            U32 uAddr = (((U32)g_pTempAddr + uMask) & ~uMask);
            return (F32*)uAddr;
        }

        inline void TempFree(F32* pFrame)
        {
            Assert(g_pTempAddr != NULL);

            delete [] g_pTempAddr;
            g_pTempAddr = NULL;
        }
        inline void ContinuousSRC(F32* pTo, F32* pFrom, U32 uLength, F32 fFirst)
        {
   	        F32* afDataIn = pFrom;
   	        F32* afDataOut = pTo;

            S32 sIndex = 0;
            F64 fFraction = 0.0;

            F64 fStep = Core::fFrameLength / (F32)uLength;

            for (U32 uCount = 0; uCount < uLength; uCount++)
            {
                F32 fSampleA = (sIndex == 0) ? fFirst : afDataIn[sIndex - 1];
                F32 fSampleB = afDataIn[sIndex];
                F32 fDiff = fSampleB - fSampleA;
                F32 fDelta = fDiff * (F32)fFraction;
                F32 fValue = fSampleA + fDelta;

                afDataOut[uCount] = fValue;

                fFraction += fStep;
                if (fFraction >= 1.0)
                {
                    S32 sInc = (S32)fFraction;
                    fFraction -= (F64)sInc;
                    sIndex += sInc;
                }
            }
        }
#endif //MINIMAL_SPU_FRAME_FUNCTIONS

#endif // PLATFORM_PS3_SPU

    }
}

#endif // _FRAME_H_


