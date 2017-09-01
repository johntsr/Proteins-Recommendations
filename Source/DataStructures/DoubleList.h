#ifndef __DOUBLE_LIST__
#define __DOUBLE_LIST__

#include "../General/Math.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

template<class T, class U> class DoubleList;

template<class T>
class Node{
	protected:
		T*	 Data;
		Node<T>* Next;
		Node<T>* Previous;
		bool to_delete;

		Node(bool del){
			Next = Previous = NULL;
			Data = NULL;
			to_delete = del;
		}

		void		setNext(Node<T>* _next)		{ Next = _next; }
		void		setPrevious(Node<T>* _prev)	{ Previous = _prev; }

		~Node(){
			if( Data != NULL && to_delete ){
				delete Data;
			}
		}

		T* rmGet(void){
			T* ret = Data;
			Data = NULL;
			return ret;
		}

	public:

		bool getDel(void){ return to_delete; }
		void setDel(bool del){ to_delete = del; }

		void setData(T* _data){
			if( Data != NULL && to_delete ){
				delete Data;
			}
			Data = _data;
		}

		T* 			data(void)					{ return Data; }
		Node<T>*	next(void)					{ return Next; }
		Node<T>*	previous(void)				{ return Previous; }
		bool		hasNext(void)				{ return Next != NULL; }
		bool		hasPrevious(void)			{ return Previous != NULL; }


		Node<T>* 	getNode( int i ){
			Node<T>* temp = this;
			if( i > 0 ){
				while( i-- > 0 ){
					temp = temp->next();
					if( temp == NULL )
						return NULL;
				}
			}
			else{
				while( i++ < 0 ){
					temp = temp->previous();
					if( temp == NULL )
						return NULL;
				}
			}

			return temp;
		}

		template<class U1, class U2>
		friend class List;
};

template<class T>
class InfoNode{
	private:
		Node<T>* 	Start;
		Node<T>* 	End;
		int 		Count;

	public:
		InfoNode(void){
			Start = End = NULL;
			Count = 0;
		}

		Node<T>*	start(void)					{ return Start;}
		Node<T>*	end(void)					{ return End;}
		int			count(void)					{ return Count;}

		void		setStart(Node<T>* temp)	{ Start = temp;}
		void		setEnd(Node<T>* temp)		{ End = temp;}
		int			incrCount(void)				{ return Count++;}

		int	decrCount(void){
			if (Count > 0)
				return Count--;
			else
				return Count;
		}
};

enum SortType { ASCENDING, DESCENTING, NO_SORT };

class BadRemove{};

template<class T, class U = T >
class List{
	protected:
		InfoNode<T> _List;
		SortType _Sort;

		Node<T>* 	insert(  Node<T>* prevPos, T* Data, bool del){
			Node<T>* node = new Node<T>(del);
			return _insert( node , prevPos, Data );
		}
		bool 		properPosition( T* data, T* givenData ){
			switch (  _Sort ) {
				case ASCENDING:
					return ( *givenData < *data );
				case DESCENTING:
					return ( *givenData > *data );
				default:
					return true;
			}
		}

		bool isEqual( T* data, U givenData ){
				return *data == givenData;
		}

		Node<T>* 	_insert( Node<T>* newPos, Node<T>* prevPos, T* Data){
			newPos->setData(Data);
			if( empty() ){
				_List.setStart(newPos);
				_List.setEnd(newPos);
				newPos->setNext(NULL);
				newPos->setPrevious(NULL);
			}
			else
				if( prevPos == NULL ){				/*insert at Start*/
					newPos->setPrevious(NULL);
					newPos->setNext( _List.start() );
					_List.start()->setPrevious(newPos);
					_List.setStart(newPos);
				}
				else{/*insert at middle*/
					newPos->setNext( prevPos->next() );
					newPos->setPrevious(prevPos);
					prevPos->setNext(newPos);
					if( newPos->next() )
						( newPos->next() )->setPrevious(newPos);
				}
				if ( prevPos == _List.end() ) {
					_List.setEnd(newPos);
				}
			_List.incrCount();
			return newPos;
		}

		Node<T>* 	findPosition( T* Data ){
			int i = 0;
			Node<T> *temp = NULL, *prevTemp = NULL;
			for( i = 0, temp = _List.start(); i < _List.count() ; i++ ){
				if( properPosition( Data, temp->data() ) ){
					if( i > 0 )
						return prevTemp;
					else
						return NULL;
				}
				prevTemp = temp;
				temp = temp->next();
			}
			return _List.end();
		}

