#ifndef _CIR_QUEUE_H_INCLUDED
#define _CIR_QUEUE_H_INCLUDED


#include <iostream>

using namespace std;


template<typename T>
class CirQueue
{
public:
	CirQueue ( size_t _capacity_ = 1 ) : _capacity ( _capacity_ ),
		_length ( 0 ),
		pBase ( new T[_capacity_] ),
		pHead ( pBase ),
		pLast ( pBase )
	{}


	~CirQueue ()
	{
		delete[]pBase;
	}


	void ClearQueue ()
	{
		pHead = pBase;
		pLast = pBase;
		_length = 0;
	}


	bool IsEmpty ()const { return !_length; }
	size_t size ()const { return  _length; }
	size_t capacity ()const { return _capacity; }
	T& GetHead ()const { return *pHead; }

	void Push ( T &e );
	T Pop ();
	void vist ();
	void expandMem ( size_t size );


private:
	void Insert ( T &e );

	size_t _capacity;
	size_t _length;
	T *pBase;
	T *pHead;
	T *pLast;

};


template<typename T>
void CirQueue<T>::expandMem ( size_t size )
{
	T *ptmpNew ( new T[_capacity + size] );
	T *ptmpNewCopy ( ptmpNew );
	T *ptmpheadCopy ( pHead );
	T *ptmplimit ( pBase + _capacity - 1 );


	for ( int i = 0; i < _capacity; ++i )
	{
		*ptmpNewCopy = *ptmpheadCopy;
		if ( ptmpheadCopy == ptmplimit )
		{
			ptmpheadCopy = pBase;
			continue;
		}
		++ptmpNewCopy;
		++ptmpheadCopy;
	}
	delete[]pBase;
	pBase = ptmpNew;
	pHead = pBase;
	pLast = pBase + _capacity;
	_capacity += size;
}


template<typename T>
void CirQueue<T>::Insert ( T &e )
{
	if ( pLast != ( pBase + _capacity ) )
	{
		//*pLast = e;
		memcpy ( pLast, &e, sizeof(e));
		++pLast;
	}
	else
	{
		*pBase = e;
		pLast = pBase + 1;
	}
}


template<typename T>
void CirQueue<T>::Push ( T &e )
{
	if ( _length < _capacity )
		++_length;

	Insert ( e );
}


template<typename T>
T CirQueue<T>::Pop ()
{
	if ( _length > 0 )
	{
		--_length;
		if ( pHead == pBase + _capacity - 1 )
		{
			T tmp ( *pHead );
			pHead = pBase;
			return tmp;
		}
		return *( pHead++ );
	}
	return *pBase;
}


template<typename T>
void CirQueue<T>::vist ()
{
	T *ptmphead ( pHead );
	T * const ptmplimit ( pBase + _capacity );
	for ( int i = 0; i<_length; ++i )
	{
		if ( ptmphead == ptmplimit )
			ptmphead = pBase;


		std::cout << " '" << *ptmphead << "' ";
		++ptmphead;
	}
	cout << endl;
}



#endif // _CIR_QUEUE_H_INCLUDED