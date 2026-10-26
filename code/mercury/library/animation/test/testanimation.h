#include <core/core.h>
#include <core/maths.h>
#include <core/containers.h>
//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/tools/CompressedCurveUtils.h>
#include <animation/common/AnimationMaths.h>
#include <animation/common/StringUtil.h>
#include <animation/pose/PoseFormat.h>
#include <btl/utility/string.h>
#include <system/DebugLog.h>
#include <system/Misc/Random.h>
#include <cxxtest/TestSuite.h>

Debug::Logger& gLog = Debug::GetLogger( "mercury.animation.test" );


class AnimationTest: public CxxTest::TestSuite
{
public:

	void testPatternMatching()
	{
		using namespace Animation::StringUtil;
		
		TS_TRACE( "===== testPatternMatching() =====\n" );

		{
			const CHAR8* p = "*hello";
			
			TS_ASSERT(!Match("hello1", p));
			TS_ASSERT(Match("asd_hello", p));
			TS_ASSERT(!Match("asd_hello1", p));
			TS_ASSERT(Match("hellohello", p));
			TS_ASSERT(Match("123hello123hello", p));
		}

		{
			const CHAR8* p = "*HELLO";
			
			TS_ASSERT(!Match("hello1", p));
			TS_ASSERT(MatchAnyCase("asd_hello", p));
			TS_ASSERT(!MatchAnyCase("asd_hello1", p));
			TS_ASSERT(MatchAnyCase("hellohello", p));
			TS_ASSERT(Match("123hello123HELLO", p));
		}

		{
			const CHAR8* p = "abc";
			
			TS_ASSERT(Match("abc", p));
			TS_ASSERT(!Match("abcabc", p));
			TS_ASSERT(!Match("abc*", p));
			TS_ASSERT(!Match("aabcc", p));
			TS_ASSERT(!Match("abcc", p));
		}

		{
			const CHAR8* p = "*ello";
			
			TS_ASSERT(Match("hello", p));
			TS_ASSERT(Match("asd_hello", p));
			TS_ASSERT(Match("jello", p));
			TS_ASSERT(!Match("jello1", p));
		}

		{
			const CHAR8* p = "*hello*";
			
			TS_ASSERT(Match("hello", p));
			TS_ASSERT(Match("asd_hello", p));
			TS_ASSERT(!Match("jello", p));
			TS_ASSERT(Match("test_hello_test", p));
			TS_ASSERT(Match("hello_test", p));
			TS_ASSERT(Match("hellohello", p));
			TS_ASSERT(Match("hellohellohello", p));
			TS_ASSERT(Match("hellohellohellohello", p));
			TS_ASSERT(Match("123hello123hello", p));
		}
		
		{
			const CHAR8* p = "*abc*def";
			
			TS_ASSERT(Match("abcdef", p));
			TS_ASSERT(Match("123abc456def", p));
			TS_ASSERT(Match("123abcdef", p));
			TS_ASSERT(!Match("abcdef123", p));
			TS_ASSERT(Match("1abc2def", p));
			TS_ASSERT(!Match("1abc2def3", p));
		}
		
		{
			const CHAR8* p = "**abc";
			
			TS_ASSERT(Match("abc", p));
			TS_ASSERT(Match("abcabc", p));
			TS_ASSERT(Match("defabc", p));
			TS_ASSERT(!Match("abcdef", p));
		}	
		
		{
			const CHAR8* p = "*hello*1";
			
			TS_ASSERT(Match("hello1hello1", p));
			TS_ASSERT(!Match("hello12", p));
			TS_ASSERT(Match("hello21", p));
			TS_ASSERT(Match("hello21hello21", p));
		}			

		{
			const CHAR8* p = "***";
			
			TS_ASSERT(Match("a", p));
			TS_ASSERT(Match("", p));
			TS_ASSERT(Match("abc", p));
			TS_ASSERT(Match("def", p));
		}			

		{
			const CHAR8* p = "";
			
			TS_ASSERT(!Match("a", p));
			TS_ASSERT(Match("", p));
			TS_ASSERT(!Match("abc", p));
			TS_ASSERT(!Match("def", p));
		}	
		
		{
			const CHAR8* p = "prefix_*_suffix";
			
			TS_ASSERT(!Match("a", p));
			TS_ASSERT(Match("prefix_BLAH_suffix", p));
			TS_ASSERT(!Match("prefixprefix_BLAH_suffixsuffix", p));
			TS_ASSERT(Match("prefix_prefix_BLAH_suffix_suffix", p));
			TS_ASSERT(!Match("prefix_suffix", p));
			TS_ASSERT(Match("prefix__suffix", p));
		}			

	}
	
