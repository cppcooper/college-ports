#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES

#include <ostream>


typedef unsigned __int8 ubyte;


///This is the return type of an operator[] overload inside another class and that is the only use of this class
//This class is basically a bit accessor for a rudimentary bitset<8>
class u8bits_access
{
private:
	ubyte* B;					///Points to Byte's address
	ubyte b;					///Specifies the bit position
	ubyte bitmask;				///Bitmask based on bit position

public:
	u8bits_access( int Bit, ubyte* Byte )
	{
		B = Byte;
		b = Bit;
		bitmask = 1 << b;
	}

	//Compare the state of a bit
	bool u8bits_access::operator==( bool a )
	{
		if ( a )
			return ( ( *B & bitmask ) != 0 );
		else
			return ( ( *B & bitmask ) == 0 );
	}

	//Set the state of a bit
	u8bits_access& u8bits_access::operator=( bool a )
	{
		if ( a )
			*B |= bitmask; ///Sets the bit in bitmask to true
		else
			*B &= ~bitmask; ///Sets the bit in bitmask to false
		return *this;
	}

	//Observe the state of a bit
	bool u8bits_access::operator()()
	{
		return ( ( *B & bitmask ) != 0 );
	}
};


//This struct is basically a rudimentary bitset<8>
struct u8bits
{
	ubyte B;

	friend std::ostream& operator<<( std::ostream& out, const u8bits& obj )
	{
		///Quick and dirty object streaming
		for ( int i = 8; i > 0; --i )
		{
			int bitmask = ( 1 << i );

			///We convert the bit to ascii '1' or '0'
			if ( obj.B & bitmask )
				out << "1";
			else
				out << "0";
		}
		return out;
	}

	u8bits_access operator[]( int i )
	{
		///Operator overloads within an operator overload can't be done without another object
		u8bits_access R = u8bits_access( i, &B );
		return R;
	}
};																	

//This allows us to use an 8-bit variable as an array of booleans AND a single variable
union u8byte
{
	ubyte Byte;
	u8bits Bit;
};

#endif
