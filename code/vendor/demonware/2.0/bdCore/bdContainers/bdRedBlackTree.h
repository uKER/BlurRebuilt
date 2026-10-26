// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A templated red-black tree (balanced).

#ifndef BD_RED_BLACK_TREE_H
#define BD_RED_BLACK_TREE_H

#include <bdCore/bdMemory/bdMemory.h>

// need an array for the iterator.
#include <bdCore/bdContainers/bdFastArray.h>

// gives us access to comparison functors.
//  especially the bdLessThan functor.
#include <bdCore/bdUtilities/bdComparisonFunctors.h>

template <typename T, typename LESS_THAN_FUNCTOR>
class bdRedBlackTree;
// This class should be nested in bdRedBlackTree<T>, but as MSVC6 insists
// on having all the code at the definition, it has been moved outside the
// class to keep the header clean.
/// An iterator for the bdRedBlackTree class.
/// Provides an ordered iteration of a tree.
template <typename T, typename LESS_THAN_FUNCTOR = bdLessThan<T> >
class bdRedBlackTreeIterator
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Make the tree a friend of the iterator.
		friend class bdRedBlackTree<T, LESS_THAN_FUNCTOR>;

		/// Default constructor.
		inline bdRedBlackTreeIterator();

		/// Conversion operator so that we can use the iterator as a boolean.
		/// This will evaluate to true unless the \a m_node pointer points to
		/// \a BD_NULL.
		inline operator bdBool() const;

	protected:

		/// Set up the internal data structures so that the iterator will
		/// function correctly.
		/// \param tree[in] The red black tree to be iterated.
		void initialize(const bdRedBlackTree<T, LESS_THAN_FUNCTOR> *const tree);

		/// Typedef a red black tree node as a Node to make things more
		/// readable.
		typedef typename bdRedBlackTree<T, LESS_THAN_FUNCTOR>::Node Node;

		/// An accessor method for the first element in the tree.
		/// \return A pointer to the first node in the tree.
		Node* first();

		/// Move the internal pointer \a m_node onto the next node in the tree.
		/// \return A pointer to the current node in the tree.
		Node* next();

		/// Move the internal pointer \a m_node onto the next node in the tree.
		/// \return A pointer to the current node in the tree.
		/// \note This behaves much like next(), but stores a larger stack of
		/// nodes so it's possible to go forward and backward in the same
		/// iteration.
		/// \note Do not mix calls to bdRedBlackTreeIterator::next() and
		/// bdRedBlackTreeIterator::forward() within the same iteration.
		Node* forward();

		/// An accessor method for the current element in the tree.
		/// \return A pointer to the current node in the tree.
		/// \note unlike next() & forward() this function does not alter the
		///  iterator.
		inline Node* getNode() const ;

	protected:

		/// A pointer to the tree being iterated.
		const bdRedBlackTree<T, LESS_THAN_FUNCTOR>* m_tree;
		/// The node that the iteration is currently on.
		Node* m_node;
		/// An array to keep track of where the iteration has already been.
		bdFastArray<Node*> m_stack;
};

