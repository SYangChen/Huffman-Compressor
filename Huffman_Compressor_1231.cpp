/*  資工110 
 *  B063040061 
 *  陳少洋
 *  Dec. 31, 2018
 *	撰寫Huffman壓縮軟體，並同時具備壓縮、解壓縮兩種功能。
 *	根據使用者由命令列輸入之資料，給予Huffman編碼壓縮，
 *	將壓縮表、壓縮前後資料大小、壓縮率輸出於標準輸出。 
 */ 


#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <bitset>

using namespace std ;

typedef struct element {
	char ch ;		// store 8bit char
	int times ;		// appear times
}element;

typedef struct node {
	element el ;	// element above
	bool leaf ;		// internal or leaf node
	node *left ;	// binary tree left
	node *right ;	// binary tree right
}node;

typedef vector<bool> huffCode ;
huffCode huffList[257] ;

fstream fin ;				// file input
fstream fout ;				// file output
FILE *pf ;					// c flie 
element code[257] ;			// compute appear times
vector<char> originData ;	// store oringin data
vector<node*> mHeap ;		// min heap
node *rootH ;				// huffman tree 
int size ;					// min heap size
int unCFileSize ;			// uncompress file size
int CFileSize ;				// compress file size
char record ;
// static unsigned int outbit = 0x01;

// ---------------using function------------------

void Init() ;
bool LoadFileBinary( string ) ;
bool LoadFileBinary( char*, char* ) ;
void Read() ;
void ReadBin() ;
void WriteOutFile( string ) ;
void LoadDecompressFile( char* ) ;
void Compute() ;
void BuildMinHeap() ;
void PushIntoMinHeap( node* ) ;
void RemoveFromMinHeap() ;
node *GetMin() ;
void TrickleDownMin( int ) ;
void BuildHCTree() ;
void GenerateHuffCode( node*, huffCode, huffCode* ) ; 
void PrintTree() ;
void Inorder( node* ) ;
void Compress() ;
void PrintFileHeader() ;
void PrintTable() ;
void BuildTreeTable( char, char* ) ;
void Decompress() ;

// ---------------using function------------------

void Init() {
	// initiate function
	for ( int i = 0 ; i < 256 ; i++ )
		code[i].times = 0 ;
	size = 0 ;
	unCFileSize = 0 ;
	CFileSize = 0 ;
	rootH = NULL ;
}

bool LoadFileBinary( string read ) {   					// read.c_str()       
	fin.open( read.c_str(), ios::in | ios::binary );	// file open / read 
	if ( !fin )
		return false ;
	else {
		Read() ;
		// "getline" or "get" here (input)
		fin.close() ;
		return true ;
	} // end else 
} 

bool LoadFileBinary( char *read, char *out ) {   	// read.c_str()       
	// fin.open( read, ios::in | ios::binary );		// file open / read 
	if ( ( pf = fopen(  read,"rb" ) ) == NULL )		// file open / read
		return false ;
  	ReadBin() ;
  	LoadDecompressFile( out ) ;
	fclose(pf) ;
	return true ;
} 

void Read() {
	char i ;
	fin.read( &i, 1 ) ;
	// fin.get(i) ;
	while ( !fin.eof() ) {
		// cout << i << endl ;
		unCFileSize++ ;
		code[i+128].ch = i ;		// char -127~128
		code[i+128].times++ ;		// unsigned char 0~255
		originData.push_back(i) ;
		/*
		node *temp = new node ;
		temp->el = code[i] ;
		temp->leaf = true ;
		temp->left = NULL ;
		temp->right = NULL ;
		PushIntoMinHeap( temp ) ;
		*/
		// cout << (int)i << "->" << i << endl ;
		fin.read( &i, 1 ) ;
		// fin.get(i) ;
	}
	// cout << "end" << endl ;
}

void ReadBin() {
	char line[100] ;
	char trash ;
	char uch ;
	for ( int i = 0 ; i < 3 ; i++ ) {
		fscanf( pf, "%[^\n]", line ) ;		// trash line
		trash = fgetc( pf ) ;				// trash end line
	}
	fread( &uch, 1, 1, pf ) ;
	fscanf( pf, "%[^\n]", line ) ;			// trash line
	trash = fgetc( pf ) ;					// trash end line
	while ( strcmp( line, "nd of file header" ) != 0 ) {
		// cout << uch << "here" << line << endl ;
		BuildTreeTable( uch, line+1 ) ;
		fread( &uch, 1, 1, pf ) ;
		fscanf( pf, "%[^\n]", line ) ;
		trash = fgetc( pf ) ;
	} // end while
	record = fgetc( pf ) ;
	// cout << record ;
}

