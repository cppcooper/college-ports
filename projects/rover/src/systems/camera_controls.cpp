#include <rover/core/global.h>
#include <rover/systems/engine/game.h>

//Rotates the camera to indicated angle over time (aka animated rotation)
void game::Rotate_Camera( float angle )
{
	glm::vec3 axis;
	axis.y = angle < 0 ? -1 : 1;
	angle = angle < 0 ? angle * -1 : angle;
	float angle_segment = angle / 60.0f;
	while ( angle > 0 )
	{
		///We lock the view matrix so that the Draw thread can't execute
		View_Mutex.lock();
		///Rotate camera
		View_Angle = glm::rotate( View_Angle, angle_segment, axis );
		///Calculate view matrix
		View_Matrix = View_Angle * View_Position;
		///Unlock to allow Draw thread to execute
		View_Mutex.unlock();

		///decrease remainder
		angle -= angle_segment;
		std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
	}

	///We are rotated, we must calculate our axis of movement in relation to the camera angle
	//(Forward, Backward, Left, Right)
	glm::vec4 dir = View_Angle * glm::vec4( 0, 0, 1, 0 );
	player_straight_axis = glm::vec3( dir.x, dir.y, dir.z );
	dir = View_Angle * glm::vec4( 1, 0, 0, 0 );
	player_side_axis = glm::vec3( dir.x, dir.y, dir.z );
}



//Remaining methods are self explanatory
///##############################
void game::Rotate_Left()
{
	Rotate_Camera( 90.0f );
}

void game::Rotate_Right()
{
	Rotate_Camera( -90.0f );
}

void game::Move_Forward()
{
	View_Mutex.lock();
	View_Position = glm::translate( View_Position, player_straight_axis );
	View_Matrix = View_Angle * View_Position;
	View_Mutex.unlock();
}

void game::Move_Backward()
{
	View_Mutex.lock();
	View_Position = glm::translate( View_Position, -1.0f * player_straight_axis );
	View_Matrix = View_Angle * View_Position;
	View_Mutex.unlock();
}

void game::Move_Left()
{
	View_Mutex.lock();
	View_Position = glm::translate( View_Position, 1.0f * player_side_axis );
	View_Matrix = View_Angle * View_Position;
	View_Mutex.unlock();
}

void game::Move_Right()
{
	View_Mutex.lock();
	View_Position = glm::translate( View_Position, -1.0f * player_side_axis );
	View_Matrix = View_Angle * View_Position;
	View_Mutex.unlock();
}
