#ifndef GENESIS_LIST_H
#define GENESIS_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/utility/SmartPtr.h>

namespace Util
{

//////////////////////////////////////////////
// Double Linked List Class
//////////////////////////////////////////////

template < class T_Item > class C_Item;
template < class T_Item > class C_List;

template < class T_Item > class C_List
{
    friend class C_Item<T_Item>;
        
    public :
        
        typedef T_Item ItemType;
        
        class C_Iterator
        {
            public:
                
                C_Iterator( const C_List<T_Item>& list ) : m_Item( list.First() )  { }
                C_Iterator( const C_List<T_Item>* list ) : m_Item( list->First() ) { }
                
                C_Iterator( T_Item* pItem ) : m_Item( pItem )         {}
                ~C_Iterator()                                         {}
                
                operator T_Item& () const        { return *m_Item; }
                operator T_Item* () const        { return m_Item; }
                T_Item& operator * () const      { return *m_Item; }
                T_Item* operator -> () const     { return m_Item; }
                
                C_Iterator& operator = ( T_Item* pItem )
                {
                    m_Item = pItem;
                    return *this;
                }
                
                C_Iterator& operator = ( C_Iterator& iterator )
                {
                    return operator = ( (T_Item*)iterator );
                }
                
                T_Item* operator ++ ( ) // No int parameter defines this to be the prefix ++ operator!
                {
                    if ( m_Item )
                    {
                        m_Item = m_Item->Next();
                    }

                    return m_Item;
                }
                
                T_Item* operator -- ( ) // No int parameter defines this to be the prefix -- operator!
                {
                    if ( m_Item )
                    {
                        m_Item = m_Item->Prev();
                    }

                    return m_Item;
                }
                
                T_Item* operator ++ ( int ) // Int parameter defines this to be the postfix ++ operator!
                {
                    T_Item* result = m_Item;
                    if ( m_Item )
                    {
                        m_Item = m_Item->Next();
                    }
                    return result;
                }
                
                T_Item* operator -- ( int ) // Int parameter defines this to be the postfix -- operator!
                {
                    T_Item* result = m_Item;
                    if ( m_Item )
                    {
                        m_Item = m_Item->Prev();
                    }
                    return result;
                }
                
                T_Item* Next() const
                {
                    if ( m_Item )
                    {
                        return m_Item->Next();
                    }
                    else
                    {
                        return NULL;
                    }
                }
                
                T_Item* Prev() const
                {
                    if ( m_Item )
                    {
                        return m_Item->Prev();
                    }
                    else
                    {
                        return NULL;
                    }
                }
                
                T_Item*     Begin() const           { return m_pFirst; }
                T_Item*     End() const             { return NULL; }
                T_Item*     ReverseBegin() const    { return m_pLast; }
                T_Item*     ReverseEnd() const      { return NULL; }
                
            private:
                
                T_Item*           m_Item;
        };
        
        C_List();
        ~C_List();
        
        T_Item*             First() const       { return m_pFirst; }
        T_Item*             Last() const        { return m_pLast; }
        S32                 Elements() const    { return m_Elements; }
                            
        void                PushFirst( T_Item* pItem );
        void                PushLast( T_Item* pItem );
                            
        void                Remove( T_Item* pItem );
                            
        void                DeleteAll();
        void                RemoveAll();
                            
        T_Item*             PopFirst();
        T_Item*             PopLast();
                            
        void                InsertBefore( T_Item* pToInsert, T_Item* pListItem );
        void                InsertAfter( T_Item* pToInsert, T_Item* pListItem );

		// added to be more like Ut containers
		S32                 Size() const		{ return Elements(); }
		void                PushFront( T_Item* pItem ) { PushFirst(pItem); }
		void                PushBack( T_Item* pItem ) { PushLast(pItem); }
		T_Item*				PopFront( T_Item* pItem ) { return PopFirst(); }
		T_Item*				PopBack( T_Item* pItem ) { return PopLast(); }
   
    private :
          
        T_Item*             m_pFirst;
        T_Item*             m_pLast; 
        S32                 m_Elements;
};


template < class T_Item > class C_Item
{
    friend class C_List<T_Item>;
        
    public :
        
        C_Item() :
			m_pPrev( NULL ),
			m_pNext( NULL ),
			m_pList( NULL )
		{
		}
        
        ~C_Item()			   
        { 
        	if ( m_pList )
        	{
        		m_pList->Remove( (T_Item*)this ); 
        	}
        }
        
        C_List<T_Item>*     List() const { return m_pList; }        
        T_Item*             Prev() const { return m_pPrev; }
        T_Item*             Next() const { return m_pNext; }

        void RemoveFromList()	   
        { 
        	if ( m_pList )
        	{
        		m_pList->Remove( (T_Item*)this ); 
        	}
        }
    
    private :
        
        T_Item*             m_pPrev;
        T_Item*             m_pNext;
        C_List<T_Item>*     m_pList;
};
 

//
// List class methods
//

template < class T_Item >
C_List<T_Item>::C_List() :
    m_pFirst( NULL ),
    m_pLast( NULL ),
    m_Elements( 0 )
{
}


template < class T_Item >
C_List<T_Item>::~C_List()
{
    DeleteAll();
}


template < class T_Item >
void C_List<T_Item>::PushFirst( T_Item* pItem )
{
    assert( pItem != NULL );
    assert( pItem->List() == NULL );
    
    if ( m_pFirst )
    {
        m_pFirst->m_pPrev = pItem;
        pItem->m_pNext = m_pFirst;
    }
    else
    {
        m_pLast = pItem;
    }
    
    m_pFirst = pItem;
    
    pItem->m_pList = this;

    m_Elements += 1;
}


template < class T_Item >
void C_List<T_Item>::PushLast( T_Item* pItem )
{
    assert( pItem != NULL );
    assert( pItem->List() == NULL );
    
    if ( m_pLast )
    {
        m_pLast->m_pNext = pItem;
        pItem->m_pPrev = m_pLast;
    }
    else
    {
        m_pFirst = pItem;
    }
    
    m_pLast = pItem;
    
    pItem->m_pList = this;
    
    m_Elements += 1;
}


template < class T_Item >
void C_List<T_Item>::Remove( T_Item* pItem )
{
    assert( pItem != NULL );
    assert( pItem->List() == this );
    
    if ( m_pFirst == pItem )
    {
        m_pFirst = pItem->Next();
    }
    
    if ( m_pLast == pItem )
    {
        m_pLast = pItem->Prev();
    }
    
    T_Item* itemPrev = pItem->Prev();
    T_Item* itemNext = pItem->Next();
    
    if ( itemPrev )
    {
        itemPrev->m_pNext = itemNext;
    }
    
    if ( itemNext )
    {
        itemNext->m_pPrev = itemPrev;
    }
    
    pItem->m_pPrev = NULL;
    pItem->m_pNext = NULL;
    pItem->m_pList = NULL;
    
    m_Elements -= 1;
}




template < class T_Item >
void C_List<T_Item>::DeleteAll()
{
    while ( m_pFirst )
    {
        //delete PopFirst();
        delete m_pFirst;
    }
}



template < class T_Item >
void C_List<T_Item>::RemoveAll()
{
    while ( m_pFirst )
    {
        Remove( m_pFirst );
    }
}



template < class T_Item >
T_Item* C_List<T_Item>::PopFirst()
{
    if ( m_pFirst )
    {
        T_Item* pItem = m_pFirst;
        /// TODO: Implement proper PopFirst() rather than using Remove()
        Remove( pItem );
        return pItem;
    }
    else
    {
        return NULL;
    }
}


template < class T_Item >
T_Item* C_List<T_Item>::PopLast()
{
    if ( m_pLast )
    {
        T_Item* pItem = m_pLast;
        /// TODO: Implement proper PopLast() rather than using Remove()
        Remove( pItem );
        return pItem;
    }
    else
    {
        return NULL;
    }
}


template < class T_Item >
void C_List<T_Item>::InsertBefore( T_Item* pToInsert, T_Item* pListItem )
{
    assert( pToInsert != NULL );
    assert( pListItem != NULL );
    assert( pToInsert->List() == NULL );
    assert( pListItem->List() == this );
    
    T_Item* pListItemPrev = pListItem->m_pPrev;
    
    pToInsert->m_pPrev = pListItemPrev;
    pToInsert->m_pNext = pListItem;
    
    if ( pListItemPrev )
    {
        pListItemPrev->m_pNext = pToInsert;
    }
    
    pListItem->m_pPrev = pToInsert;
    
    if ( m_pFirst == pListItem )
    {
        m_pFirst = pToInsert;
    }
    
    pToInsert->m_pList = this;
    
    m_Elements += 1;
}


template < class T_Item >
void C_List<T_Item>::InsertAfter( T_Item* pToInsert, T_Item* pListItem )
{
    assert( pToInsert != NULL );
    assert( pListItem != NULL );
    assert( pToInsert->List() == NULL );
    assert( pListItem->List() == this );
    
    T_Item* pListItemNext = pListItem->m_pNext;
    
    pToInsert->m_pPrev = pListItem;
    pToInsert->m_pNext = pListItemNext;
    
    if ( pListItemNext )
    {
        pListItemNext->m_pPrev = pToInsert;
    }
    
    pListItem->m_pNext = pToInsert;
    
    if ( m_pLast == pListItem )
    {
        m_pLast = pToInsert;
    }
    
    pToInsert->m_pList = this;
    
    m_Elements += 1;
}        


template < class T_Stream, class T_Item >
inline T_Stream& operator << (T_Stream& stream, const C_List<T_Item>& in)
{
    U32 elements = in.Elements();
    stream << elements;
        
#ifdef COMPILER_GNU        
    for( C_List<T_Item>::C_Iterator.Begin(); C_List<T_Item>::C_Iterator != C_List<T_Item>::C_Iterator.End(); C_List<T_Item>::C_Iterator++ )
    {
        stream << C_List<T_Item>::C_Iterator;
    }
#else
    for( C_List<T_Item>::C_Iterator it = in.Begin(); it != in.End(); it++ )
    {
        stream << it;
    }
#endif
    return stream;
}

template < class T_Stream, class T_Item >
inline T_Stream& operator >> (T_Stream& stream, C_List<T_Item>& out)
{
    U32 elements;
    stream >> elements;

    for(U32 i=0; i<elements; i++)
    {
        T_Item* pNewItem = new T_Item();
        stream >> *pNewItem;
        out.PushLast(pNewItem);
    }
    return stream;
}

// Item helpers

template < class T_Item >
class C_PtrItem : public C_Item< C_PtrItem< T_Item > >
{
public:
    