	void testPoseFormat()
	{
		TS_TRACE("==== testPoseFormat ===\n");
		
		Animation::C_PoseFormat pf;
		
		static const CHAR8* kFieldNames[] =
		{
			"E_Lighthouse_01group",
			"E_Lighthouse_02group",
			"E_Lighthouse_03group",
			"E_Lighthouse_04group",
			"E_Lighthouse_05group",
			"E_Lighthouse_06group",
			"E_Lighthouse_07group",
			"E_Lighthouse_08group",
			"E_Kamov",
			"E_explodingDebris_261",
			"E_explodingDebris_244",
			"E_explodingDebris_243",
			"E_explodingDebris_379",
			"E_explodingDebris_415",
			"E_explodingDebris_440",
			"E_explodingDebris_396",
			"E_explodingDebris_391",
			"E_explodingDebris_216",
			"E_explodingDebris_220",
			"E_explodingDebris_248",
			"E_explodingDebris_160",
			"E_explodingDebris_104",
			"E_explodingDebris_259",
			"E_explodingDebris_286",
			"E_explodingDebris_211",
			"E_explodingDebris_180",
			"E_explodingDebris_256",
			"E_explodingDebris_202",
			"E_explodingDebris_313",
			"E_explodingDebris_253",
			"E_explodingDebris_290",
			"E_explodingDebris_298",
			"E_explodingDebris_252"
		};
		
		// Populate string table with random strings.
		for (U32 comboIndex = 0; comboIndex < 64; ++comboIndex)
		{
			Btl::String128 str;
			Btl::String16 comboStr = Btl::String16::Format("_%3d", comboIndex);
			for (U32 fieldIndex = 0, count = sizeof(kFieldNames)/sizeof(kFieldNames[0]); fieldIndex < count; ++fieldIndex)
			{
				str = kFieldNames[fieldIndex];
				str += comboStr;
				Animation::C_StringId test(str.Chars());
			}
		}
				
		for (U32 fieldIndex = 0, count = sizeof(kFieldNames)/sizeof(kFieldNames[0]); fieldIndex < count; ++fieldIndex)
		{
			U32 i = pf.AddField(Animation::E_PoseFieldType_JointTransform, kFieldNames[fieldIndex]);
			TS_ASSERT(i == fieldIndex );
		}

		for (U32 fieldIndex = 0, count = sizeof(kFieldNames)/sizeof(kFieldNames[0]); fieldIndex < count; ++fieldIndex)
		{
			U32 i = pf.AddField(Animation::E_PoseFieldType_JointTransform, kFieldNames[fieldIndex]);
			TS_ASSERT(i == fieldIndex );
		}		
	}


