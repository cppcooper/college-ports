#include <word/graphics/2d/sprite_batch.h>
#include <word/misc/bitpackers.h>

uintxx SpriteBatch::GlobalState;
glm::vec3 SpriteBatch::World_Displacement;
std::unordered_set<SpriteBatch*> SpriteBatch::Global_Batch;

void SpriteBatch::Initiatilize()
{
	static bool ready = false;
	if ( !ready )
	{
		GlobalState = uintxx( 4 );
		GlobalState.state = 0;
		ready = true;
	}
}

SpriteBatch::SpriteBatch()
{
	Initiatilize();
	GlobalState[total_batches]++;
	BatchState = uintxx( 4 );
	BatchState.state = 0;
}

SpriteBatch::~SpriteBatch()
{
	Global_Batch.erase( Global_Batch.find( &( *this ) ) );
}

void SpriteBatch::Add_Sprite( sprite& obj )
{
	if ( BatchState[added].Get_Value() == 0 )
	{
			BatchState[added] = 1;
			//Pushing this SpriteBatch* allocation
			Global_Batch.emplace( &( *this ) );
	}
	BatchState[total_sprites]++;
	Render_List.push_back( batched_sprite( obj ) );
}

void SpriteBatch::Draw()
{
	for ( auto iter = Render_List.begin(); iter != Render_List.end(); ++iter )
	{
		iter->Prepare( World_Displacement );
		iter->s->Draw();
	}
	World_Displacement.x = 0;
	World_Displacement.y = 0;
	World_Displacement.z = 0;
}

void SpriteBatch::Draw_All_Batches()
{
	for ( auto iter = Global_Batch.begin(); iter != Global_Batch.end(); ++iter )
	{
		( *iter )->Draw();
	}
}

void SpriteBatch::Add_Displacement( glm::vec3 &disp )
{
	World_Displacement += disp;
	disp.x = 0;
	disp.y = 0;
}
