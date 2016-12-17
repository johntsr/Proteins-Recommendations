#include "DoubleList.h"

// public bool T::operator ==(U);
// public T::operator U(void);
// static public int T::hash(U);

template<class T, class U>
class Bucket{
	private:
		int Size;					// the size doesn' t change
		T**  Entries;				// a table of entries
		bool* to_delete;

		template<class U1, class U2>
		friend class BucketStructure;

		T* remove(bool* del){
			// get the first you see
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL){
					T* temp = Entries[i];
					Entries[i] = NULL;
					*del = to_delete[i];
					return temp;
				}
			}
			return NULL;
		}

		Bucket(int _size){
			Size = _size;
			Entries = new T*[Size];
			to_delete = new bool[Size];
			for( int i = 0 ; i < Size; i++){
				Entries[i] = NULL;
			}
		}

		T* get(U myKey){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL && *( Entries[i] ) == myKey ){
					return Entries[i];
				}
			}
			return NULL;
		}

		bool full(void){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] == NULL){
					return false;
				}
			}
			return true;
		}

		bool empty(void){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL){
					return false;
				}
			}
			return true;
		}

		void add(T* myValue, bool del){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] == NULL){
					Entries[i] = myValue;
					to_delete[i] = del;
					return;
				}
			}
		}

		T* remove(U myKey){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL && *( Entries[i] ) == myKey){
					T* temp = Entries[i];
					Entries[i] = NULL;
					return temp;
				}
			}
			return NULL;
		}

		int size(void){ return Size; }

		bool contains(U myKey){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL && *( Entries[i] ) == myKey ){
					return true;
				}
			}
			return false;
		}

		~Bucket(void){
			for( int i = 0 ; i < Size; i++){
				if( Entries[i] != NULL && to_delete[i] )
		            delete Entries[i];
			}
			delete[] Entries;
			delete[] to_delete;
		}
};

template<class T, class U>
class BucketStructure{
	private:
		Bucket<T,U>	bucketEntries;
		List<T,U>*	OverflowEntries;
		List<T,U>* 	splitBuffer;
		int Size;

		template<class U1, class U2>
		friend class HashTable;

		bool hasMoreToSplit(void){
			return !splitBuffer->empty();
		}

		void readyForSplit(void){
			// I empty the content of the normal and the overflow buckets
			// this content now goes to splitBuffer
			// in Remove(), splitBuffer gradually becomes empty and its content is re-distributed in the Hash Table
			List<T,U>* tempOF = splitBuffer;
			splitBuffer = OverflowEntries;
			OverflowEntries = tempOF;

			T* temp = NULL;
			bool del = false;
			while( !bucketEntries.empty() ){
				temp = bucketEntries.remove( &del );
				splitBuffer->insertAtStart( temp, del );
			}
		}

		T* remove(bool* del){
			// Remove is called repeatedly for splitting the bucket
			// remove an entry in order to re-hash it
			*del = splitBuffer->start()->getDel();
			return splitBuffer->removeAtStart();
		}

		BucketStructure(int _size): bucketEntries(_size) {
			Size = _size;
			OverflowEntries = new List<T,U>();
			splitBuffer = new List<T,U>();
		}

		void add(T* value, bool del){
			if ( ! bucketEntries.full() ){
				//printf("Got in the table!\n" );
				bucketEntries.add( value , del);
			}
			else{
				OverflowEntries->insertAtStart( value , del);
			}
		}

		T* get(U _data){
			T* temp = bucketEntries.get(_data);
			if(temp != NULL){
				return temp;
			}
			else{
				Node<T>* node = OverflowEntries->findFirst(_data);
				if( node == NULL ){
					return NULL;
				}
				else{
					return node->data();
				}
			}
		}

		T* remove(U _data){
			T* temp = bucketEntries.remove(_data);
			if(temp != NULL){
				return temp;
			}
			else{
				return OverflowEntries->removeFirst(_data);
			}
		}

		bool contains(U _data){
			return bucketEntries.contains( _data ) || OverflowEntries->contains(_data);
		}

		~BucketStructure(void){
			delete OverflowEntries;
			delete splitBuffer;
		}

};

template<class T, class U>
class HashTable{
	private:
		BucketStructure<T,U>** BucketTable;			// a table of the structure in which I store the buckets
		int _i;										// the current round
		int p;										// the position of the bucket I will split Next
		int customersCount;							// how many customers I have inserted
		int bucketsCount;							// how many bucketStructures I have
		bool justChangedRound;
		int m;										// the initial number of buckets
		double SplitPoint;
		int Capacity;

