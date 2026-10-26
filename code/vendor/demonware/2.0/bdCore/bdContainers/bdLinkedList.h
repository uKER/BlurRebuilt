// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A doubly linked list template class.

#ifndef BD_LINKED_LIST_H
#define BD_LINKED_LIST_H

#include <bdCore/bdMemory/bdMemory.h>

/// A doubly linked list template class.
/// All data placed into this linked list is copied, so the correct
/// constructors and destructors are called automatically.
/// \note This linked list class makes no attempt to be cache friendly. Nodes
/// are simply new'd and deleted as they are added and removed from the list.
template <typename T>
class bdLinkedList
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:

		/// A node in a bdLinkedList.
		/// These nodes are created and manipulated by bdLinkedList.
		class Node
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				/// Value constructor.
				/// Initializes the data stored at the Node to \a data.
				/// /param data[in]	The data to be stored at the Node.
				inline Node(const T& data)
				: m_data(data)
				{
				}

				/// The data stored at this node.
				T m_data;

				/// A pointer to the next node in the list.
				/// This will be equal to BD_NULL if this is the last node
				/// in the list.
				Node *m_next;

				/// A pointer to the previous node in the list.
				/// This will be equal to BD_NULL if this is the first node
				/// in the list.
				Node *m_prev;
		};

	public:

		//
		// Construction and destruction.
		//

		/// Default constructor.
		/// Creates an empty list, the list size will be zero and the head
		/// and tail equal to \c BD_NULL.
		inline bdLinkedList();

		/// Destructor.
		/// Cleans up and deallocates all memory associated with the list.
		inline ~bdLinkedList();

		//
		// Methods.
		//

		/// Check that there are entries in the list.
		/// \return
		/// -	True, if the list is empty (\c m_size == 0).
		/// -	False, if the list is not empty.
		inline bdBool isEmpty() const;

		/// Get the number of entries in the list.
		/// \return	The number of entries currently in the list.
		inline bdUInt getSize() const;

		/// Remove all entries from the list.
		/// Deletes all entries in the list, releasing any associated memory.
		/// \c ~T() will be on called on the data stored at each node.
		inline void clear();

		/// Get read and write access to the first entry in the list.
		/// \warning Do not call this function on an empty list.
		/// \return	A reference to the data stored at the first node of the
		///			list.
		inline T& getHead();

		/// Get read only access to the first entry in the list.
		/// \warning Do not call this function on an empty list.
		/// \return	A const reference to the data stored at the first node of
		///			the list.
		inline const T& getHead() const;

		/// Get read and write access to the last entry in the list.
		/// \warning	Do not call this function on an empty list.
		/// \return		A reference to the data stored at the last node of the
		///				list.
		inline T& getTail();

		/// Get read and write access to the last entry in the list.
		/// \warning	Do not call this function on an empty list.
		/// \return		A const reference to the data stored at the last node of
		///				the list.
		inline const T& getTail() const;

		/// Append an entry to the beginning of the list.
		/// A new Node is constructed with \a value and is wired to be the head
		/// of the list.
		/// \param	value[in]	The data to add into the list.
		inline void addHead(const T &value);

		/// Append an entry to the end of the list.
		/// A new Node is constructed with \a value and is wired to be the tail
		/// of the list.
		/// \param value[in]	The data to add into the list.
		inline void addTail(const T &value);

		/// Remove the first entry from the list.
		/// If it exists, the head entry in the list is deleted. Upon return
		/// the head of the list will be either the second entry or \c BD_NULL.
		inline void removeHead();

		/// Remove the last entry from the list.
		/// If it exists, the tail entry in the list is deleted. Upon return
		/// the tail of the list will be the penultimate entry in the list or
		/// \c BD_NULL.
		inline void removeTail();

		/// An iterator type.
		/// Defines an abstract type to allow easy iteration of the list.
		typedef void* Position;

		/// Get the \c Position of the first node in the list.
		/// \return	The \c Position of the first node in the list, or \c BD_NULL if
		///			the list is empty.
		inline Position getHeadPosition() const;

		/// Get the \c Position of the last node in the list.
		/// \return	The \c Position of the last node in the list, or \c BD_NULL if
		///			the list is empty.
		inline Position getTailPosition() const;

		/// Iterate forward through the list (non-const).
		/// Gets the data stored at the list entry identified by \a position.
		/// Also sets \a position to the \c Position of the next node in the list,
		/// or \c BD_NULL if \a position is the tail entry.
		/// \param position[in,out]	A reference to a \c Position returned by a
		///							call to forward(), backward(),
		///							getHeadPosition() or getTailPosition().
		/// \return	A reference to the data stored at \a position in the
		///			list. The data returned is that at \a position before it is
		///			stepped forward.
		inline T& forward(Position &position);

		/// Iterate forwards through the list (const).
		/// Gets the data stored at the list entry identified by \a position.
		/// Also sets \a position to the \c Position of the next node in the list,
		/// or \c BD_NULL if \a position is the tail entry.
		/// \param position[in,out]	A reference to a \c Position returned by a
		///							call to forward(), backward(),
		///							getHeadPosition() or getTailPosition().
		/// \return	A const reference to the data stored at \a position in
		///			the list. The data returned is that at \a position before
		///			it is stepped forward.
		inline const T& forward(Position &position) const;

		/// Iterate backwards through the list (non-const).
		/// Gets the data stored at the list entry identified by \a position.
		/// Also sets \a position to the \c Position of the previous node in the
		/// list, or \c BD_NULL if \a position is the head entry.
		/// \param position[in,out]	A reference to a \c Position returned by a
		///							call to forward(), backward(),
		///							getHeadPosition() or getTailPosition().
		/// \return		A reference to the data stored at \a position in the
		///				list. The data returned is that at \a position before
		///				it is stepped back.
		inline T& backward(Position &position);

		/// Iterate backwards through the list (const).
		/// Gets the data stored at the list entry identified by \a position.
		/// Also sets \a position to the \c Position of the previous node in
		/// the list, or \c BD_NULL if \a position is the head entry.
		/// \param position[in,out]	A reference to a \c Position returned by a
		///							call to forward(), backward(),
		///							getHeadPosition() or getTailPosition().
		/// \return		A const reference to the data stored at \a position in
		///				the list. The data returned is that at \a position
		///				before it is stepped back.
		inline const T& backward(Position &position) const;

		/// Search the list.
		/// Searches the list for the first entry after \a beginAfter whose data
		/// equals \a value.
		/// \warning	This function is O(n), and in order to use it
		///				\c T::operator== must be defined.
		/// \param value[in]		The data to search for.
		/// \param beginAfter[in]	The starting position of the search.
		/// \return	The \c Position of the matching node if found, \c BD_NULL
		///			otherwise.
		inline Position find(const T &value,
							 const Position beginAfter = BD_NULL) const;

		/// Get the data at a given \c Position (non-const).
		/// Returns the data stored at the list entry identified by
		/// \a position.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		/// \return	A reference to the data stored at \a position in the
		///			list.
		inline T& getAt(const Position position);

		/// Get the data at a given \c Position (const).
		/// Returns the data stored at the list entry identified by
		/// \a position.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		/// \return	A const reference to the data stored at \a position in the
		///			list.
		inline const T& getAt(const Position position) const;

		/// Set the data at a given \c Position.
		/// Sets the data stored at the list entry identified by
		/// \a position.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		/// \param value[in]	The data to be set.
		inline void setAt(const Position position,
						  const T &value);

		/// Removes a node from the list.
		/// Deletes the Node identified by \a position and rewires
		/// the list. After removal position is advanced to the next element in the list.
		/// \param position[in|out]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		inline void removeAt(Position &position);

		/// Insert a node into the list.
		/// Creates and inserts a Node into the list before the specified
		/// \c Position.
		/// If \a position is equal to \c BD_NULL the new node will be inserted
		/// before the current head of the list.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition(),
		///						getTailPosition() or \c BD_NULL.
		/// \param value[in]	The data to be set in the new node.
		inline void insertBefore(const Position position,
								 const T& value);

		/// Insert a list into the list.
		/// Merges the nodes of \a list into this list before the specified
		/// \c Position.
		/// If \a position is equal to \c BD_NULL the new node will be inserted
		/// before the current head of the list.
		/// \note After this operation \c list.m_head and \c list.m_tail are
		/// set to \c BD_NULL and \c list.m_size is set to zero. Hence ownership
		/// of any allocated memory is transfered to this object.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition(),
		///						getTailPosition()  or \c BD_NULL.
		/// \param list[in]		The bdLinkedList<T> to be merged.
		inline void insertBefore(const Position position,
								 bdLinkedList<T>& list);

		/// Insert a node into the list.
		/// Creates and inserts a Node into the list after the specified
		/// \c Position.
		/// If \a position is equal to \c BD_NULL the new node will be inserted
		/// after the current tail of the list.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition(),
		///						getTailPosition() or \c BD_NULL.
		/// \param value[in]	The data to be set in the new node.
		inline void insertAfter(const Position position,
								const T& value);

		/// Insert a list into the list.
		/// Merges the nodes of \a list into this list after the specified
		/// \c Position.
		/// If \a position is equal to \c BD_NULL the new node will be inserted
		/// after the current tail of the list.
		/// \note	After this operation \c list.m_head and \c list.m_tail are
		///			set to \c BD_NULL and \c list.m_size is set to zero. Hence
		///			ownership of any allocated memory is transfered to this
		///			object.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition(),
		///						getTailPosition() or \c BD_NULL.
		/// \param list[in]		The bdLinkedList to be merged.
		inline void insertAfter(const Position position,
								bdLinkedList<T>& list);

		/// Move a node to the front of the list.
		/// Moves the Node identified by \a position to the front
		/// of the list and rewires the list.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		inline void moveToHead(const Position position);

		/// Move a node to the end of the list.
		/// Moves the Node identified by \a position to the end
		/// of the list and rewires the list.
		/// \param position[in]	A \c Position returned by a call to forward(),
		///						backward(), getHeadPosition() or
		///						getTailPosition().
		inline void moveToTail(const Position position);

	protected:

		/// The first/head node in the list.
		Node *m_head;

		/// The last/tail node in the list.
		Node *m_tail;

		/// The number of nodes in the list.
		bdUInt m_size;
};

#include <bdCore/bdContainers/bdLinkedList.inl>

#endif // BD_LINKED_LIST_H
