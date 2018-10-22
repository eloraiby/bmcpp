#pragma once

#include <bmcpp/cpp-rt.hpp>

namespace BmCpp
{

template<typename T>
struct List : BaseAllocation
{
private:
    struct Node;
public:
    template<typename I>
    struct BaseIterator
    {
        BaseIterator()	: n(nullptr), owner(nullptr)	{}

        BaseIterator(const BaseIterator<I>& other) : n(other.n), owner(other.owner)	{}

        BaseIterator<I>&
        operator++()
        {
            n	= n->next;
            return *this;
        }

        BaseIterator<I>
        operator++(int)
        {
            BaseIterator<I>	tmp;
            tmp.n	= n;
            n	= n->next;
            return tmp;
        }

        BaseIterator<I>&
        operator--()
        {
            if( !n )
                n	= owner->tail;
            else
                n	= n->prev;
            return *this;
        }

        BaseIterator<I>
        operator--(int)
        {
            BaseIterator<I>	tmp;
            tmp.n	= n;
            if( !n )
                n	= owner->tail;
            else
                n	= n->prev;
            return tmp;
        }

        I&
        operator *() const
        {
            return n->data;
        }

        I*
        operator ->() const
        {
            return &(n->data);
        }

        template<typename O>
        bool
        operator == (const BaseIterator<O>& it) const
        {
            return n == it.n && owner == it.owner;
        }

        template<typename O>
        bool
        operator != (const BaseIterator<O>& it) const
        {
            return n != it.n || owner != it.owner;
        }

        BaseIterator<I>&
        operator = (const BaseIterator<I>& other)
        {
            n	= other.n;
            owner	= other.owner;
            return *this;
        }

        // iterator -> const_iterator
        // const_iterator -//-> iterator
        operator BaseIterator<const I> () const
        {
            return BaseIterator<const I>(n, owner);
        }

    private:
        BaseIterator(Node* n, const List* owner)	: n(n), owner(owner)	{}
        Node*		n;
        const List*	owner;
        friend struct	List;
        friend struct	const_iterator;

        template< typename V, typename H, typename P >
        friend struct	rhash_set;
    };

    typedef BaseIterator<T> Iterator;
    typedef BaseIterator<const T> ConstIterator;


    List() : length(0), head(nullptr), tail(nullptr)		{}

    List(const List<T>& other) : length(0), head(nullptr), tail(nullptr)
    {
        for( List<T>::ConstIterator it = other.cbegin(), end = other.cend();
                it != end;
                ++it )
            push_back(*it);
    }

    List<T>&
    operator = (const List<T>& other)
    {
        clear();
        for( List<T>::ConstIterator it = other.cbegin(), end = other.cend();
                it != end;
                ++it )
            push_back(*it);
        return *this;
    }

    ~List()
    {
        clear();
    }

    void
    clear()
    {
        Node*		n	= head;
        Node*		next	= nullptr;
        while( n )
        {
            next	= n->next;

            if( n && n == head )
                head		= n->next;

            if( n && n == tail )
                tail		= n->prev;

            // prev & next are setup upon node deletion
            delete n;

            --length;

            n	= next;
        }
    }

    void
    push_back(const T& t)
    {
        insert(Iterator(nullptr, this), t);
    }

    void
    push_front(const T& t)
    {
        insert(Iterator(head, this), t);
    }

    void
    pop_back()
    {
        erase(Iterator(tail, this));
    }

    void
    erase(ConstIterator pos)
    {
        Node*		n	= pos.n;
        if( n && n == head )
            head		= n->next;

        if( n && n == tail )
            tail		= n->prev;

        // prev & next are setup upon node deletion
        delete n;
        --length;
    }

    ///
    /// insert an element before the pos iterator
    /// @param pos the position to insert the element before
    /// @param t the element to insert
    ///
    Iterator
    insert(Iterator pos, const T& t)
    {
        ++length;
        // special cases before
        if( pos.n == head )
        {
            Node*	n	= new Node(nullptr, head, t);
            head		= n;
            if( head->next == nullptr )
                tail	= n;

            return Iterator(n, this);
        }
        else if( pos.n == nullptr )	// insert after tail and before end()
        {
            Node* n	= new Node(tail, nullptr, t);
            tail	= n;
            return Iterator(n, this);
        }

        // default case
        Node* n	= new Node(pos.n->prev, pos.n, t);
        return Iterator(n, this);
    }

    Iterator
    begin() const
    {
        return Iterator(head, this);
    }

    Iterator
    last() const
    {
        return Iterator(tail, this);
    }

    Iterator
    end() const
    {
        return		Iterator(nullptr, this);
    }

    ConstIterator
    cbegin() const
    {
        return ConstIterator(head, this);
    }

    ConstIterator
    cend() const
    {
        return		ConstIterator(nullptr, this);
    }

    size_t
    size() const
    {
        return length;
    }

    bool
    empty() const
    {
        return length == 0;
    }

    T&
    front() const
    {
        return (head->data);
    }


private:

    struct Node : BaseAllocation
    {
        Node*		prev;
        Node*		next;
        T		data;

        Node(Node* prev, Node* next, const T& t) : prev(prev), next(next), data(t)
        {
            if( prev )
                prev->next	= this;
            if( next )
                next->prev	= this;
        }

        ~Node()
        {
            if( prev )
                prev->next	= next;

            if( next )
                next->prev	= prev;
        }
    };

    size_t			length;
    Node*			head;
    Node*			tail;

    template<typename I>
    friend struct BaseIterator;
};	// struct list
}	// namespace BmCpp