void WriteOutFile( string write ) {
		
	// string write ;
	// write = "output" ;
	// write.append( num ) ;
	// write.append( "txt" ) ;
	// fout.open( write.c_str(), ios::out | ios::binary ) ;	// file open / out & append ( ios::out|ios::app )
	// fout << "Hello!! success" << endl;               	// output data in file
	pf = fopen(  write.c_str(),"wb" );					 	// create file in binary mode
	Compute() ;
	PrintFileHeader() ;
	Compress() ;
	fclose(pf) ;
	// fout.close() ;
	// cout << "\nFile build : " << fout << endl ;
	
} // WriteOut()

void LoadDecompressFile( char *write ) {
	fout.open( write, ios::out | ios::binary ) ;
	Decompress() ;
	fout.close() ;
	cout << "Decompress file build." << endl ;
}

void Compute() {
	for ( int i = 0 ; i < 256 ; i++ ) {
		if ( code[i].times != 0 )
			CFileSize += code[i].times*huffList[i].size() ;
	}	// compute file size after compress
	record = CFileSize%8+48 ;
}

void BuildMinHeap() {
	for( int i = 0 ; i < 256 ; i++ ) {
		if ( code[i].times != 0 ) {
			// cout << "push" << code[i].ch << "----" << code[i].times << endl ; 
			node *temp = new node ;
			temp->el = code[i] ;
			temp->leaf = true ;
			temp->left = NULL ;
			temp->right = NULL ;
			PushIntoMinHeap( temp ) ;	
		}
	}
	// Build up min heap by vector storing pointer
}

void PushIntoMinHeap( node *newNode ) {
	mHeap.push_back( newNode ) ;
	node *temp ;
	size++ ;
	int cur = size-1 ;                                        // current node
	int parent = ( cur-1 )/2 ;								  // current node's parent
	while (( parent >= 0) && (( mHeap[cur]->el.times < mHeap[parent]->el.times ) || 
			( mHeap[cur]->el.times == mHeap[parent]->el.times && mHeap[cur]->el.ch < mHeap[parent]->el.ch )))  {
		temp = mHeap[parent] ;
		mHeap[parent] = mHeap[cur] ;                  // swap
		mHeap[cur] = temp ;
		cur = parent ;
		parent = ( cur-1 )/2 ;
	}	// min heap rule
}

void TrickleDownMin(int nodeIndex) {
	int leftChildIndex, rightChildIndex, minIndex ;
	node *tmp;
	leftChildIndex = 2*nodeIndex+1 ; // getLeftChildIndex(nodeIndex);
	rightChildIndex = 2*nodeIndex+2 ; // getRightChildIndex(nodeIndex);
	if ( rightChildIndex >= size ) {
	    if ( leftChildIndex >= size )
	        return ;
	    else
	        minIndex = leftChildIndex ;
	} 
	else {
	    if ( mHeap[leftChildIndex]->el.times < mHeap[rightChildIndex]->el.times || 
			( mHeap[leftChildIndex]->el.times == mHeap[rightChildIndex]->el.times && 
			  mHeap[leftChildIndex]->el.ch < mHeap[rightChildIndex]->el.ch ))
	        minIndex = leftChildIndex ;
	    else
	        minIndex = rightChildIndex ;
	}
	if ( mHeap[nodeIndex]->el.times > mHeap[minIndex]->el.times || 
		( mHeap[nodeIndex]->el.times == mHeap[minIndex]->el.times && 
		  mHeap[nodeIndex]->el.ch > mHeap[minIndex]->el.ch )) {
	    tmp = mHeap[minIndex] ;
	    mHeap[minIndex] = mHeap[nodeIndex] ;
	    mHeap[nodeIndex] = tmp ;
	    TrickleDownMin( minIndex ) ;
	}	// heapify
}

void RemoveFromMinHeap() {
	if ( size == 0 )
		cout << "heap error" << endl ;
	else {
		mHeap[0] = mHeap[size-1] ;
		mHeap.erase( mHeap.begin()+size-1 ) ;
		size-- ;
		if ( size > 0 )
			TrickleDownMin(0) ;
	}	// remove the min element then reheap
}

node *GetMin() {
	node *temp = mHeap[0] ;
	// cout << "gogogo " << temp->el.ch << "/////" << temp->el.times << endl ;
	RemoveFromMinHeap() ;
	return temp ;
}

