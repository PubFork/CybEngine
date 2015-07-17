#pragma once

namespace cyb {

template <class T>
struct LinkedListNode;

// Intrusive, circular, linked list.
template <class T>
class LinkedList {
public:
	LinkedList() {
		m_owner = nullptr;
		m_head = this;
		m_prev = this;
		m_next = this;
	}

	~LinkedList() {
		Clear();
	}

	// Check if the list is empty.
	bool IsEmpty() const { return m_head == this; }

	// Get number of entries in the list.
	size_t Size() const {
		LinkedList *node;
		int numEntries = 0;

		for ( node = m_head->m_next; node != m_head; node = node->m_next ) {
			numEntries++;
		}

		return numEntries;
	}

	// Clears the list if it's the head node, 
	// else just removes the node from the list.
	void Clear() {
		if ( m_head == this ) {
			while ( m_next != this ) {
				m_next->Remove();
			}
		} else {
			Remove();
		}
	}

	// Removes the node from the list.
	void Remove() {
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;

		m_head = this;
		m_prev = this;
		m_next = this;
	}

	// Push a new node to the front.
	void PushFront( LinkedList &node ) {
		node.Remove();

		node.m_prev         = m_head;
		node.m_next         = m_head->m_next;
		m_head->m_next      = &node;
		node.m_next->m_prev = &node;
		node.m_head         = m_head;
	}

	// Push a new node the back.
	void PushBack( LinkedList &node ) {
		node.Remove(); 

		node.m_next         = m_head;
		node.m_prev         = m_head->m_prev;
		m_head->m_prev      = &node;
		node.m_prev->m_next = &node;
		node.m_head         = m_head;
	}

	// Get a pointer to the prev nodes owner, or nullptr if prev points to the head.
	T *Prev() const {
		if ( !m_prev || ( m_prev == m_head ) ) {
			return nullptr;
		}

		return m_prev->m_owner;
	}

	// Get a pointer to the next nodes owner, or nullptr if next points to the head.
	T *Next() const {
		if ( !m_next || ( m_next == m_head ) ) {
			return nullptr;
		}

		return m_next->m_owner;
	}

	// Get a pointer the nodes owner.
	T *Owner() const { return m_owner; }

	// Set owner of the node.
	void SetOwner( T *object ) { m_owner = object; }

	// Get a pointer to the prev node, or nullptr if prev points to the head.
	LinkedList *PrevNode() const {
		if ( m_prev == m_head ) {
			return nullptr;
		}

		return m_prev;
	}

	// Get a pointer to the next node, or nullptr if next points to the head.
	LinkedList *NextNode() const {
		if ( m_next == m_head ) {
			return nullptr;
		}

		return m_next;
	}

	//=========== c++11 range based iteration support:
	template <class T>
	class _iterator {
	public:
		_iterator( LinkedList<T> *node ) : m_node( node ) {}
		~_iterator() = default;
		_iterator &operator*() { return *this; }
		T *operator->() { return m_node->m_owner; }
		T *operator&() { return m_node->m_owner; }
		const T *operator->() const { return m_node->m_owner; }

		bool operator!=( const _iterator &it ) {
			return m_node->m_head != it.m_node->m_head ||
				   m_node->m_prev != it.m_node->m_prev ||
				   m_node->m_next != it.m_node->m_next;
		}

		_iterator &operator++() {
			m_node = m_node->m_next;
			return *this;
		}

	private:
		LinkedList<T> *m_node;
	};

	typedef _iterator<T> iterator;
	typedef const _iterator<T> const_iterator;

	iterator begin() { return iterator( m_head->m_next ); }
	iterator end()   { return iterator( m_head ); }
	const_iterator begin() const { return const_iterator( m_head->m_next ); }
	const_iterator end()   const { return const_iterator( m_head ); }

private:
	LinkedList *m_head;
	LinkedList *m_prev;
	LinkedList *m_next;
	T *m_owner;
};

}	// namespace cyb