		Node<T>* 	_remove( Node<T>* prevPos ){
			Node<T> *temp = NULL;
			if( !empty() ){
				if( prevPos == NULL ){							/*delete at Start*/
					temp = _List.start();
					if( temp->next() ){ /*if not one and only Element*/
						temp->next()->setPrevious(NULL);
					}
					_List.setStart( temp->next() );
					if( _List.count() == 1)
						_List.setEnd(NULL);
				}
				else{												/*delete at middle*/
					temp = prevPos->next();
					prevPos->setNext( temp->next() );

					if( prevPos == _List.end()->previous() ){
						_List.setEnd(prevPos);
					}

					if( temp->next() )
						temp->next()->setPrevious(prevPos);

				}
				_List.decrCount();
			}
			return temp;
		}



	public:
		List(SortType sort = NO_SORT){ _Sort = sort; }

		~List(){ flush(); }

		int 	empty( void ){
			return ( _List.start() == NULL );
		}

		Node<T>* 	insertBefore(Node<T>* Pos, T* Data, bool del ){
			if( Pos == NULL ){
				return insertAtStart( Data, del);
			}
			return insert( Pos->previous() , Data, del);
		}

		Node<T>* 	insertAfter(Node<T>* Pos, T* Data, bool del ){
			return insert( Pos , Data, del);
		}

		Node<T>* 	insertAtStart(  T* Data, bool del ){
			return insert( NULL, Data, del);
		}

		Node<T>* 	insertAtEnd(  T* Data, bool del ){
			return insert( _List.end(), Data, del);
		}

		Node<T>* 	insertSorted(  T* Data, bool del ){
			return insert( findPosition( Data ), Data, del);
		}

		Node<T>* 	insertRandom(  T* Data, bool del ){
			Node<T>* randPrev = start();
			int times = (int)Math::dRand(0.0, count());
			for(; times > 0; times--){
				randPrev = randPrev->next();
			}
			return insert(randPrev, Data, del);
		}

		T* removeFirst(U Data, Node<T>* _start = NULL, Node<T>* _end = NULL){
			Node<T>* temp = findFirst(Data, _start, _end);
			return remove(temp);
		}

		T* removeLast(U Data, Node<T>* _start = NULL, Node<T>* _end = NULL){
			return remove( findLast(Data, _end, _start) );
		}

		void removeAll(U Data){
			if( empty() ){
				return;
			}
			Node<T>* temp = start();
			while( (temp = findFirst(Data, temp->previous() ) ) != NULL ){
				temp = _remove( temp->previous() );
				delete temp;
			}
		}

		T* 	removeAtStart( void ){
			Node<T>* temp = _List.start();
			return remove( temp );
		}

		T* 	removeAtEnd( void ){
			Node<T>* temp = _List.end();
			return remove( temp );
		}

		T* remove( Node<T>*& Pos){
			if( Pos == NULL ){
				throw BadRemove();
			}

			Node<T>* temp = _remove( Pos->previous() );
			if( temp == NULL ){
				throw BadRemove();
			}
			Pos = temp->next();
			T* retData = temp->rmGet();
			delete temp;
			return retData;
		}

		void 	flush( void ){
			Node<T> *temp1, *temp2;
			temp1 = _List.start();
			while( _List.decrCount() ){
				temp2 = temp1;
				temp1 = temp1->next();
				delete temp2;
			}
			_List.setStart(NULL);
			_List.setEnd(NULL);
		}

		Node<T>* 	end( void ){
			return _List.end();
		}

		Node<T>* 	start( void ){
			return _List.start();
		}

		int count(void){ return _List.count(); }

		bool contains(U givenData){
			return findFirst(givenData) != NULL;
		}

		Node<T>* 	findFirst(U givenData, Node<T>* _start = NULL, Node<T>* _end = NULL ){
			Node<T>* temp = _start;
			if( temp == NULL ){
				temp = start();
			}
			else{
				temp = temp->next();
			}

			while( temp != NULL && temp != _end ){
				if( isEqual( temp->data(), givenData ) ){
					return temp;
				}
				temp = temp->next();
			}
			return NULL;
		}

		Node<T>* 	findLast(U givenData, Node<T>* _start = NULL, Node<T>* _end = NULL ){
			Node<T>* temp = _end;
			if( temp == NULL ){
				temp = end();
			}
			else{
				temp = temp->previous();
			}

			while( temp != NULL && temp != _start ){
				if( isEqual( temp->data(), givenData ) ){
					return temp;
				}
				temp = temp->previous();
			}
			return NULL;
		}

		void findAll( List<T, U>& findList, U givenData){
			Node<T>* temp = _List.start();
			while( temp != NULL ){
				if( isEqual( temp->data(), givenData ) ){
					findList.insertAtEnd( temp->data(), false);
				}
				temp = temp->next();
			}
		}


};

#endif
