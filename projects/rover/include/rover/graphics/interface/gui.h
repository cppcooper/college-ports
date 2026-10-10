#ifndef GUI_H
#define GUI_H
#include <rover/graphics/2d/sprite.h>
#include <rover/graphics/interface/ffont.h>


/********************************
# Base Class
# Without `element` the GUI cannot be managed

* Created By:		Josh Cooper
* Modified By:		Josh Cooper	on Dec. 25, 2014
********************************/
class element
{
	public:
		glm::vec3 position;
		virtual void Draw() = 0;
};

/********************************
# Window Class
# This exists so that N elements can be nested within a single element (a window)
# Not fully integrated

* Created By:		Josh Cooper
* Modified By:		Josh Cooper	on Dec. 30, 2014
********************************/
class window : public element
{
	private:
		sprite* my_sprite;
		std::vector<element*> window_elements;

	public:
		window( sprite* my_sprite, std::vector<element*> window_elements )
		{
			this->my_sprite = my_sprite;
			this->window_elements.swap( window_elements );
		}
		virtual void Draw();
};

/********************************
# Text Class
# Draws text

* Created By:		Josh Cooper
* Modified By:		Josh Cooper	on Dec. 30, 2014
********************************/
class text : public element
{
	protected:
		std::string my_string;

	public:
		text( std::string my_text )
		{
			this->my_string = my_text;
		}
		virtual void Draw();
};

/********************************
# Button Class
# Interactive button
# Not fully implemented/integrated

* Created By:		Josh Cooper
* Modified By:		Josh Cooper	on Dec. 30, 2014
********************************/
class button : public element
{
	private:
		sprite* my_sprite;
		text* my_text_ptr;

	public:
		button( sprite* my_sprite, text* text_ptr )
		{
			this->my_sprite = my_sprite;
			this->my_text_ptr = text_ptr;
		}
		virtual void Draw();
};

/********************************
# Dynamic Text Class
# Draw variables as text without changing the text every time the variable values change
# ex:  base string: "Health: %i%"
# ex:  output: "Health: 210"

* Created By:		Josh Cooper
* Modified By:		Josh Cooper	on Dec. 30, 2014
********************************/
class dynamic_text : public text
{
	private:
		std::vector<void*> variable_list;

	public:
		dynamic_text( std::string my_text, std::vector<void*> variable_list ) : text( my_text )
		{
			this->variable_list.swap( variable_list );
		}
		virtual void Draw();
};

#endif