    C_PtrItem( T_Item* pItem ) :
        m_pItem( pItem )
    {
    }
    
    ~C_PtrItem()
    {
    }

    operator T_Item* () const
    {
        return m_pItem;
    }

    T_Item* operator -> () const
    {
        return m_pItem;
    }
    
private:
    
    T_Item*       m_pItem;
};

template < class T_Item >
class C_MemberItem : public C_Item< C_MemberItem< T_Item > >
{
public:
    
    C_MemberItem()
    {
    }
    
    C_MemberItem( const T_Item& copy ) :
        m_Data( copy )
    {
    }
    
    ~C_MemberItem()
    {
    }
    
    operator T_Item& ()
    {
        return m_Data;
    }

    operator T_Item* ()
    {
        return &m_Data;
    }
        
    operator const T_Item& () const
    {
        return m_Data;
    }
    
    T_Item* operator -> ()
    {
        return &m_Data;
    }
    
    const T_Item* operator -> () const
    {
        return &m_Data;
    }
    
private:
    
    T_Item       m_Data;
};


// smart pointer item helpers

template < class T_Item >
class C_RefItem : public C_Item< C_RefItem< T_Item > >
{
public:
    
    C_RefItem( T_Item& item ) :
        m_Item( item )
    {
    }
    
    ~C_RefItem()
    {
    }

