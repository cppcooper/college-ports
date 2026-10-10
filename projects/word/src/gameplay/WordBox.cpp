#include <word/gameplay/WordBox.h>
#include <word/gameplay/WordSearch.h>
#include <fstream>

void WordBox::Load_Data( std::string file_name )
{
	std::ifstream File_Handle;
	File_Handle.open( file_name );

	File_Handle.seekg( 0, File_Handle.end );
	unsigned int File_Len = File_Handle.tellg( );
	File_Handle.seekg( 0, File_Handle.beg );

	char* buffer = new char[File_Len];
	buffer[File_Len - 1] = 0;

	File_Handle.read( buffer, File_Len );

	width = buffer[0];
	height = buffer[1];

	unsigned int puzzle_length = width * height;
	Letters = new char[puzzle_length];

	for ( int x = 0; x < width; ++x )
	{
		for ( int y = 0; y < height; ++y )
		{
			Letters[( y * width ) + x] = buffer[2 + ( y * width ) + x];
		}
	}

	//Words begin at 2+length+1
	/// First char should be {
	//Followed by words separated by commas `,`
	char* word_start = &buffer[2 + puzzle_length + 1];

	unsigned int pos = 0;
	unsigned int delimiter = 0;

	std::string WordList( word_start );

	while ( true )
	{
		//Test if delimiter is up to or at the comma
		delimiter = WordList.find( ',', pos );

		if ( delimiter != WordList.npos )
		{
			//Test if substring is from pos to delimiter or up to delimiter or some other variation
			std::string Word = WordList.substr( pos, delimiter - pos );

			//Change according to test results
			pos = delimiter + 1;

			WordSearch::Add_Answer_Word( Word );
		}
		else
		{
			delimiter = WordList.find( '}', pos );
			std::string Word = WordList.substr( pos, delimiter - pos );
			WordSearch::Add_Answer_Word( Word );
			break;
		}
	}
}