		int power(int a , int n ){
			if( n == 0){
				return 1;
			}
			else if( n % 2 == 0){
				int t = power(a, n/2);
				return t*t;
			}
			else{
				int t = power(a, n/2);
				return a*t*t;
			}
		}

		void tryToInsert(T* value, bool del){
			int position = hash( T::hash( (U)(*value) ) , _i );		// get the hash value of h(i)
			if( position < p ){										// indicates I have to use the hash function of the Next round
				position = hash( T::hash( (U)(*value) ) , _i + 1 );
			}
			BucketTable[position]->add( value, del );
			customersCount++;
		}

		double loadingFactor(void){
			return customersCount / ( power(2,_i + 1) * m ) ;
		}

		void expandHashTable(void){
			if ( justChangedRound ){
				// make a new table, twice bigger
				BucketTable = (BucketStructure<T,U>**) realloc( BucketTable, 2 * bucketsCount * sizeof(BucketStructure<T,U>*) );
				for( int i = bucketsCount ; i < 2 * bucketsCount ; i++){
					BucketTable[i] = NULL;
				}
			}
			BucketTable[ bucketsCount ] = new BucketStructure<T,U>(Capacity);
			bucketsCount++;
		}

		void splitBucket(void){
			int position = -1;
			bool del = false;
			BucketTable[p]->readyForSplit();
			while( BucketTable[p]->hasMoreToSplit() ){
				T* tempEntry = BucketTable[p]->remove(&del);
				position = hash( T:: hash( (U)*(tempEntry) ), _i + 1 );
				BucketTable[position]->add(tempEntry, del);
			}
			p++;
		}

		void resetAfterExpand(void){
			justChangedRound = false;
			if( bucketsCount == power(2, _i + 1) * m ){										// the buckets have doubled, go to Next round
				_i++;
				p = 0;
				justChangedRound = true;
			}
		}

		int hash( int k , int round){
			return k % ( m * power(2,round) );
		}

	public:
		HashTable(int _m = 17, double _point = 0.75, int capacity = 5){
			justChangedRound = true;
			_i = 0;
			p = 0;
			customersCount = 0;
			m = _m;
			bucketsCount = m;
			Capacity = capacity;
			BucketTable = (BucketStructure<T,U>**)malloc(m * sizeof(BucketStructure<T,U>*) );

			for(int i = 0 ; i < m ; i++){
				BucketTable[i] = new BucketStructure<T,U>(capacity);
			}
		}

		void insert(T* value, bool del){
			tryToInsert(value, del);
			while( loadingFactor() > SplitPoint ){	// maybe one split is not enough, so "while" instead of "if"
				expandHashTable();
				splitBucket();
				resetAfterExpand();
			}
		}

		bool contains( U _data ){
			int	proposingPosition = hash( T::hash(_data) , _i );
			bool foundInFirstPosition = BucketTable[proposingPosition]->contains(_data);
			if( foundInFirstPosition )
				return true;
			else{
				// now, check the position of the Next round hash function
				proposingPosition = hash( T::hash(_data) , _i + 1);
				if( proposingPosition > bucketsCount - 1 )						// don't have that many buckets yet!
					return false;
				else
					return BucketTable[proposingPosition]->contains(_data);
			}
		}

		T* operator [] (U key){
			int position = hash( T::hash(key) , _i );
			T* temp = BucketTable[position]->get(key);
			if( temp != NULL ){
				return temp;
			}
			else{
				position = hash( T::hash(key) , _i + 1 );
				if( position > bucketsCount - 1 ){ // don't have that many buckets yet!
					return NULL;
				}
				else{
					return BucketTable[position]->get(key);
				}
			}
		}

		T* remove (U key){
			int position = hash( T::hash(key) , _i );
			T* temp = BucketTable[position]->remove(key);
			if( temp != NULL ){
				customersCount--;
				return temp;
			}
			else{
				position = hash( T::hash(key) , _i + 1 );
				if( position > bucketsCount - 1 ){ // don't have that many buckets yet!
					return NULL;
				}
				else{
					temp = BucketTable[position]->remove(key);
					if(temp != NULL){
						customersCount--;
					}
					return temp;
				}
			}
		}

		~HashTable(void){
			for(int i = 0 ; i < bucketsCount ; i++)
				delete BucketTable[i];
			free(BucketTable);
		}
};