void BuildHCTree() {	// build huffman compress tree table
	node *tempRoot = NULL ;
	while ( size > 1 ) {
		node *l, *r ;	// always pick up the two lowest element
		l = GetMin() ;
		r = GetMin() ;
		tempRoot = new node ;
		tempRoot->el.times = l->el.times+r->el.times ;
		tempRoot->leaf = false ;
		if ( l->el.ch < r->el.ch ) {
			tempRoot->left = l ;
			tempRoot->el.ch = l->el.ch ;
			tempRoot->right = r ;
		}
		else {
			tempRoot->left = r ; 
			tempRoot->el.ch = r->el.ch ;
			tempRoot->right = l ;
		} 
		PushIntoMinHeap( tempRoot ) ;
	}
	rootH = tempRoot ;
} 

void GenerateHuffCode( node *walk, huffCode prefix ) {
	if ( walk->leaf ) {					// leaf node
		huffList[walk->el.ch+128] = prefix ;
		// cout << walk->el.ch << "----" << endl ;
	}
	else {			// internal node
		huffCode leftPrefix = prefix ;
		leftPrefix.push_back( false ) ;
		GenerateHuffCode( walk->left, leftPrefix ) ;

		huffCode rightPrefix = prefix ;
		rightPrefix.push_back( true ) ;
		GenerateHuffCode( walk->right, rightPrefix ) ;
	}
}
/*
void write_bit(bool bit)
{
    outbit <<= 1;                    // shift byte to make room
    if (bit) outbit |= 0x01;         // set lowest bit id desired

    if (outbit & 0x100) {            // was the sentinel bit shifted out?
    	outbit = outbit&0xff ;
        fwrite (&outbit, 1, 1, pf) ;// write_byte(outbit & 0xff);   // final output of 8-bit chunk
        outbit = 0x01;               // reset to sentinel vylue
    }
}

void flush_bit()
{
    while (outbit != 0x01) write_bit(false); 
}
*/
void Compress() {
	int end, codelength, eightBit = 0 ;
	char data ;
	unsigned char out = 0 ;
	bool temp ;
	// bitset<1> b ;
	end = originData.size() ;
	for ( int i = 0 ; i < end ; i++ ) { // find the direct vector index
		data = originData[i] ;
		codelength = huffList[data+128].size() ;
		for ( int j = 0 ; j < codelength ; j++ ) {
			temp = huffList[data+128][j] ;
			// write_bit(temp) ;
			if ( temp )
				out = (out<<1)|0x01 ;
			else
				out = out << 1 ;
			eightBit++ ;
			if ( eightBit == 8 ) {
				fwrite (&out, 1, 1, pf);
				eightBit = 0 ;
				out = 0 ;
			}	// after collect 8 bit to output
		}
		// flush_bit() ;
	}
	if ( eightBit != 0 ) {
			while ( eightBit < 8 ) {
				// cout << eightBit << endl ;
				out = out << 1 ;
				eightBit = eightBit+1 ;
			}
			fwrite (&out, 1, 1, pf) ;
	}	// less then 8 bit 
}

void PrintFileHeader() {
	double temp = (double)CFileSize/8 ;
	fprintf( pf, "Oringinal File Size : %d byte\n", unCFileSize ) ;
	fprintf( pf, "Compress File Size : %lf byte\n", temp ) ;
	fprintf( pf, "Compression Ratio : %lf\n", unCFileSize/temp ) ;
	for ( int i = 0 ; i < 256 ; i++ ) {
		if ( code[i].times != 0 ) {
			fprintf( pf, "%c=", (char)(i-128) ) ;// cout << (char)i << "=" ;
			for ( vector<bool>::iterator it = huffList[i].begin() ; it != huffList[i].end() ; it++ )
			 	fprintf( pf, "%d", (*it?1:0) ) ;
			fprintf( pf, "\n" ) ;
		}
	}
	fprintf( pf, "end of file header\n" ) ;
	fprintf( pf, "%c", record ) ;
}

void PrintTable() {
	cout << "Oringinal File Size : " << unCFileSize << " byte" << endl ;
	cout << "Compress File Size : " << (double)CFileSize/8 << "byte" << endl ;
	cout << "Compression Ratio : " << (double)unCFileSize/((double)CFileSize/8) << endl << endl ;
	cout << "Compress Table : " << endl ;
	for ( int i = 0 ; i < 256 ; i++ ) {
		if ( code[i].times != 0 ) {
			if ( i < 160 || i == 255 )
				cout << "[" << i << "]=" ;
			else 
				cout << (char)(i-128) << "=" ;
			for ( vector<bool>::iterator it = huffList[i].begin() ; it != huffList[i].end() ; it++ )
			 	cout << (*it?1:0) ;
			cout << endl ;
		}
	}
}