    operator T_Item* () const
    {
        return &m_Item;
    }

    T_Item* operator -> () const
    {
        return &m_Item;
    }
    
private:
    
    T_Item&       m_Item;
};


template < class T_Item >
class C_RefPtrItem : public C_Item< C_RefPtrItem< T_Item > >
{
public:
    
    C_RefPtrItem( T_Item* ptr ) :
        m_Ptr( ptr )
    {
    }
    
    C_RefPtrItem( const Sys::RefPtr<T_Item>& ptr ) :
        m_Ptr( ptr )
    {
    }
    
    ~C_RefPtrItem()
    {
        m_Ptr = NULL;
    }
    
    operator Sys::RefPtr<T_Item> () const
    {
        return m_Ptr;
    }
    
    T_Item* operator -> () const
    {
        return m_Ptr.operator -> ();
    }
    
private:
    
    Sys::RefPtr<T_Item>     m_Ptr;
};



template < class T_Item >
class C_WeakPtrItem : public C_Item< C_WeakPtrItem< T_Item > >
{
public:
    
    C_WeakPtrItem( T_Item* ptr ) :
        m_Ptr( ptr )
    {
    }
    
    C_WeakPtrItem( const Sys::WeakPtr<T_Item>& ptr ) :
        m_Ptr( ptr )
    {
    }
    
    ~C_WeakPtrItem()
    {
        m_Ptr = NULL;
    }
    
    operator Sys::WeakPtr<T_Item> () const
    {
        return m_Ptr;
    }
    
    T_Item* operator -> () const
    {
        return m_Ptr.operator -> ();
    }
    
    //T_Item* operator -> () /// Needed???
    //{
    //    return m_Ptr.operator -> ();
    //}
    
private:
    
    Sys::WeakPtr<T_Item>    m_Ptr;
};


}; // namespace Util

#endif