	void testEulerConversion()
	{
		TS_TRACE( "===== testEulerConversion() =====\n" );
		
		// Maya rotations are anticlockwise when looking down the rotation axis towards the origin.
		// Game rotations are clockwise when looking down the axis.
		
		Maths::Vector3 euler = Maths::MakeVector3(
			Maths::DegToRad(50.f),
			Maths::DegToRad(-50.f),
			Maths::DegToRad(76.f)
		);
		
		
		Maths::Mat33 rotateX = Maths::MakeMat33_RotX(Maths::X(euler));
		Maths::Mat33 rotateY = Maths::MakeMat33_RotY(Maths::Y(euler));
		Maths::Mat33 rotateZ = Maths::MakeMat33_RotZ(Maths::Z(euler));
		
		//Maths::Mat33 xFlip = Maths::MakeMat33(-1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f);
		Maths::Mat33 rotateMatrix = rotateX * rotateY * rotateZ;
		
		Maths::Vector3 p = Maths::MakeVector3(0.f, 1.f, 0.f);
		p = p * rotateMatrix;
		
		Maths::Quaternion quatFromMatrix = Maths::MakeUnitQuaternion(rotateMatrix);
		
		Maths::Vector3 euler2;
		Animation::QuaternionToEuler(quatFromMatrix, euler2);
		euler2 = euler2 * Maths::MakeVF32(180.f / Maths::Pi);
		
		Maths::Quaternion quatFromEuler;
		Animation::QuaternionFromEuler(quatFromEuler, euler);
		
		Maths::VF32 error = quatFromEuler | quatFromMatrix;
		
		TS_ASSERT(error > Maths::MakeVF32(0.99f));
		
		Maths::Vector3 euler3;
		Animation::QuaternionToEuler(quatFromEuler, euler3);
		euler3 *= Maths::MakeVF32(180.f/Maths::Pi);
	}

	void testAnimationMaths()
	{
		TS_TRACE( "===== testAnimationMaths() =====\n" );
		
		
		// Maximum error is half a degree.
		F32 maxError = (Maths::Pi2 / 360.f) / 2.f;
		
		Util::C_Random r;
		
		Util::C_String str;
		for ( U32 i = 0; i < 50000; ++i )
		{
		
			F32 y = r.SignedFRand();
			F32 x = r.SignedFRand();
		
			F32 fastResult = Maths::GetF32( Animation::FastArcTan2( Maths::MakeVF32( y ), Maths::MakeVF32( x ) ) );
			F32 accurateResult = Maths::ArcTan( Maths::HF32( y ), Maths::HF32( x) );
			
			TS_ASSERT( Maths::Abs(fastResult - accurateResult) < maxError );
		}
	}