void BuildTreeTable( char ch, char *bitStream ) {
	int i ;
	if ( rootH == NULL ) {
		rootH = new node ;
		rootH->leaf = false ;
		rootH->left = NULL ;
		rootH->right = NULL ;
	}
	node *walk = rootH ;
	for ( i = 0 ; i < strlen(bitStream)-1 ; i++ ) {
		if ( bitStream[i] == '0' ) {
			if ( walk->left == NULL ) {
				walk->left = new node ;
				walk->left->leaf = false ;
				walk->left->left = NULL ;
				walk->left->right = NULL ;
			}
			walk = walk->left ;
		} // end if
		else {
			if ( walk->right == NULL ) {
				walk->right = new node ;
				walk->right->leaf = false ;
				walk->right->left = NULL ;
				walk->right->right = NULL ;
			}
			walk = walk->right ;
		} // end else
	} // end for
	if ( bitStream[i] == '0' ) {
		walk->left = new node ;
		walk = walk->left ;
	}
	else {
		walk->right = new node ;
		walk = walk->right ;
	}
	// cout << ch << "ssssssssss" << endl ;
	walk->el.ch = ch ;
	walk->leaf = true ;
	walk->left = NULL ;
	walk->right = NULL ;
}

void Decompress() {	// find till the leaf bit by bit
	unsigned char uch, uuch ;
	bool temp ;
	node *walk = rootH ;
	fread( &uch, 1, 1, pf ) ;
	while ( fread( &uuch, 1, 1, pf ) != NULL ) {
		for ( unsigned char i = 128 ; i >= 1 ; i/=2 ) {
			// deal with bit by bit ( input 8 bit at once )
			temp = i&uch ;	// bit mask
			// cout << temp ;
			if ( !temp ) {
				if ( walk->left->leaf ) {
					fout << walk->left->el.ch ;
					walk = rootH ;
				}
				else 
					walk = walk->left ;
			}
			else {
				if ( walk->right->leaf ) {
					fout << walk->right->el.ch ;
					walk = rootH ;
				}
				else
					walk = walk->right ;
			}
		}
		uch = uuch ;
	}
	// cout << record << endl ;
	for ( unsigned char i = 128, j = 0 ; j < (record-'0') ; i/=2, j++ ) {
		// deal with bit by bit ( input 8 bit at once )
		temp = i&uuch ;	// bit mask
		// cout << temp ;
		if ( !temp ) {
			if ( walk->left->leaf ) {
				fout << walk->left->el.ch ;
				walk = rootH ;
			}
			else 
				walk = walk->left ;
		}
		else {
			if ( walk->right->leaf ) {
				fout << walk->right->el.ch ;
				walk = rootH ;
			}
			else
				walk = walk->right ;
		}
	}
}
/*
void PrintTree() {
	node *walk = rootH ;
	Inorder( walk ) ;
}

void Inorder( node *walk ) {
	if ( walk == NULL )
		return ;
	if ( walk->leaf == true )
		cout << "here" << walk->el.ch << " " << walk->el.times << endl ;
	Inorder( walk->left ) ;
	Inorder( walk->right ) ;
}
*/
int main( int argc, char **argv ) {
	// string read = "in.txt" ;
	if ( argc != 6 ) return 0 ;
	// cout << argc << argv[1] << endl ; 
	if ( strcmp( argv[1], "-c" ) == 0 ) {
		string str = argv[3] ;
		if ( !LoadFileBinary( str ) ) {
			cout << "error compress" << endl ;
			return 0 ;
		}
		BuildMinHeap() ;
		BuildHCTree() ;
		GenerateHuffCode( rootH, huffCode() ) ;
		str = argv[5] ;
		WriteOutFile( str ) ;
		// Compress() ;
		PrintTable() ;
		// PrintTree() ; 
		// cout << huffList['C'][0] << huffList['C'][1] << huffList['C'][2] << endl ;
	}
	else if ( strcmp( argv[1], "-u" ) == 0 ) {
		if ( !LoadFileBinary( argv[3], argv[5] ) ) {
			cout << "error decompress" << endl ;
			return 0 ;
		}
		// PrintTree() ;
	}
	else
		cout << "error command" << endl ;
	system("pause") ;
}

