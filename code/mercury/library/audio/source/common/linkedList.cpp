#include "linkedList.h"

//------------------------------------------------------------------------------
//  constructor
//------------------------------------------------------------------------------
template <class Object>
Audio::List<Object>::List()
{
    header = new ListNode<Object>;
    m_iSize = 0;
}

//------------------------------------------------------------------------------
//  copy constructor
//------------------------------------------------------------------------------
template <class Object>
Audio::List<Object>::List(const List<Object>& rhs)
{
    header = new ListNode<Object>;
    m_iSize = rhs.size();
    *this = rhs;
}

//------------------------------------------------------------------------------
//  destructor
//------------------------------------------------------------------------------
template <class Object>
Audio::List<Object>::~List()
{
    makeEmpty();
    delete header;
}

//------------------------------------------------------------------------------
//  last
//------------------------------------------------------------------------------
template <class Object>
Audio::Iterator<Object> Audio::List<Object>::last() const
{
    ListNode<Object>* node = header;

    while (node->next)

        node = node->next;

    return Iterator<Object>(node);
}

//------------------------------------------------------------------------------
//  insert
//------------------------------------------------------------------------------
template <class Object>
void Audio::List<Object>::insert(const Object& x, const Iterator<Object>& p)
{
    if (p.current != 0)
    {
        p.current->next = new ListNode<Object>(x, p.current->next);
        m_iSize++;
    }
}

//------------------------------------------------------------------------------
//  find
//------------------------------------------------------------------------------
template <class Object>
Audio::Iterator<Object> Audio::List<Object>::find(const Object& x) const
{
    ListNode<Object>* itr = header->next;

    while (itr != NULL && itr->element != x)
        itr = itr->next;

    return Iterator<Object>(itr);
}

//------------------------------------------------------------------------------
//  findPrevious
//------------------------------------------------------------------------------
template <class Object>
Audio::Iterator<Object> Audio::List<Object>::findPrevious(const Object& x) const
{
    ListNode<Object>* itr = header;

    while (itr->next != 0 && itr->next->element != x)
        itr = itr->next;

    return Iterator<Object>(itr);
}

//------------------------------------------------------------------------------
//  remove
//------------------------------------------------------------------------------
template <class Object>
void Audio::List<Object>::remove(const Object& x)
{
    Iterator<Object> p = findPrevious(x);

    if (p.current->next != 0)
    {
        ListNode<Object> *oldNode = p.current->next;
        p.current->next = p.current->next->next;  // Bypass deleted node

        delete oldNode;

        m_iSize--;
    }
}

//------------------------------------------------------------------------------
//  =
//------------------------------------------------------------------------------
template <class Object>
const Audio::List<Object>& Audio::List<Object>::operator = (const List<Object>& rhs)
{
    Iterator<Object> ritr = rhs.first();
    Iterator<Object> itr = zeroth();

    if (this != &rhs)
    {
        makeEmpty();
        for (; !ritr.isPastEnd(); ritr.advance(), itr.advance())
            insert(ritr.retrieve(), itr);
    }

    return *this;
}


