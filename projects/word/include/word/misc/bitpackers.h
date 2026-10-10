#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES

#include <ostream>
#include <assert.h>

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
	u8bits_access( short Bit, ubyte* Byte )
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
																	

//This allows us to use an 8-bit variable as an array of booleans AND a single variable
union u8byte
{
	ubyte Byte;
	//uintxx Bit;
};

using uint = unsigned int;
using uint8 = unsigned __int8;
using ushort = unsigned short;

class Bit_Packed_Array
{
private:
	uint* Integer_Array = nullptr;
	uint bitmask;
	ushort bit_depth;
	ushort Array_Length;

public:
	Bit_Packed_Array( ushort UNIT_Array_Size, ushort bit_depth )
	{
		this->bit_depth = bit_depth;
		Array_Length = UNIT_Array_Size;
		bitmask = 0;
		for ( ushort i = 0; i < bit_depth; ++i )
		{
			bitmask |= 1 << i;
		}
		delete Integer_Array;
		Integer_Array = new uint[UNIT_Array_Size];
	}
};

class masked_uintxx
{
private:
	uint* Array_Element;
	ushort mask;
	uint bitmask;
	ushort bit_position;

public:
	masked_uintxx( uint* Array_Element, __int8 num_of_values, __int8 index )
	{
		ushort bits = sizeof( uint ) * 8;
		ushort bit_depth = bits / num_of_values;
		if ( index >= num_of_values )
		{
			assert( index < num_of_values );
		}

		this->Array_Element = Array_Element;
		bit_position = index * bit_depth;
		mask = 0;
		ushort i = 0;
		do
		{
			mask = mask << 1;
			mask += 1;
		} while ( ++i < bit_depth );
		bitmask = mask << bit_position;
	}
	//The bits within the mask will be kept of the parameter value
	masked_uintxx& operator=( ushort value )
	{ 
		uint& state = *Array_Element;
		//Zero the masked section
		*Array_Element &= ~bitmask;
		//Assign masked section the `value`
		*Array_Element |= ( mask & value ) << bit_position;
		state;
		return *this;
	}
	masked_uintxx& operator+=( ushort operand )
	{
		ushort new_value = Get_Value() + operand;
		*this = new_value;
		return *this;
	}
	masked_uintxx& operator-=( ushort operand )
	{
		ushort new_value = Get_Value() - operand;
		*this = new_value;
		return *this;
	}
	masked_uintxx& operator++()
	{
		( *this ) += 1;
		return *this;
	}
	masked_uintxx& operator--( )
	{
		( *this ) -= 1;
		return *this;
	}
	ushort Get_Value()
	{
		//Isolates masked section and returns the resultant value
		ushort t = ( ( *Array_Element & bitmask ) >> bit_position );
		return t;
	}
};

struct uintxx
{
	uint state = 0;
	__int8 num_of_values; // number of stored values inside of the state

	uintxx( __int8 num_of_values = 32, uint state = 0 )
	{
		this->num_of_values = num_of_values;
		this->state = state;
	}
	uintxx( uintxx& ob )
	{
		state = ob.state;
		num_of_values = ob.num_of_values;
	}

	masked_uintxx operator[]( __int8 i )
	{
		masked_uintxx R( &state, num_of_values, i );
		return R;
	}
};


#endif