	void testQuaternionCompression()
	{
		TS_TRACE( "===== TestQuaternionCompression() =====\n" );

		// Test single axis quaternion compression.
		{
			F32 maxErrorInDegrees = 0.;
			F32 angleWithMaxErrorInDegrees = 0.f;
			F32 averageErrorInDegrees = 0.f;
			
			bool flipped = false;
			
			for ( S32 i = 0; i < 720; ++i )
			{
				F32 angle = Maths::DegToRad( static_cast<F32>( i ) );

				Maths::C_Quaternion q(
					sinf( angle * 0.5f ) * Maths::Constants< Maths::HVec3 >::UnitAxisPositiveY(),
					Maths::HVecW( cosf( angle * 0.5f ) ) );

				Maths::C_Quaternion q2;

				// Code is same for all axes.
				U16 quatY16;
				AnimationTools::CompressedCurveUtils::CompressSingleAxisQuaternion16( q, AnimationTools::CompressedCurveUtils::E_QuaternionAxis_Y, quatY16 );
				AnimationTools::CompressedCurveUtils::DecompressSingleAxisQuaternion16( quatY16, AnimationTools::CompressedCurveUtils::E_QuaternionAxis_Y, q2 );

				// (Don't) ignore changes in sign when calculating error.
				if ( ( q2 | q ) < Maths::Constants< Maths::HF32 >::Zero() )
				{
					flipped = true;
				//	q2 = -q2;
				}


				F32 errorInDegrees = Maths::RadToDeg( 2.f * acosf( Maths::Clamped( static_cast<F32>( ( q | q2 ) ), 0.f, 1.f ) ) );

				//LOGINFO( gLog, "QY16: angle %f, (Y, W) pre (%f,%f), (Y, W) post (%f, %f), error in degrees %g\n",
				//	Maths::RadToDeg( angle ),
				//	(F32) q.Imag().Y(),
				//	(F32) Maths::HF32( q.Real() ),
				//	(F32) q2.Imag().Y(),
				//	(F32) Maths::HF32( q2.Real() ),
				//	errorInDegrees
				//);

				averageErrorInDegrees += errorInDegrees;

				if ( errorInDegrees > maxErrorInDegrees )
				{
					angleWithMaxErrorInDegrees = static_cast<F32>( i );
					maxErrorInDegrees = errorInDegrees;
				}
			}

			averageErrorInDegrees /= 720.f;

			LOGINFO( gLog, "Q16: maximum error %g degrees at an angle of %g degrees, average error was %g degrees (%s)\n", maxErrorInDegrees, angleWithMaxErrorInDegrees, averageErrorInDegrees, flipped ? "introduced flip" : "no flips!" );

			// Verify error is within expected limits.
			TS_ASSERT( maxErrorInDegrees < AnimationTools::CompressedCurveUtils::kMaxSingleAxisQuaternion16ErrorInDegrees );
		}

	#if defined(DUMP_QUAT_ERRORS)
	#ifdef CFG_RELEASE
		const char* filename = "c:/quat32_trials_release.csv";
	#else
		const char* filename = "c:/quat32_trials_debug.csv";
	#endif
		FILE* fp = fopen( filename, "wt" );
	#endif
	
		// Test quaternion 32 and 48 compression.
		{
			Util::C_Random random;

			random.Seed( 5489UL );
			
			F32 maxErrorAngle32 = 0.f;

			F64 maxErrorInDegrees32 = 0.0;
			F64 maxErrorInDegrees48 = 0.0;
			
			F64 minCosineError32 = 1.0;

			F64 averageErrorInDegrees32 = 0.0;
			F64 averageErrorInDegrees48 = 0.0;

			S32 numTrials = 10000;
			
			bool flippedQ32 = false;
			bool flippedQ48 = false;
			
			S32 maxErrorIndex32 = 0;

			for ( S32 i = 0; i < numTrials; ++i )
			{
				
		
	#if 0
				F32 angle = 2.f * Maths::Pi2 * i / numTrials;
				static const F32 kOneOverRoot3 = 1.f / sqrtf( 3.f );
				Maths::C_Quaternion q( sinf( 0.5f * angle ) * Maths::HVec3( kOneOverRoot3, kOneOverRoot3, kOneOverRoot3 ), Maths::HVecW( cosf( 0.5f * angle ) ) );

	#else
				F32 angle = 2.f * Maths::Pi2 * random.FRand();
				Maths::C_Quaternion q(
					sinf( 0.5f * angle ) * random.SphereRand(),
					Maths::HVecW( cosf( 0.5f * angle ) ) );
	#endif

				Maths::C_Quaternion q2;
				Maths::C_Quaternion q3;

				U32 quat32;
				U16 quat48[3];
				AnimationTools::CompressedCurveUtils::CompressQuaternion32( q, quat32 );
				AnimationTools::CompressedCurveUtils::DecompressQuaternion32( quat32, q2 );

				AnimationTools::CompressedCurveUtils::CompressQuaternion48( q, quat48 );
				AnimationTools::CompressedCurveUtils::DecompressQuaternion48( quat48, q3 );
				
				MathsStorage::Float4 vQ;
				MathsStorage::Float4 vQ2;
				MathsStorage::Float4 vQ3;
				Maths::Store< Maths::C_Quaternion, MathsStorage::Float4>( q, &vQ );
				Maths::Store< Maths::C_Quaternion, MathsStorage::Float4>( q2, &vQ2 );
				Maths::Store< Maths::C_Quaternion, MathsStorage::Float4>( q3, &vQ3 );
							
				F64 qDotQ2 = static_cast< F32 >( q | q2 );
				F64 qDotQ3 = static_cast< F32 >( q | q3 );
				
				F64 qDotQ2_test = (F64) vQ.x*vQ2.x + (F64) vQ.y*vQ2.y + (F64) vQ.z*vQ2.z + (F64) vQ.w*vQ2.w;
				F64 qDotQ3_test = (F64) vQ.x*vQ3.x + (F64) vQ.y*vQ3.y + (F64) vQ.z*vQ3.z + (F64) vQ.w*vQ3.w;
				
				qDotQ2 = qDotQ2_test;
				qDotQ3 = qDotQ3_test;
				
				// Check for changes in sign
				if ( qDotQ2 < 0.f )
				{
					//q2 = -q2;
					flippedQ32 = true;
				}
				if ( qDotQ3 < 0.f )
				{
					//q3 = -q3;
					flippedQ48 = true;
				}
				
				F64 cosineError32 = Maths::Clamped( qDotQ2, 0.0, 1.0 );

				F64 errorInDegrees32 = Maths::RadToDeg( 2.0 * acos( cosineError32 ) );
				F64 errorInDegrees48 = Maths::RadToDeg( 2.0 * acos( Maths::Clamped( qDotQ3, 0.0, 1.0 ) ) );
				
	#if defined(DUMP_QUAT_ERRORS)
				fprintf( fp, "%g, %g\n", Maths::RadToDeg( angle ), errorInDegrees32 );
	#endif

				//fprintf( fp, "(%d) raw: (%f,%f,%f,%f) unpacked: (%f,%f,%f,%f) dot: %f\n",
				//	i,
				//	vQ.x, vQ.y, vQ.z, vQ.w,
				//	vQ2.x, vQ2.y, vQ2.z, vQ2.w,
				//	qDotQ2
				//);



	#if 0
				LOGINFO( gLog, "Q32: angle %f, pre (%f, %f, %f, %f), post (%f, %f, %f, %f), error %f degrees",
					Maths::RadToDeg( angle ),
					(F32) q.Imag().X(),
					(F32) q.Imag().Y(),
					(F32) q.Imag().Z(),
					(F32) Maths::HF32( q.Real() ),
					(F32) q2.Imag().X(),
					(F32) q2.Imag().Y(),
					(F32) q2.Imag().Z(),
					(F32) Maths::HF32( q2.Real() ),
					errorInDegrees32
				);
				LOGINFO( gLog, "Q64: angle %f, pre (%f, %f, %f, %f), post (%f, %f, %f, %f), error %f degrees",
					Maths::RadToDeg( angle ),
					(F32) q.Imag().X(),
					(F32) q.Imag().Y(),
					(F32) q.Imag().Z(),
					(F32) Maths::HF32( q.Real() ),
					(F32) q3.Imag().X(),
					(F32) q3.Imag().Y(),
					(F32) q3.Imag().Z(),
					(F32) Maths::HF32( q2.Real() ),
					errorInDegrees48
				);
	#endif
				averageErrorInDegrees32 += errorInDegrees32;
				averageErrorInDegrees48 += errorInDegrees48;

				if ( cosineError32 < minCosineError32 )
				{
					maxErrorIndex32 = i;
					maxErrorAngle32 = angle;
					//maxErrorInDegrees32 = errorInDegrees32;
					minCosineError32 = cosineError32;
				}

				if ( errorInDegrees48 > maxErrorInDegrees48 )
				{
					maxErrorInDegrees48 = errorInDegrees48;
				}
			}

	#if defined(DUMP_QUAT_ERRORS)
			fclose( fp );
	#endif

			averageErrorInDegrees32 /= static_cast<F64>( numTrials );
			averageErrorInDegrees48 /= static_cast<F64>( numTrials );
			
			maxErrorInDegrees32 = Maths::RadToDeg( 2.0 * acos( minCosineError32 ) );

			// Verify error is within expected limits.
			TS_ASSERT( static_cast< F32 >( maxErrorInDegrees32 ) < AnimationTools::CompressedCurveUtils::kMaxQuaternion32ErrorInDegrees );
			TS_ASSERT( static_cast< F32 >( maxErrorInDegrees48 ) < AnimationTools::CompressedCurveUtils::kMaxQuaternion48ErrorInDegrees );

			LOGINFO( gLog, "Q32: maximum error found was %g degrees (%d), average error was %g degrees (%s)\n", maxErrorInDegrees32, maxErrorIndex32, averageErrorInDegrees32, flippedQ32 ? "flipped" : "no flip" );
			LOGINFO( gLog, "Q48: maximum error found was %g degrees, average error was %g degrees (%s)\n", maxErrorInDegrees48, averageErrorInDegrees48, flippedQ48 ? "flipped" : "no flip" );
		}
	}

};


