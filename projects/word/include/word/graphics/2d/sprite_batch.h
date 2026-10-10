#include <vector>
#include <unordered_set>

#include <word/graphics/2d/sprite.h>
#include <word/misc/bitpackers.h>

class SpriteBatch
{
private:
	enum { total_batches, displacement };
	static uintxx GlobalState;
	enum { added, total_sprites };
	uintxx BatchState;
	struct batched_sprite
	{//64B + 18B = 82Bytes
		glm::mat4 m; //64 Bytes
		GLfloat a; //4 Bytes
		GLfloat x; //4 Bytes
		GLfloat y; //4 Bytes
		sprite* s; //4 Bytes
		ushort frame; //2 Bytes
		batched_sprite( sprite& obj )
		{
			frame = obj.frame_index;
			m = obj.modelMatrix;
			a = obj.alpha;
			x = obj.dest_x;
			y = obj.dest_y;
			s = &obj;
		}
		void Copy( sprite& obj )
		{
			frame = obj.frame_index;
			m = obj.modelMatrix;
			a = obj.alpha;
			x = obj.dest_x;
			y = obj.dest_y;
			s = &obj;
		}
		void Prepare( glm::vec3 &displacement )
		{
			s->frame_index = frame;
			s->modelMatrix = m;
			s->alpha = a;
			x += displacement.x;
			y += displacement.y;
			s->dest_x = x;
			s->dest_y = y;
		}
	};
	static std::unordered_set<SpriteBatch*> Global_Batch;
	std::vector<batched_sprite> Render_List;
	static glm::vec3 World_Displacement;

protected:
	static void Initiatilize();

public:
	SpriteBatch();
	~SpriteBatch();
	void Add_Sprite( sprite& obj );
	void Draw();
	static void Draw_All_Batches();
	static void Add_Displacement( glm::vec3 &disp );
};
