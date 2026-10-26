#ifndef MESSAGETYPELIST_AB208365_D23C_4C74_A00F_170CD1018E1C_H
#define MESSAGETYPELIST_AB208365_D23C_4C74_A00F_170CD1018E1C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <misc_shared/messaging/MessageTypeRange.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace Messaging
{

/// Interface for visiting message types.
class I_MessageTypeVisitor
{
public:
	virtual ~I_MessageTypeVisitor();

	/**
	@brief Called for each message type to be visited.
	@param range range of message types.
	@returns true to continue or false to stop.
	*/
	virtual bool VisitMessageType(const C_MessageTypeRange& range) = 0;

};

/// Accumulates message types by visiting.
class C_MessageTypeListAccumulator : public I_MessageTypeVisitor
{
public:
	explicit C_MessageTypeListAccumulator(const U32 hintCapacity = 0);
	virtual ~C_MessageTypeListAccumulator();

	const C_MessageTypeRanges& GetResults() const {return m_Results;} ///< @returns results.

	virtual bool VisitMessageType(const C_MessageTypeRange& range); // from I_MessageTypeVisitor

private:
	C_MessageTypeRanges	m_Results; ///< results of visiting.

};

/// Interface for providing message type lists.
class I_MessageTypeList
{
public:
	virtual ~I_MessageTypeList();

	/**
	@returns number of message type ranges in the list, may include duplicates.
	*/
	virtual U32 CalcNumRanges() const = 0;

	/**
	@returns number of message types in the list, may include duplicates.
	*/
	virtual U32 CalcNumTypes() const = 0;

	/**
	@brief Searches the message type list.
	@param msgType message type to search for.
	@returns true if the message type is in the list.
	*/
	virtual bool Find(const TMsgType msgType) const = 0;

	/**
	@brief Visits messages types in a list.
	Message types are visited in arbitrary order and may contain duplicates.
	@param visitor object to visit message types.
	@returns false if the visitor stopped.
	*/
	virtual bool Visit(I_MessageTypeVisitor& visitor) const = 0;

};

/// An immutable list of message types.
class C_MessageTypeListFixed : public I_MessageTypeList
{
public:
	explicit C_MessageTypeListFixed(const C_MessageTypeRange* pMessageRanges, const I_MessageTypeList* pParent = NULL);
	C_MessageTypeListFixed(const C_MessageTypeListFixed& copy);
	
	virtual ~C_MessageTypeListFixed();

	C_MessageTypeListFixed& operator = (const C_MessageTypeListFixed& copy);

	// from I_MessageTypeList
	virtual U32 CalcNumRanges() const;
	virtual U32 CalcNumTypes() const;
	virtual bool Find(const TMsgType msgType) const;
	virtual bool Visit(I_MessageTypeVisitor& visitor) const;

private:
	const C_MessageTypeRange*	m_pMessageRanges; ///< weak ptr, never NULL, array of message ranges terminated by invalid range
	const I_MessageTypeList*	m_pParent; ///< weak ptr, can be NULL, a list of message types from which we inherit

};

/// A mutable list of message types.
class C_MessageTypeListVariable : public I_MessageTypeList
{
public:
	C_MessageTypeListVariable();
	C_MessageTypeListVariable(const C_MessageTypeListVariable& copy);

	virtual ~C_MessageTypeListVariable();

	C_MessageTypeListVariable& operator = (const C_MessageTypeListVariable& copy);

	void Insert(const TMsgType msgType); ///< add a message type.
	void Insert(const I_MessageTypeList& list); ///< add a list of message types.

	// from I_MessageTypeList
	virtual U32 CalcNumRanges() const;
	virtual U32 CalcNumTypes() const;
	virtual bool Find(const TMsgType msgType) const;
	virtual bool Visit(I_MessageTypeVisitor& visitor) const;

private:
	C_MessageTypeRanges	m_MessageTypeRanges; ///< message ranges in the list, no duplicates, sorted in ascending order.

};

} // namespace Messaging


#endif // MESSAGETYPELIST_AB208365_D23C_4C74_A00F_170CD1018E1C_H
