#ifndef CORE_ALGORITHMS_H
#define CORE_ALGORITHMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Core
{
	namespace Algorithms
	{
		// Clamp "Value" so it is never less than "Low", or greater than "High".

		template<typename T> __forceinline T Clamp(const T Value, const T Low, const T High)
		{
			return (Low > Value) ? Low : ((Value > High) ? High : Value);
		}

		// Wrap "Value" between "Low" and "High", will never equal "High", beware if "Value"
		// is outside twice the range of "Low" to "High", then the wrapped value will also be
		// outside of the "Low" to "High" range.

		template<typename T> __forceinline T Wrap(const T Value, const T Low, const T High)
		{
			return (Low > Value) ? (Value + (High - Low)) : ((Value > High) ? (Value - (High - Low)) : Value);
		}

		// Return the larger of "Lhs" and "Rhs".

		template<typename T> __forceinline T Max(const T& Lhs, const T& Rhs)
		{
			return (Rhs > Lhs) ? Rhs : Lhs;
		}

		// Return the smaller of "Lhs" and "Rhs".

		template<typename T> __forceinline T Min(const T& Lhs, const T& Rhs)
		{
			return (Lhs > Rhs) ? Rhs : Lhs;
		}

		// Return the square of "Value".

		template<typename T> __forceinline T Square(const T Value)
		{
			return Value * Value;
		}

		// Return the square of "Value", but persists the original sign of the number.

		template<typename T> __forceinline T SquarePersistSign(const T Value)
		{
			return (Value < 0) ? -(Value * Value) : (Value * Value);
		}

		template<> __forceinline float SquarePersistSign(const float Value)
		{
			return fabsf(Value) * Value;
		}

		// Return the cube of "Value".

		template<typename T> __forceinline T Cube(const T Value)
		{
			return Value * Value * Value;
		}

		// Linear interpolation between "Low" and "High" by "Time".

		template<typename T, typename S> __forceinline T Lerp(const T& Low, const T& High, const S Time)
		{
			return (T)(Low + (High - Low) * Time);
		}

		// Exchange the values of "Lhs" and "Rhs".

		template<typename T> __forceinline void Swap(T& Lhs, T& Rhs)
		{
			const T Tmp(Lhs);
			Lhs = Rhs;
			Rhs = Tmp;
		}

		// Set "Var" to "Value", then returns the previous value of "Var".

		template<typename T> __forceinline T PushAssign(T& Var, const T Value)
		{
			T Old(Var);
			Var = Value;
			return Old;
		}

		// Cast the input into a Dword, most useful with SetRenderStates which require a float input.

		template<typename T> __forceinline DWORD DwordCast(const T Value)
		{
			return *((DWORD*)&Value);
		}

		// Count the number of items in a fixed array.

		template<typename T> __forceinline int ArrayCount(const T& Array)
		{
			return sizeof(Array) / sizeof(Array[0]);
		}

		// Return the byte offset to "Member" in a structure.  Ex. MemberOffset<Vector4::vz>()

		template<void* const Member> int MemberOffset()
		{
			return (int)Member;
		}

		// Convert degrees to radians.

		__forceinline float DegToRad(const float Angle)
		{
			return Angle * (Core::Constants::kPi / 180.0f);
		}

		// Convert radians to degrees.

		__forceinline float RadToDeg(const float Angle)
		{
			return Angle * (180.0f / Core::Constants::kPi);
		}

		// Align to upper boundry

		template<typename T> __forceinline T AlignUpper(T Value, int Alignment)
		{
			return (T)((((int)Value) + (Alignment - 1)) & ~(Alignment - 1));
		}

		// Align to lower boundry

		template<typename T> __forceinline T AlignLower(T Value, int Alignment)
		{
			return (T)(((int)Value) & ~(Alignment - 1));
		}

		// Find the highest bit set in value

		__forceinline int HighestBitOf(int value)
		{
			unsigned int n = ((value - 0x100) >> 16) & 8;

			value <<= n; 

			unsigned int m = ((value - 0x1000) >> 16) & 4;

			n		+= m; 
			value	<<= m; 
			m		= ((value - 0x4000) >> 16) & 2;
			n		+= m; 
			value	= (value << m) >> 14;
			m		= 13 - n + (value & ~(value >> 1));

			return m;
		}

		// Find the sign of the value

		template<typename Type>
		__forceinline int Sign(const Type& value)
		{
			if (value < 0)
				return -1;
			else if (value > 0)
				return 1;
			else
				return 0;
		}

		// NormaliseAngle

		inline float NormaliseAngle(const float Angle)
		{
			return (((float)(((int)((Angle + Core::Constants::kPi) * (65536.0f / Core::Constants::k2Pi))) & 65535)) * (Core::Constants::k2Pi / 65536.0f)) - Core::Constants::kPi;
		}
	};
};

#endif // CORE_ALGORITHMS_H