#include <word/core/global.h>
#include <word/gameplay/WordSearch.h>
#include <vector>
#include <thread>
#include <future>



std::map<ushort, std::unordered_set<std::string>> WordSearch::Search_Words;

WordSearch::WordSearch( WordBox* search_space )
{
	highlight = get_game_engine().sprite_mgr.Get_Sprite( "tileset" );
	highlight->Scale( 24.0f );
	highlight[2];
	show_solution = true;
	data = search_space;
}


void WordSearch::Add_Answer_Word( std::string &word )
{
	ushort length = word.length();
	auto List_ITER = Search_Words.find( length );

	//We searched the word list for words of equal length
	if ( List_ITER != Search_Words.end( ) )
	{
		// If-> We found a list of words matching the length provided
		List_ITER->second.emplace( word );
	}
	else
	{
		//There are no words of this size at all
		std::unordered_set<std::string> new_list;
		new_list.emplace( word );

		Search_Words.emplace( length, new_list );
	}
}

void WordSearch::Scan_WordBox()
{
	Search_Areas.clear();
	Scan_DiagonalDown();
	Scan_DiagonalUp();
	Scan_Horizontal();
	Scan_Vertical();
	auto SearchAlg = [this]( std::promise<std::vector<FoundWord>>& promise, std::unordered_set<std::string>& search_words, std::vector<WordBoxWord>& search_areas )
	{
		std::vector<FoundWord> compared_matches;
		for ( int i = 0; i < search_areas.size(); ++i )
		{
			WordBoxWord& search_area = search_areas[i];
			for ( auto search_word = search_words.begin(); search_word != search_words.end(); search_word++ )
			{
				long int Comparison_Results = compare_strings( *search_word, search_area );
				if ( Comparison_Results != 0 )
				{
					uintxx data;
					data.num_of_values =3;
					data.state = Comparison_Results;
					ushort start_index = data[0].Get_Value() != 0 ? data[1].Get_Value() - search_word->length() + 1 : data[2].Get_Value();

					FoundWord word;
					word.word = *search_word;
					word.length = search_word->length();
					word.locale = &search_area;

					word.start.x = search_area.start.x;
					word.start.y = search_area.start.y;
					for ( int i = 0; i < start_index; ++i )
					{
						word.start.x += search_area.Dir[0];
						word.start.y += search_area.Dir[1];
					}

					compared_matches.push_back( word );
				}
			}
		}
		promise.set_value( compared_matches );
	};
	
	struct ThreadResults
	{
		std::future<std::vector<FoundWord>> future;
		std::promise<std::vector<FoundWord>> promise;
	};
	using std::thread;
	using Task = std::pair < thread*, ThreadResults* >;
	
	std::vector<Task> Task_List;
	
	//Search_Areas.get_allocator
	for ( auto search_word_list : Search_Words )
	{
		for ( int i = search_word_list.first; i <= largest_search_area; ++i )
		{
			if ( Search_Areas.find( i ) != Search_Areas.end() )
			{
				std::vector<WordBoxWord>& search_area_list = Search_Areas[i];
				ThreadResults* shared_state = new ThreadResults;
				shared_state->future = shared_state->promise.get_future();

				std::thread *T = new std::thread( SearchAlg, std::move( shared_state->promise ), search_word_list.second, std::ref( search_area_list ) );
				Task_List.push_back( std::make_pair( T, shared_state ) );
			}
		}
	}

	//While there are tasks we haven't checked on
	while ( Task_List.size() != 0 )
	{
		auto task = Task_List.begin();
		task->first->join();
		auto results = task->second->future.get();

		for ( auto word_iterator = results.begin(); word_iterator != results.end(); ++word_iterator )
		{
			this->Words_Found.push_back( *word_iterator );
		}
		delete task->first;// thread
		delete task->second; // data

		//Task data is saved and pointers deleted, erase the task
		Task_List.erase( task );
	}
	Selected_Word = Words_Found.empty()? -1 : 0;
	return void( 0 );
}


//It performs multiple scans on the word box along any lines that are parrallel to the vector V, requires a thought out function to increment the starting coordinates for each scan
int WordSearch::Scan_AlongVector( path search_path, short Dir[], void( *increment )( WordBox* data, position& c ), bool( *word_Incomplete )( path& p, position& t ) )
{
	ushort count = 0;
	position c;
	c.x = search_path.S.x;
	c.y = search_path.S.y;
	position T;
	//Assuming Top is Zero

	//Loop until goal coordinates are reached
	while ( true )
	{
		T.x = c.x;
		T.y = c.y;

		WordBoxWord WBW;
		WBW.start = T;
		WBW.Dir[0] = Dir[0];
		WBW.Dir[1] = Dir[1];
		
		//Continues to scan letters until either TX or TY reach the perimeter
		do
		{
			char letter = data->Get_Letter( T.x, T.y );
			if ( letter == 0 ) break; // The defined end must have been out of bounds
			WBW.word += letter;
			T.x += Dir[0];
			T.y += Dir[1];
			//repeat if Y isn't going back past the starting axis AND X isn't at the end yet
		} while ( word_Incomplete( search_path, T ) );
		
		if ( WBW.word.empty() ) break;

		int length = WBW.word.length();
		if ( length > largest_search_area )
			largest_search_area = length;

		auto List_ITER = Search_Areas.find( length );


		//We searched the word list for words of equal length
		if ( List_ITER != Search_Areas.end() )
		{
			// If-> We found a list of words matching the length provided
			List_ITER->second.push_back( WBW );
			count++;
		}
		else
		{
			//There are no words of this size at all
			std::vector<WordBoxWord> new_list;
			new_list.push_back( WBW );

			Search_Areas.emplace( length, new_list );
			count++;
		}
		increment( data, c );
	}
	return count;
}

