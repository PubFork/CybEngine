#pragma once

namespace cyb {

template <class T>
class IntrusiveList {
public:
	IntrusiveList() {
		m_owner = nullptr;
		m_head = this;
		m_prev = this;
		m_next = this;
	}

	~IntrusiveList() {
		Clear();
	}

	// Check if the list is empty.
	bool IsEmpty() const { return m_head == this; }

	// Get number of entries in the list
	size_t Size() const {
		IntrusiveList<T> *node;
		int numEntries = 0;

		for ( node = m_head->m_next; node != m_head; node = node->m_next ) {
			numEntries++;
		}

		return numEntries;
	}

	// Clears the list if it's the head node, else just removes
	// the node from the.
	void Clear() {
		if ( m_head == this ) {
			while ( m_next != this ) {
				m_next->Remove();
			}
		}
		else {
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

	void InsertBefore( IntrusiveList &node ) {
		Remove();

		m_next         = &node;
		m_prev         = node.m_prev;
		node.m_prev    = this;
		m_prev->m_next = this;
		m_head         = node.m_head;
	}

	void InsertAfter( IntrusiveList &node ) {
		Remove();

		m_prev         = &node;
		m_next         = node.m_next;
		node.m_next    = this;
		m_next->m_prev = this;
		m_head         = node.m_head;
	}

	void AddToEnd( IntrusiveList &node ) {
		InsertBefore( *node.m_head );
	}

	void AddToFront( IntrusiveList &node ) {
		InsertAfter( *node.m_head );
	}

	T *Prev() const {
		if ( !m_prev || ( m_prev == m_head ) ) {
			return nullptr;
		}

		return m_prev->m_owner;
	}

	T *Next() const {
		if ( !m_next || ( m_next == m_head ) ) {
			return nullptr;
		}

		return m_next->m_owner;
	}

	T *Owner() const { return m_owner; }

	void SetOwner( T *object ) {
		m_owner = object;
	}

	IntrusiveList<T> *PrevNode() const {
		if ( m_prev == m_head ) {
			return nullptr;
		}

		return m_prev;
	}

	IntrusiveList<T> *NextNode() const {
		if ( m_next == m_head ) {
			return nullptr;
		}

		return m_next;
	}

private:
	IntrusiveList *m_head;
	IntrusiveList *m_prev;
	IntrusiveList *m_next;
	T *m_owner;
};

}	// namespace cyb