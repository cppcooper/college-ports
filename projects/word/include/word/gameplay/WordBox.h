#pragma once
#include <assert.h>
#include <string>
#include <iof_chunk.h>

using ushort = unsigned short;
struct WordBox
{
	char* Letters = nullptr;
	ushort width = 0;
	ushort height = 0;

	char& Get_Letter( short x, short y )
	{
		if ( x < width && x >= 0 && y < height && y >= 0 )
		{
			return Letters[( y * width ) + x];
		}
		else
		{
			static char zero = 0;
			return zero;
		}
	}
	void Load_Data( std::string file_name );
};