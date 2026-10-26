// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


/// \note if modifying this class please ensure that you only make use of
///  the less than operator [ < ] And not the greater than operator [ > ].

/// \note 2 It occurs to me that thermight be a better/quicker simpler way
///   to insert and remove nodes by simply having a pointer to the last red
///   that was passed as we marched down the tree.
///  This should cut down substantially on the number of rotations required
///   which would be nice.

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::bdRedBlackTreeIterator()
: m_tree(BD_NULL),
m_node(BD_NULL)
{
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::operator bdBool() const
{
	return m_node != BD_NULL;
}

template <typename T, typename LESS_THAN_FUNCTOR>
void bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::initialize(const bdRedBlackTree<T, LESS_THAN_FUNCTOR> *const tree)
{
	m_tree = tree;

	if(m_tree)
	{
		m_node = first();
	}
	else
	{
		m_stack.clear();
		m_node = BD_NULL;
	}
}

template <typename T, typename LESS_THAN_FUNCTOR>
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::first()
{
	m_node = BD_NULL;
	m_stack.clear();

	if(m_tree)
	{
		m_node = m_tree->m_rootNode;
		while(m_node->m_left != &m_tree->m_nullNode)
		{
			m_stack.pushBack(m_node);
			m_node = m_node->m_left;
		}

		if (m_node == &m_tree->m_nullNode)
			m_node = BD_NULL;
	}

	return m_node;
}

template <typename T, typename LESS_THAN_FUNCTOR>
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::next()
{
	Node* oldNode = m_node;

	// make sure the tree is not empty.
	if(m_tree && (m_node != BD_NULL))
	{
		m_node = m_node->m_right;

		while(m_node != &m_tree->m_nullNode)
		{
			m_stack.pushBack(m_node);
			m_node = m_node->m_left;
		}

		const bdUInt stackSize = m_stack.getSize();

		if(stackSize == 0)
		{
			m_node = BD_NULL;
		}
		else
		{
			m_node = m_stack[stackSize-1];
			m_stack.popBack();
		}
	}

	return oldNode;
}


//
// This section can be used to allow forward and backward traversal
// of the tree. The extra cost is that the stack will be deeper. To use these
// remove next from the class and replace with forward and backward. In the
// bdRedBlackTree::next call forward and create a symetric
// bdRedBlackTree::previous function. You can use last instead of first in
// initialize to set up for a reverse traverse.
//
/*
template <typename T, typename LESS_THAN_FUNCTOR>
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::last()
{

	m_node = BD_NULL;
	m_stack.clear();

	if(m_tree)
	{
		m_node = m_tree->m_rootNode;
		while(m_node->m_right != &m_tree->m_nullNode)
		{
			m_stack.pushBack(m_node);
			m_node = m_node->m_right;
		}

		if (m_node == &m_tree->m_nullNode)
			m_node = BD_NULL;
	}

	return m_node;
}

template <typename T, typename LESS_THAN_FUNCTOR>
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::backward()
{

	Node* oldNode = m_node;

	if(m_tree)
	{
		if(m_node->m_left != &m_tree->m_nullNode)
		{
			// something to the left so go left and then as
			// far right as possible
			m_stack.pushBack(m_node);
			m_node = m_node->m_left;

			while(m_node->m_right != &m_tree->m_nullNode)
			{
				m_stack.pushBack(m_node);
				m_node = m_node->m_right;
			}
		}
		else
		{
			// nothing to the right, and we have already visited the left
			// so step back up the tree

			Node* upNode = BD_NULL;

			do
			{
				const bdUInt stackSize = m_stack.getSize();
				if(stackSize == 0)
				{
					m_node = BD_NULL;
					break;
				}

				upNode = m_node;
				m_node = m_stack[stackSize - 1];
				m_stack.popBack();
			}
			while (upNode == m_node->m_left);
		}
	}

	return oldNode;
}
*/

template <typename T, typename LESS_THAN_FUNCTOR>
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::forward()
{
	Node* oldNode = m_node;

	if(m_tree)
	{
		if(m_node->m_right != &m_tree->m_nullNode)
		{
			// something to the right so go right and then as
			// far left as possible
			m_stack.pushBack(m_node);
			m_node = m_node->m_right;

			while(m_node->m_left != &m_tree->m_nullNode)
			{
				m_stack.pushBack(m_node);
				m_node = m_node->m_left;
			}
		}
		else
		{
			// nothing to the right, and we have already visited the left
			// so step back up the tree

			Node* upNode = BD_NULL;

			do
			{
				const bdUInt stackSize = m_stack.getSize();
				if(stackSize == 0)
				{
					m_node = BD_NULL;
					break;
				}

				upNode = m_node;
				m_node = m_stack[stackSize - 1];
				m_stack.popBack();
			}
			while (upNode == m_node->m_right);
		}
	}

	return oldNode;

}

template <typename T, typename LESS_THAN_FUNCTOR> inline
typename bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR>::getNode() const 
{
	return m_node;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdRedBlackTree<T, LESS_THAN_FUNCTOR>::bdRedBlackTree()
: m_rootNode(BD_NULL),
  m_size(0)
{
    m_nullNode.m_left = &m_nullNode;
    m_nullNode.m_right = &m_nullNode;

    m_rootNode = &m_nullNode;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdRedBlackTree<T, LESS_THAN_FUNCTOR>::~bdRedBlackTree()
{
    clear();
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::isEmpty() const
{
    return m_size == 0;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdUInt bdRedBlackTree<T, LESS_THAN_FUNCTOR>::getSize() const
{
    return m_size;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::clear()
{
    clearSubTree(m_rootNode);
	m_rootNode = &m_nullNode;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::insert(const T& data)
{	
	return insertTopDown (data);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::remove(const T& data)
{
	return removeTopDown(data);
}

template <typename T, typename LESS_THAN_FUNCTOR>
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::find(const T& data) const
{
    Node *node = m_rootNode;

	/// an quick breakdown of required operations.
	/// worst case scenario node is leaf at twice black depth.
	///  i.e. path is red-black-red-black etc....
	///  then required operations are
	///  black depth is log(n) where n is the size of tree
	///  2 less than calls per node traversal
	///  (blackDepth*2)*(lessThanFunctorCall*2)
	///
	/// total = 4 * log(sizeOfTree) * lessThanOperationTime.

	bdBool aLessThanB = false;
	bdBool bLessThanA = false;
    while(node != &m_nullNode)
    {
		aLessThanB = m_lessThan(data, node->m_data);
		bLessThanA = m_lessThan(node->m_data, data);

		// equal if a !< b && b !< a
		if(aLessThanB == false && bLessThanA == false)
		{
			// found the node.
			return true;
		}
		else if(aLessThanB == true)
		{
			// go to left child
			node = node->m_left;
		}
		else
		{
			// go to right child
			node = node->m_right;
		}
    }
    return false;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::initializeIterator(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const
{
    iterator.initialize(this);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
const T& bdRedBlackTree<T, LESS_THAN_FUNCTOR>::next(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const
{
	const Node *const node= iterator.next();
	return node->m_data;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
const T& bdRedBlackTree<T, LESS_THAN_FUNCTOR>::getAt(bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> &iterator) const
{
	const Node *const node= iterator.getNode();
	return node->m_data;
}

template <typename T, typename LESS_THAN_FUNCTOR>
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::checkTree() const
{
#ifdef BD_DEBUG
	// null node should always be black but it's not always.
	if(m_nullNode.m_color == RED)
	{
		return false;
	}
	const bdBool rootIsBlack = checkRootIsBlack();
	const bdBool blackDepthsEqual = checkBlackDepth();
	const bdBool redsOnlyHaveBlackKids = checkRedChildren();
	const bdBool correctNodeCount = checkNodeCount();
	checkBlackDepth();

	return rootIsBlack &&
		   blackDepthsEqual &&
		   redsOnlyHaveBlackKids &&
		   correctNodeCount;
#else
	return true;
#endif // BD_DEBUG
}

template <typename T, typename LESS_THAN_FUNCTOR>
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::insertTopDown(const T& data)
{
	bdBool result = false;

	// initialise the path marking pointers.
	{
		m_current = m_rootNode;
		m_parent = &m_nullNode;
		m_grand = &m_nullNode;
		m_great = &m_nullNode;
	}

	// Recolor m_rootNode's kids if required.
	{
		// if the two children of root are RED then we can color them BLACK
		//  without affecting the balance of the tree.
		// This is required to keep the tree working, and also because it is desirable to
		//  have less red nodes, as this leads to a tree with better balance.
		if ((m_rootNode->m_left->m_color == RED) && (m_rootNode->m_right->m_color == RED))
		{
			m_rootNode->m_left->m_color  = BLACK;
			m_rootNode->m_right->m_color = BLACK;
		}
	}

	// check if node already exists, add if it doesn't
	{
		bdBool aLessThanB = false;
		bdBool bLessThanA = false;

		// while loop to march down the tree.
		while(m_current != &m_nullNode )
		{
			aLessThanB = m_lessThan(data, m_current->m_data);
			bLessThanA = m_lessThan(m_current->m_data, data);
			
			// if a !< b && b !< a then a == b
			if(aLessThanB == false && bLessThanA == false)
			{
				return false;
			}
			else
			{
				// advance the path marking pointers.
				m_great = m_grand;
				m_grand = m_parent;
				m_parent = m_current;

				// decide which node to go to next.
				if(aLessThanB == true)
				{
					m_current = m_current->m_left ;
				}
				else // bLessThanA == true
				{
					m_current = m_current->m_right ;
				}
				
				// check the new current node and see if a reorient is required.
				if ((m_current->m_left->m_color == RED) && (m_current->m_right->m_color == RED))
				{
					// the reason we need to do a reorient here is that we don't have a black node
					//  at the next level, which might be a problem where we want to get a black
					//  node to add our new red node to.
					// Alternate explanation:
					//  If the two kids are red then it is possible that this subtree is entirely full.
					//  i.e. every black node has 2 red children
					//  this would make it impossible to add any more nodes to the subtree
					//  so we must reorient the tree when we detect this, just in case. eof. 
					//   [easiest thing is to ask, and I'll merrily dig out some diagrams ]
					reorient(data);
				}
			}
		}

		// add the node, if we've gotten this far.
		{
			// if we manage to exit the while loop then we haven't found the node
			//  so it is safe to add it.
			m_current = new Node(data, &m_nullNode, &m_nullNode);

			m_size++;

			// where the tree was previously empty we need only
			//  make m_rootNode point to the newly created node.
			if(m_rootNode == &m_nullNode)
			{
				m_rootNode = m_current;
			}
			else
			{
				// we can use this boolean because it now refers to 
				//  the old current, which is the parent of the new current.
				// It also cuts out a compare functor call.
				if(aLessThanB == true)
				{
					m_parent->m_left = m_current;
				}
				else
				{
					m_parent->m_right = m_current;
				}
				if(m_parent->m_color == BLACK)
				{
					// we can add red node without unbalancing the tree as long
					//  as it's parent is black.
					m_current->m_color = RED;
				}
				else
				{
					// otherwise we need to do a reorient.
					reorient(data);
				}
				// node has been added successfully, insofar as we can tell.
			}
			result = true;
		}	
	}
	return result;
}

template <typename T, typename LESS_THAN_FUNCTOR>
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::removeTopDown(const T& data)
{
	bdBool result = false;

	// these booleans will indicate the path we have taken down the tree
	bdBool parentIsLeftChild = false;
	bdBool currentIsLeftChild = false;

	// set the path travelled pointers
	m_great = &m_nullNode;
	m_grand = &m_nullNode;
	m_parent = &m_nullNode;
	m_current = m_rootNode;

	// indicates "the road not taken" [see Robert Frost]
	Node* sibling = &m_nullNode;

	// initial condition checks.
	{
		// if the tree is empty then we can't delete anything.
		if(m_rootNode == &m_nullNode)
		{
			return false;
		}
	}
	// initial setup
	{
		// we need to have a red node in the tree in order to delete
		//  a node, so one of m_rootNode, m_rootNode->m_left
		//  or m_rootNode->right must be red.

		//if both of roots children are black we can safely set root to be red.
		if ((m_rootNode->m_left->m_color == BLACK) &&
			(m_rootNode->m_right->m_color == BLACK))
		{
			m_rootNode->m_color = RED;
		}
		else
		{
			// check if the node we are going to next is black
			{
				// we may need to rotate the tree to get the red node onto the correct path.
				// this is very similar to the code at the bottom of the while loop.
				// it might therefore indicate that with some re organisaion of the while loop
				// this check & rotation could be avoided.

				// the best way to understand this code is to look at the
				//  way in which it will occur.
				// 1) It will fulfil these conditions iff:
				//    -> next node is black.
				//       -> which means the sibling is red
				//          -> siblings children must be black.
				//
				// Therefore after the rotation,
				//  - the sibling will be root of the subtree.
				//  - old root will be one of it's children.
				//
				//  - both children of old root will be black
				//    so ok to color old root red.
				//
				//  - and new root is made black

				bdBool dataLessThanRootData = m_lessThan(data, m_rootNode->m_data);
				if((m_rootNode->m_left->m_color == BLACK) && (dataLessThanRootData == true))
				{
					// make the old root red as it will soon be rotated and be guaranteed
					//  to have 2 black children.
					m_rootNode->m_color = RED;
					rotateWithRightChild(m_rootNode);

					// the new root is now the parent
					m_parent = m_rootNode;
					currentIsLeftChild = true;

					// make the new root black to make a valid tree.
					m_rootNode->m_color = BLACK;
				}
				else if((m_rootNode->m_right->m_color == BLACK) && ( dataLessThanRootData == false))
				{
					// make the old root red as it will soon be rotated and be guaranteed
					//  to have 2 black children.
					m_rootNode->m_color = RED;
					rotateWithLeftChild(m_rootNode);

					// the new root is now the parent
					m_parent = m_rootNode;
					currentIsLeftChild = false;

					// make the new root black to make a valid tree.
					m_rootNode->m_color = BLACK;
				}
			}
		}
	}

	// while loop to march down the tree.
	{
		// we will march until we arrive at a leaf node.
		while(	(m_current->m_left != &m_nullNode) ||
			(m_current->m_right != &m_nullNode))
		{
			// march one step further down the tree.
			{
				// advance pointers and update bool
				parentIsLeftChild = currentIsLeftChild;
				m_grand = m_parent;
				m_parent = m_current;

				// decide which branch of the tree to take.
				{
					
					/// Review this logic it's commentary, not adequate as an explanation
					///  even though I wrote it :)
					
					// this decision needs to take account of the fact that the appropriate
					//  path might be the null node.
					// Since we know that one of the nodes must be non null
					//  we can safely go one way if the other node is null.

					if ((m_lessThan(data, m_parent->m_data)) || (m_current->m_right == &m_nullNode))
					{
						sibling = m_current->m_right;
						m_current = m_current->m_left;
						currentIsLeftChild = true;
					}
					else
					{
						sibling = m_current->m_left;
						m_current = m_current->m_right;
						currentIsLeftChild = false;
					}
				}

				// swap data if necessary
				{
					// we check the value in the parent so that we can swap it
					//  without having to worry about keeping the tree navigable.
					//  - from this point until the function ends the tree is invalid
					//    because the node we have moved is placed on the wrong side
					//    of it's replacement.
					//    But our sub tree is ok so we will continue and the invalidity
					//    will dissappear as a result of our deleteing the target node
					// if a !< b && b !< a then a == b
					if((m_lessThan(m_parent->m_data, data) == false) && (m_lessThan(data, m_parent->m_data) == false))
					{
						swap(m_parent);
					}
				}
			}

			// check if the tree needs to be re-organised.
			{
				// if current node is red we can just keep marching.
				if (m_current->m_color == RED)
				{
					// The current node is red, in which case we can simply
					//  drop down to the next level in the tree.
					continue;
				}
				else
				{
					// We need to rejig the tree.
					{
						//  There are 2 initial possibilities.
						//  1) The current node has 2 black children.
						//  2) The current node has at least 1 red child.
						if ((m_current->m_left->m_color == BLACK) &&
							(m_current->m_right->m_color == BLACK)  )
						{
							// There is 1 possibility.
							if(sibling->m_color == BLACK)
							{
								if ((sibling->m_left->m_color  == BLACK)&&
									(sibling->m_right->m_color == BLACK)  )
								{
									// we can just push the red from the parent down to current
									// and sibling
									m_current->m_color = RED;
									// we won't change the null node.
									if(sibling != &m_nullNode)
									{
										sibling->m_color = RED;
									}
									m_parent->m_color = BLACK;
								}
								else
								{
									// at least one of siblings children is red
									bdBool leftSiblingIsRed = false;
									bdBool rightSiblingIsRed = false;

									// calculate the above booleans.
									{
										if (sibling->m_left->m_color == RED)
										{
											leftSiblingIsRed = true;
										}
										if (sibling->m_right->m_color == RED)
										{
											rightSiblingIsRed = true;
										}
									}
									// Sanity check
									{
										BD_ASSERT(leftSiblingIsRed || rightSiblingIsRed,
											"bdRedBlackTree<T, LESS_THAN_FUNCTOR>::remove, neither of the siblings children are red. "
											"Tree is invalid.");
									}
									// rotate the tree as required
									{
										//if grand is the null node then we can just rotate the parent.
										if(m_grand == &m_nullNode)
										{
											// make grand point to parent as the result of the rotation will be the new grand.
											m_grand = m_parent;

											if (currentIsLeftChild)
											{
												if (leftSiblingIsRed)
												{
													// start double rotate
													rotateWithLeftChild(m_parent->m_right);
													sibling = m_parent->m_right;
												}
												// rotate
												rotateWithRightChild(m_grand);
											}
											else
											{
												if (rightSiblingIsRed)
												{
													// start double rotate
													rotateWithRightChild(m_parent->m_left);
													sibling = m_parent->m_left;
												}

												// rotate
												rotateWithLeftChild(m_grand);
											}
											// at this point we want to re color nodes.
											{
												// we wish to paint the top of the sub tree red.
												// & paint it's children black
												m_grand->m_color = RED;
												m_grand->m_left->m_color = BLACK;
												m_grand->m_right->m_color = BLACK;
												m_current->m_color = RED;
											}


										}
										// we must worry about the tree wiring as we will rotate the m_grand->left/m_grand->right
										else
										{
											if (currentIsLeftChild)
											{
												if (leftSiblingIsRed)
												{
													// start double rotate
													rotateWithLeftChild(m_parent->m_right);
													sibling = m_parent->m_right;
												}

												if(parentIsLeftChild)
												{
													rotateWithRightChild(m_grand->m_left);
												}
												else
												{
													rotateWithRightChild(m_grand->m_right);
												}
											}


											else
											{
												if (rightSiblingIsRed)
												{
													// start double rotate
													rotateWithRightChild(m_parent->m_left);
													sibling = m_parent->m_left;
												}

												// rotate
												if(parentIsLeftChild)
												{
													rotateWithLeftChild(m_grand->m_left);
												}
												else
												{
													rotateWithLeftChild(m_grand->m_right);
												}
											}
											// set the node colors

											// at this point we want to re color nodes.
											{
												// we wish to paint the top of the sub tree red.
												// & paint it's children black

												//if (currentIsLeftChild)
												if (parentIsLeftChild)
												{
													m_grand->m_left->m_color = RED;
													m_grand->m_left->m_right->m_color = BLACK;
													m_grand->m_left->m_left->m_color = BLACK;
												}
												else
												{
													m_grand->m_right->m_color = RED;
													m_grand->m_right->m_right->m_color = BLACK;
													m_grand->m_right->m_left->m_color = BLACK;
												}
												m_current->m_color = RED;
											}


										}
									}
								}
							}
						}
						else
						{
							//  The current node has a red child.

							// at least one of currents children is red
							//  this means we can re jig the tree locally to
							//  get the required red node.
							// Again there are two possibilities
							// 1) the next node we are going to is red,
							//    - in which case we can drop down to that node.
							// 2) the next node we are going to is not red.

							/// unfortunately we need to know where we are going before we
							///  decide to go there as it makes re-jiging the tree easier.
							Node *next;
							Node *other;
							bdBool nextLeft;

							// figure out where to next.
							{
								if (m_lessThan(data, m_current->m_data))
								{
									nextLeft = true;
									next = m_current->m_left;
									other = m_current->m_right;
								}
								else
								{
									nextLeft = false;
									next = m_current->m_right;
									other = m_current->m_left;
								}
							}


							// if the next node is black then do rejigging.
							if(next->m_color == BLACK)
							{
								// rotate the tree appropriately
								{
									// this chunk of code simply decides whether to call
									// left or right rotate and which pointer to pass to
									//  keep the integrity of the tree.
									if(nextLeft)
									{
										if (currentIsLeftChild)
										{
											rotateWithRightChild(m_parent->m_left);
										}
										else
										{
											rotateWithRightChild(m_parent->m_right);
										}
									}
									else
									{
										if (currentIsLeftChild)
										{
											rotateWithLeftChild(m_parent->m_left);
										}
										else
										{
											rotateWithLeftChild(m_parent->m_right);
										}
									}
								}

								// set this boolean for the next looop.
								currentIsLeftChild = nextLeft;
								// then we set the colors of the rotated nodes.
								{
									m_current->m_color = RED;
									other->m_color = BLACK;
								}
								// and re jig the pointers
								{
									m_grand = m_parent;
									m_parent = other;
								}
							}
							else
							{
								// do nothing.
								continue;
							}
						}
					}
				}
		}
	}
	}
	// delete the node if it has been found
	{
		// if current is not the data we are looking for then
		//  the node is not in the tree
		
		// if a !< b && b !< a then a == b
		if(	(m_lessThan(data, m_current->m_data) == false) && 
			(m_lessThan(m_current->m_data, data) == false))
		{
			// the node to be deleted must be red, or it's deletion will
			//  invalidate the tree.
			BD_ASSERT(m_current->m_color == RED,
				"bdRedBlackTree<T, LESS_THAN_FUNCTOR>::remove, can only delete red nodes.");

			// redirect the appropriate pointer
			{
				if(m_current == m_rootNode)
				{
					m_rootNode = &m_nullNode;
				}
				else
				{	if(currentIsLeftChild)
					{
						m_parent->m_left = &m_nullNode;
					}
					else
					{
						m_parent->m_right = &m_nullNode;
					}
				}
			}

			delete m_current;

			m_size--;

			// set the result to indicate that we have deleted the node.
			result = true;
		}
	}

	// tidy up
	{
		m_rootNode->m_color = BLACK;

#ifdef BD_DEBUG		
		if(checkTree() == false)
		{
			// handy breakpoint
			// will allow us to see what exactly failed.
			//bdBool again = checkTree();
			checkTree();
			// this assert will be triggered where the tree
			//  has become invalid as a result of a remove 
			//  operation. Some thought might be required as to
			//  what might happen where this was release standard code
			//  but for the moment.
			// fixme  -- this code is a bit unusual, now that the Red Black Tree seems stable it can prolly be tidied..
			bdBool fail = false;
            BD_ASSERT(fail, "the red black tree has been rendered invalid by the previous operation, please tell EoF");
		}
#endif // BD_DEBUG
		
		m_grand = &m_nullNode;
		m_parent = &m_nullNode;
		m_current = &m_nullNode;
	}

	return result;
}

template <typename T, typename LESS_THAN_FUNCTOR> 
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::reorient(const T& data)
{
	// definite potential for re writing this section, it's a bit of a mess.
	//  but not today my friend

	// there will be obvious issues associated with the null node being used
	// namely when we are high enough in the tree to still have parent/grand/great as the nullnode.

	m_current->m_color = RED;
    m_current->m_left->m_color = BLACK;
    m_current->m_right->m_color = BLACK;

	// only need to rejig the tree if we have 2 red nodes in a row.
	if (m_parent->m_color == RED)
    {
        // two red in a row, have to rotate
        m_grand->m_color = RED;

        // true == left
        // false == right
        bdBool parentBranchTaken = false;
        bdBool grandBranchTaken = false;
        bdBool greatBranchTaken = false;

		// parent and grand can never be null where a reorient is called.
		parentBranchTaken = m_lessThan(data, m_parent->m_data);
		grandBranchTaken = m_lessThan(data, m_grand->m_data);
		
		// check if a double rotate is required.
		if (parentBranchTaken != grandBranchTaken)
		{
			if(parentBranchTaken)
			{
				rotateWithLeftChild(m_grand->m_right);
			}
			else
			{
				rotateWithRightChild(m_grand->m_left);
			}
		}
		if(m_great != &m_nullNode)
		{
			// then we need to worry about making sure the 
			//  tree is rewired correctly.
			greatBranchTaken = m_lessThan(data, m_great->m_data);
			if(grandBranchTaken)
			{
				if(greatBranchTaken)
				{
					rotateWithLeftChild(m_great->m_left);
					m_great->m_left->m_color = BLACK;
				}
				else
				{
					rotateWithLeftChild(m_great->m_right);
					m_great->m_right->m_color = BLACK;
				}
			}
			else
			{
				if(greatBranchTaken)
				{
					rotateWithRightChild(m_great->m_left);
					m_great->m_left->m_color = BLACK;
				}
				else
				{
					rotateWithRightChild(m_great->m_right);
					m_great->m_right->m_color = BLACK;
				}
			}
		}
		else
		{
			if(grandBranchTaken)
			{
				rotateWithLeftChild(m_grand);
				m_grand->m_color = BLACK;
			}
			else
			{
				rotateWithRightChild(m_grand);
				m_grand->m_color = BLACK;
			}
		}
    }


	// reset the root to black in case we have changed it previously
    m_rootNode->m_color = BLACK;
}

template <class T, typename LESS_THAN_FUNCTOR> inline
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::rotateWithLeftChild(Node* & k2)
{
    Node *k1 = k2->m_left;
    k2->m_left = k1->m_right;
    k1->m_right = k2;

    if(k2 == m_rootNode)
    {
        m_rootNode = k1;
    }

    k2 = k1;
}

template <class T, typename LESS_THAN_FUNCTOR> inline
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::rotateWithRightChild(Node* & k1)
{
    Node *k2 = k1->m_right;
    k1->m_right = k2->m_left;
    k2->m_left = k1;

    if(k1 == m_rootNode)
    {
        m_rootNode = k2;
    }

    k1 = k2;
}

template <typename T, typename LESS_THAN_FUNCTOR>
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::clearSubTree(Node *const node)
{
    if (node != &m_nullNode)
    {
        clearSubTree(node->m_left);
        clearSubTree(node->m_right);
        delete node;
        m_size--;
    }
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
typename bdRedBlackTree<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTree<T, LESS_THAN_FUNCTOR>::findMin(Node* root) const
{
    while (root->m_left != &m_nullNode)
    {
        root = root->m_left;
    }

    return root;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
typename bdRedBlackTree<T, LESS_THAN_FUNCTOR>::Node* bdRedBlackTree<T, LESS_THAN_FUNCTOR>::findMax(Node* root) const
{
    while (root->m_right != &m_nullNode)
    {
        root = root->m_right;
    }

    return root;
}

template <typename T, typename LESS_THAN_FUNCTOR> 
void bdRedBlackTree<T, LESS_THAN_FUNCTOR>::swap(Node *const node)
{

	// need to think about our swap function a bit.
	// Since we use the less than operator as a test.
	// we will go to the right where values are equal.
	// therefore the default behaviour of the swap function should
	// be to go right first so that it will use the appropriate subtree where
	// possible. Other wise we will get unfindable node syndrome. Just Ask, eof.
	// Hmmm, asking may no longer work... eof

	Node *replacement = &m_nullNode;

	if (node->m_right != &m_nullNode)
	{
		replacement = findMin(node->m_right);
	}
	else
	{
		replacement = findMax(node->m_left);
	}

	if(replacement != &m_nullNode)
	{
		const T other(node->m_data);
		node->m_data = replacement->m_data;
		replacement->m_data = other;
	}
}

#ifdef BD_DEBUG
template <typename T, typename LESS_THAN_FUNCTOR> 
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::checkRootIsBlack() const
{
	return m_rootNode->m_color == BLACK;
}
#endif // BD_DEBUG

#ifdef BD_DEBUG
template <typename T, typename LESS_THAN_FUNCTOR> 
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::checkBlackDepth() const
{
	bdBool allDepthsEqual = true;
	bdBool first = true;
	bdUInt firstBlackDepth = 0;
	bdUInt blackDepth = 0;

	bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> iterator;
	iterator.initialize(this);
	iterator.first();

	const bdUInt stackSize = iterator.m_stack.getSize();
	for(bdUInt i = 0; i < stackSize; i++)
	{
		if(iterator.m_stack[i]->m_color == BLACK)
		{
			blackDepth++;
		}
	}

	while(iterator)
	{
		const Node *const node = iterator.forward();

		// only check the leaf nodes
		// a leaf node is any node which does not have 2 kids
		if((node->m_left == &m_nullNode) || (node->m_right == &m_nullNode))
		{
			if(node->m_color == BLACK)
			{
				blackDepth++;
			}

			if(first)
			{
				first = false;
				firstBlackDepth = blackDepth;
			}
			else
			{
				if (blackDepth != firstBlackDepth)
				{
					allDepthsEqual = false;
					break;
				}
			}
		}

		blackDepth = 0;

		const bdUInt stackSize = iterator.m_stack.getSize();
		for(bdUInt i = 0; i < stackSize; i++)
		{
			if(iterator.m_stack[i]->m_color == BLACK)
			{
				blackDepth++;
			}
		}
	}

	return allDepthsEqual;
}
#endif // BD_DEBUG

#ifdef BD_DEBUG
template <typename T, typename LESS_THAN_FUNCTOR> 
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::checkRedChildren() const
{
	bdBool allBlackKids = true;

	bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> iterator;
	initializeIterator(iterator);

	while(iterator)
	{
		const Node *const node = iterator.next();

		if(node->m_color == RED)
		{
			if((node->m_left->m_color == RED) || (node->m_right->m_color == RED))
			{
				allBlackKids = false;
				break;
			}
		}
	}

	return allBlackKids;
}
#endif // BD_DEBUG

#ifdef BD_DEBUG
template <typename T, typename LESS_THAN_FUNCTOR> 
bdBool bdRedBlackTree<T, LESS_THAN_FUNCTOR>::checkNodeCount() const
{
	bdUInt nodeCount = 0;

	bdRedBlackTreeIterator<T, LESS_THAN_FUNCTOR> iterator;
	initializeIterator(iterator);

	while(iterator)
	{
		//const Node *const node = iterator.next();
		iterator.next();
		nodeCount++;
	}

	return nodeCount == getSize();
}
#endif // BD_DEBUG