/// A balanced tree, implemented as a top down red black tree.
/// This tree will guarantee order log(n) insert and deletion
///  operations which makes it very well behaved for random data
///  and will also re-balance itself so it is ok with sequential
///  data.
/// Takes T, the key as a template parameter.
template <typename T, typename LESS_THAN_FUNCTOR = bdLessThan<T> >
class bdRedBlackTree
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Make the iterator a friend of the tree.
		friend class bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>;

	protected:

		/// A node in the tree can be either Black or Red.
		enum NodeColor
		{
			RED,
			BLACK
		};

		/// A tree is made of nodes. Each node contains pointers to
		///  it's left & right children.
		/// As well as a color and it's data of type T
		class Node
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				/// Default Constructor, with default arguments.
				/// \param data[in] The object to which the node refers.
				/// \param left[in] A pointer to the left child of the node.
				/// \param right[in] A pointer to the right child of the node.
				/// \param color[in] The color of the node.
				inline Node(const T& data = T(),
							Node *const left = BD_NULL,
							Node *const right = BD_NULL,
							const NodeColor color = BLACK)
				: m_data(data),
				  m_left(left),
				  m_right(right),
				  m_color(color)
				{
				}

				/// The data in the node.
				T m_data;

				/// A pointer to the left child of the node, if one exists
				Node *m_left;

				/// A pointer to the right child of the node, if one exists
				Node *m_right;

				/// The color of the node.
				NodeColor m_color;
		};

	public:

		//
		// Construction and destruction.
		//

		/// Default constructor.
		inline bdRedBlackTree();

		/// Destructor.
		inline ~bdRedBlackTree();

		/// Establish if the tree has any nodes in it.
		/// \return True if there are no nodes in the tree, false otherwise.
		inline bdBool isEmpty() const;

		/// Establish how many nodes there are in the tree.
		/// \return The number of nodes in the tree.
		inline bdUInt getSize() const;

		/// Remove all the nodes from the tree.
		/// This function will efficiently remove all the nodes from the tree.
		/// The motivation behind this function is that iterating through and
		/// removing all the nodes from the tree is horribly inefficient.
		inline void clear();

		/// Insert an object into the tree.
		/// \param data[in] A reference to the object to be inserted into the tree.
		/// \return
		///   - True if the object was successfully inserted.
		///   - False otherwise.
		/// \note A negative return value indicates that the object was
		///  already in the tree.
		inline bdBool insert(const T& data);

		/// Remove an object from the tree.
		/// \param data[in] A reference to the object to be removed from the tree.
		/// \return
		///   - True if the object was successfully removed.
		///   - False otherwise.
		/// \note A negative return value indicates that the object was
		///  not in the tree.
		inline bdBool remove(const T& data);

		/// Find an object in the tree.
		/// \param data[in] A reference to the object to be searched for.
		/// \return
		///   - True if the object was successfully found.
		///   - False otherwise.
		bdBool find(const T& data) const;

		/// Initialize the passed in iterator so that it operates on this tree.
		/// \param iterator[in] The iterator to initialize.
		inline void initializeIterator(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const;

		/// Move the iterator on to the next node in the tree.
		/// \param iterator[in] The iterator to use.
		/// \return A reference to the object in the current node.
		inline const T& next(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const;

		/// An accessor function for the object currently being pointed to by the iterator.
		/// \param iterator[in] The iterator to use.
		/// \return A reference to the object in the current node.
		inline const T& getAt(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const;

		/// Check if the tree still conforms to the rules describing a red black tree.
		/// \return
		///   - True if the tree is valid.
		///   - False otherwise.
		bdBool checkTree() const ;

	protected:

		/// Attempt to insert an object into the tree.
		/// Use the top down insertion algorithm to insert an object
		/// into the tree.
		/// \param data[in] The object to insert into the tree.
		/// \return
		///   - True if the object was inserted.
		///   - False otherwise.
		bdBool insertTopDown(const T& data);

		/// Attempt to remove an object from the tree.
		/// Use the top down removal algorithm to remove an object
		/// from the tree.
		/// \param data[in] The object to remove from the tree.
		/// \return
		///   - True if the object was removed.
		///   - False otherwise.
		bdBool removeTopDown(const T& data);

		/// Reorient the tree at the given node.
		/// \param data[in] The node that is at the top of the desired reorientation.
		/// \note A more in depth description of how this function behaves can be found
		///  in the implementation.
		void reorient(const T& data);

		/// Rotate the passed in pointer with it's left child.
		/// It performs some simple pointer re-wiring.
		/// \param k2[in] A pointer to the current node.
		inline void rotateWithLeftChild(Node* & k2);

		/// Rotate the passed in pointer with it's right child.
		/// It performs some simple pointer re-wiring.
		/// \param k1[in] A pointer to the current node.
		inline void rotateWithRightChild(Node* & k1);

		/// Remove and delete all the nodes in the passed in sub tree.
		/// A recursive function that calls itself on both of it's children
		/// and then deletes the node that was passed in.
		/// \param node [in] The node at the top of the sub tree that is to
		///  be cleared.
		void clearSubTree(Node *const node);

		/// Locate the smallest/leftmost node in the tree.
		/// \param root[in] A pointer to the top of the subtree to search.
		/// \return A pointer to the smallest node that was located.
		inline Node* findMin(Node* root) const;

		/// Locate the biggest/rightmost node in the tree.
		/// \param root[in] A pointer to the top of the subtree to search.
		/// \return A pointer to the biggest node that was located.
		inline Node* findMax(Node* root) const;

		/// Swap a node with the node next to it in value.
		/// Used to move nodes in the tree, primarily so that they can be
		/// easily deleted. It will swap with the nearest bigger node if one
		/// exists or else with the nearest smaller node.
		/// \param node[in] The node to swap.
		/// \note If called on a node with no children it will not alter the
		///  structure of the tree.
		void swap(Node *const node);

#ifdef BD_DEBUG
		/// Check if the tree still conforms to the rule that the root must be black.
		/// \return
		///   - True if the root is black.
		///   - False otherwise
		bdBool checkRootIsBlack() const;
#endif // BD_DEBUG

#ifdef BD_DEBUG
		/// Check if the tree still conforms to the rule describing black depth.
		/// Namely that all leaf nodes have the same black depth.
		/// \return
		///   - True if all leaf nodes have the same black depth.
		///   - False otherwise
		bdBool checkBlackDepth() const;
#endif // BD_DEBUG

#ifdef BD_DEBUG
		/// Check if the tree still conforms to the rule describing red children.
		/// Namely that a red node cannot have red children.
		/// \return
		///  - True if the tree has valid red children.
		///  - False otherwise.
		bdBool checkRedChildren() const;
#endif // BD_DEBUG

#ifdef BD_DEBUG
		/// Check if the internal counter \a m_size is correct.
		/// \return
		///  - True if the tree has \a m_size nodes.
		///  - False otherwise.
		bdBool checkNodeCount() const;
#endif // BD_DEBUG

	protected:

		/// A pointer to the root node of the tree.
		Node *m_rootNode;

		/// An empty node that is used to ensure that invalid node pointers
		///  do not point to garbage.
		Node m_nullNode;

		/// A counter to keep track of the number of nodes in the tree.
		bdUInt m_size;

		// The main benefit of these pointers is that we don't have to pass a load
		// of pointers to every helper function we might call.

		/// Transient pointer to the node currently being examined.
		/// \warning There is no guarantee that this will point to what might
		///  be expected based on it's name.
		Node *m_current;

		/// Transient pointer to the parent of m_current.
		/// \warning There is no guarantee that this will point to what might
		///  be expected based on it's name.
		Node *m_parent;

		/// Transient pointer to the parent of n_parent.
		/// \warning There is no guarantee that this will point to what might
		///  be expected based on it's name.
		Node *m_grand;

		/// Transient pointer to the parent of m_grand.
		/// \warning There is no guarantee that this will point to what might
		///  be expected based on it's name.
		Node *m_great;

		/// The functor that was specified in the template argument.
		LESS_THAN_FUNCTOR m_lessThan;
};

#include <bdCore/bdContainers/bdRedBlackTree.inl>

#endif // BD_RED_BLACK_TREE_H