void WordSearch::Scan_DiagonalDown()
{//Start Bottom Left
	short V[] = { 1, 1 }; //Right + Down
	auto increment_F = []( WordBox* data, position& c )
	{
		static ushort last_cx = c.x;
		c.x = c.y == 0 ? ++c.x : 0; //move right, once at the top
		c.y = c.y == 0 ? ( c.x == last_cx ? -42 : 0 ) : --c.y; //move up, until at the top
		last_cx = c.x;
	};
	auto word_Incomplete = []( path& p, position& t )
	{
		return ( t.y - 1 != p.S.y && t.x - 1 != p.E.x );
	};
	Scan_AlongVector( path( 0, data->height - 1, data->width, -42 ), V, increment_F, word_Incomplete );
}

void WordSearch::Scan_DiagonalUp()
{//Start Top Left
	short V[] = { 1, -1 }; //Right + Up
	auto increment_F = []( WordBox* data, position& c )
	{
		c.x = c.y != data->height - 1 ? c.x : ++c.x;
		c.y = c.y != data->height - 1 ? ++c.y : c.y;
	};
	auto word_Incomplete = []( path& p, position& t )
	{
		return ( t.y >= p.S.y && t.x - 1 != p.E.x );
	};
	Scan_AlongVector( path( 0, 0, data->width, data->height ), V, increment_F, word_Incomplete );
}

void WordSearch::Scan_Horizontal()
{
	short V[] = { 1, 0 }; //Right
	auto increment_F = []( WordBox* data, position& c )
	{
		c.y++;
	};
	auto word_Incomplete = []( path& p, position& t )
	{
		return t.x - 1 != p.E.x;
	};
	Scan_AlongVector( path( 0, 0, data->width - 1, data->height - 1 ), V, increment_F, word_Incomplete );
}

void WordSearch::Scan_Vertical( )
{
	short V[] = { 0, 1 }; //Down
	auto increment_F = []( WordBox* data, position& c )
	{
		c.x++;
	};
	auto word_Incomplete = []( path& p, position& t )
	{
		return t.y - 1 != p.E.y;
	};
	Scan_AlongVector( path( 0, 0, data->width - 1, data->height - 1 ), V, increment_F, word_Incomplete );
}

void WordSearch::Draw()
{
	const float y_start = 525.0f, y_increment = -26.0f;
	const float x_start = 175.0f, x_increment = 26.0f;

	Fcout->set_properties();

	if ( highlight )
	{
		int word_count = 0;
		for ( auto item : Words_Found )
		{
			Fcout->set_properties( 20.0f );
			if ( word_count == Selected_Word )
				Fcout->print( 0, 575.0f - ( (float)word_count++ * 13.0f ), "->" + item.word + "<-" );
			else
				Fcout->print( 0, 575.0f - ( (float)word_count++ * 13.0f ), item.word );

			if ( item.show )
			{
				float x, y;
				x = ( item.start.x * x_increment );
				y = ( item.start.y * y_increment );
				for ( int i = 0; i < item.length; ++i )
				{
					highlight->Draw( x_start + x, y_start + y );
					x += ( item.locale->Dir[0] * x_increment );
					y += ( item.locale->Dir[1] * y_increment );
				}
			}
		}
	}

	Fcout->set_properties( 36.0f );
	for ( int y = 0; y < data->height; ++y )
	{
		for ( int x = 0; x < data->width; ++x )
		{
			float px = x_start + ( x_increment * x );
			float py = y_start + ( y_increment * y );

			Fcout->print( px, py, data->Get_Letter( x, y ) );
		}
	}
}

void WordSearch::SelectNext()
{
	if ( !Words_Found.empty() )
	{
		if ( Selected_Word + 1 < Words_Found.size() )
		{
			if ( Selected_Word >= 0 )
			{
				Selected_Word++;
			}
			else
			{
				Selected_Word = 0;
			}
		}
	}
}

void WordSearch::SelectLast()
{
	if ( !Words_Found.empty() )
	{
		if ( Selected_Word - 1 >= 0 )
		{
			Selected_Word--;
		}
		else if ( Selected_Word == -1 )
		{
			Selected_Word = 0;
		}
	}
}

void WordSearch::ToggleSolution()
{
	if ( Selected_Word >= 0 )
	{
		bool& ref = Words_Found[Selected_Word].show;
		ref = !ref;
	}
}
