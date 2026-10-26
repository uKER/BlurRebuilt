#ifndef MESSAGETYPERANGE_241701B2_AD9C_4561_B856_BC952DE95016_H
#define MESSAGETYPERANGE_241701B2_AD9C_4561_B856_BC952DE95016_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace Messaging
{

/// A inclusive range of message types [first,last]
struct C_MessageTypeRange
{
	TMsgType	m_First; ///< first message type in the range
	TMsgType	m_Last; ///< last message type in the range
};

extern const C_MessageTypeRange kFullMessageTypeRange;
extern const C_MessageTypeRange kInvalidMessageTypeRange;

inline bool Validate(const C_MessageTypeRange& range); ///< checks if this is a valid range

inline U32 GetSize(const C_MessageTypeRange& range); ///< gets the number of items in the range

inline bool Contains(const C_MessageTypeRange& range, const TMsgType value); ///< checks if the value is within the range
inline bool Contains(const C_MessageTypeRange& a, const C_MessageTypeRange& b); ///< checks if a wholly contains b
inline bool Overlaps(const C_MessageTypeRange& a, const C_MessageTypeRange& b); ///< checks if the ranges overlap

inline C_MessageTypeRange Union(const C_MessageTypeRange& a, const C_MessageTypeRange& b); ///< calculates the union of two ranges
inline C_MessageTypeRange Intersection(const C_MessageTypeRange& a, const C_MessageTypeRange& b); ///< calculates the intersection of two ranges


/// A set of ranges.
typedef Btl::Vector<C_MessageTypeRange> C_MessageTypeRanges;

bool Validate(const C_MessageTypeRanges& ranges); ///< checks that the ranges are valid

U32 CalcSize(const C_MessageTypeRanges& ranges); ///< calculates the total number of message types across all ranges

bool Contains(const C_MessageTypeRanges& ranges, const TMsgType value); ///< checks if this is within any of the ranges
bool Overlaps(const C_MessageTypeRanges& ranges, const C_MessageTypeRange& range); ///< checks if this overlaps any of the ranges

void Insert(C_MessageTypeRanges& dest, const TMsgType value); ///< add a type
void Insert(C_MessageTypeRanges& dest, const C_MessageTypeRange& range); ///< add a range of types
void Insert(C_MessageTypeRanges& dest, const C_MessageTypeRanges& ranges); ///< add ranges of types

} // namespace Messaging


#include <misc_shared/messaging/MessageTypeRange.inl>

#endif // MESSAGETYPERANGE_241701B2_AD9C_4561_B856_BC952DE95016_H
