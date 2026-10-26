#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

//  TODO: make list use a 'free' list

namespace Audio
{
//------------------------------------------------------------------------------
//  forward class definitions
//------------------------------------------------------------------------------
    template <class Object>
    class List;

    template <class Object>
    class Iterator;

//------------------------------------------------------------------------------
//  class ListNode
//------------------------------------------------------------------------------
    template <class Object>
    class ListNode
    {
        friend class List<Object>;
        friend class Iterator<Object>;

    public:
        ListNode(const Object& element = Object(), ListNode* n = 0) : element(element), next(n) {}

        Object   element;
        ListNode *next;
    };


//------------------------------------------------------------------------------
//  class List
//------------------------------------------------------------------------------
    template <class Object>
    class List
    {
    public:
        List();
        List(const List& rhs);
        ~List();

        inline bool isEmpty() const { return header->next == 0; }
        inline void makeEmpty() { while (!isEmpty()) remove(first().retrieve()); }
        inline Iterator<Object> zeroth() const { return Iterator<Object>(header); }
        inline Iterator<Object> first() const { return Iterator<Object>(header->next); }
        Iterator<Object> last() const;
        int size() const { return m_iSize; }

        void insert(const Object& x, const Iterator<Object>& p);
        Iterator<Object> find(const Object& x) const;
        Iterator<Object> findPrevious(const Object& x) const;
        void remove(const Object& x);

        const List& operator = (const List& rhs);

    private:
        int m_iSize;
        ListNode<Object>* header;
    };


//------------------------------------------------------------------------------
//  class Iterator
//------------------------------------------------------------------------------
    template <class Object>
    class Iterator
    {
        friend class List<Object>; // Grant access to constructor

    public:
        Iterator() : current(NULL) {}

        inline bool isPastEnd() const { return (current == 0); }
        inline void advance() { if (!isPastEnd()) current = current->next; }
        const Object& retrieve() const { if (isPastEnd()) throw 0; return current->element; }

        void replace(Object& element) { current->element = element; }
        void erase() { current->element = 0; }

    private:
        ListNode<Object>* current;    // Current position

        Iterator(ListNode<Object>* theNode) : current(theNode) {}
    };
}

#include "LinkedList.cpp"

#endif // _LINKED_LIST_H_

