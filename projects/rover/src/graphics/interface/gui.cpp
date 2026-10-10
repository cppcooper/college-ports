#include <rover/core/global.h>
#include <rover/graphics/interface/gui.h>

//Draws all the elements of a window object
void window::Draw()
{
	my_sprite->Draw();
	for ( auto iter = window_elements.begin(); iter != window_elements.end(); ++iter )
	{
		( *iter )->Draw();
	}
}

//Draws simple text
void text::Draw()
{
	Fcout->text_position = position;
	Fcout->print( my_string );
}

//Draws a button
void button::Draw()
{
	my_sprite->Draw();
	my_text_ptr->Draw();
}

//Takes a string and replaces key words with variable values found using a list of pointers
void dynamic_text::Draw()
{
	///We reconstruct our output string every time because values can change at any time
	std::string output;
	std::string var_value;

	///Each variable should be in the list, and in the correct order {else: undefined behaviour will be exhibited}
	auto var_iter = variable_list.begin();

	///Parse our string 'one' letter at a time
	for ( int i = 0; i < my_string.size(); ++i )
	{
		char temp = my_string[i];
		if ( temp == '%' )
		{
			char first = my_string[++i];
			char second = my_string[++i];

			///Variable flags must begin and end with a '%' character
			if ( second == '%' )
			{
				///Now we need to know which variable flag so we know which type to cast the variable to
				switch ( first )
				{
					case 'i':
						var_value = std::to_string( *(int*)( *var_iter ) );
						break;

					case 'f':
						var_value = std::to_string( *(float*)( *var_iter ) );
						break;

					case 'd':
						var_value = std::to_string( *(double*)( *var_iter ) );
						break;

					default:
						assert( true );
				}
				output.append( var_value );
			}
			else if ( second == 'p' && my_string[++i] == '%' )
			{
				///The user wants this float/double to be displayed as a percentage
				switch ( temp )
				{
					case 'f':
						var_value = std::to_string( ( *(float*)( *var_iter ) ) * 100 ) + '%';
						break;

					case 'd':
						var_value = std::to_string( ( *(double*)( *var_iter ) ) * 100 ) + '%';
						break;

					default:
						assert( true );
				}
				output.append( var_value );
			}
			else
			{
				i = i - 3;
			}
		}
		else
		{
			output.append( 1, temp );
		}
	}
	Fcout->text_position = position;
	///We finished parsing, now print it to screen
	Fcout->print( output );
}
