#pragma once
#include <unordered_set>
#include <vector>
#include <map>
#include <string>

#include <word/gameplay/WordBox.h>
#include <word/graphics/2d/sprite.h>
#include <word/misc/bitpackers.h>



class WordSearch
{
	struct position
	{
		short x = 0;
		short y = 0;
	};
	struct path
	{
		position S;
		position E;
		path( ushort sx = 0, ushort sy = 0, ushort ex = 0, ushort ey = 0 )
		{
			S.x = sx;
			S.y = sy;
			E.x = ex;
			E.y = ey;
		}
	};
	struct WordBoxWord
	{
		short Dir[2];
		position start;
		std::string word;
		WordBoxWord()
		{
			word = "";
		}
	};
	struct FoundWord
	{
		bool show = false;
		ushort length = 0;
		position start;
		std::string word;
		WordBoxWord* locale = nullptr;
		FoundWord()
		{
			word = "";
		}
	};

private:
	bool show_solution = false;
	sprite* highlight = nullptr;
	WordBox* data = nullptr;
	ushort largest_search_area = 0;
	std::vector<FoundWord> Words_Found;
	std::map<ushort, std::vector<WordBoxWord>> Search_Areas;
	static std::map<ushort, std::unordered_set<std::string>> Search_Words;
	short Selected_Word = -1;

protected:
	int Scan_AlongVector( 
		path search_path,
		short Dir[], 
		void( *increment )( WordBox* data, position& c ), 
		bool( *word_NotComplete )( path& p, position& t ) );
	void Scan_DiagonalDown();
	void Scan_DiagonalUp();
	void Scan_Horizontal();
	void Scan_Vertical();

public:
	static void Add_Answer_Word( std::string &word );
	void Scan_WordBox();
	void Draw();
	void SelectNext();
	void SelectLast();
	void ToggleSolution();

	WordSearch( WordBox* search_space );
	inline long int compare_strings( const std::string& search_word, const WordBoxWord& search_area )
	{
		uintxx comparison_results( 3 ); //We will bit pack our data inside this
		uintxx comparison_data;
		ushort bit_place = 0;
		__int8 i = 0;
		
		uint match_code = pow( 2, search_word.length() ) - 1;

		//Search forward
		for ( i = 0; i < search_area.word.length(); ++i )
		{
			if ( comparison_data.state != 0 ){ bit_place++; }
			if ( search_area.word[i] != search_word[bit_place] )
			{
				bit_place = 0;
				comparison_data.state = 0;
			}
			if ( search_area.word[i] == search_word[bit_place] )
			{
				comparison_data[bit_place] = 1;
				if ( comparison_data.state == match_code )
				{
					break;
				}
			}
		}
		if ( comparison_data.state == match_code )
		{
			comparison_results[0] = 1;
			comparison_results[1] = i;
		}


		//Search backward
		bit_place = 0;
		for ( i = search_area.word.length() - 1; i >= 0; --i )
		{
			if ( comparison_data.state != 0 ){ bit_place++; }
			if ( search_area.word[i] != search_word[bit_place] )
			{
				bit_place = 0;
				comparison_data.state = 0;
			}
			if ( search_area.word[i] == search_word[bit_place] )
			{
				comparison_data[bit_place] = 1;
				if ( comparison_data.state == match_code )
				{
					break;
				}
			}
		}
		if ( comparison_data.state == match_code )
		{
			comparison_results[2] = i;
		}
		return comparison_results.state;
	}

};
