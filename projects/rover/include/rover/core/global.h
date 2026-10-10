#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include <mutex>

#include <rover/systems/engine/3d_engine.h>
#include <rover/systems/engine/game.h>


extern engine_3d& get_cooper3d();
extern game& get_game_engine();

extern string fonts_dir;
extern string meshes_dir;
extern string sprites_dir;
extern string shaders_dir;
extern string textures_dir;

extern FFont* Fcout;
extern std::mutex View_Mutex;
extern glm::mat4 View_Matrix;
extern glm::mat4 View_Angle;
extern glm::mat4 View_Position;

extern unsigned int seed_value;
extern unsigned int seed_value2;

void **Matrix2D( int ncols, int  nrows, int element_size, int first_row, int first_col );
extern void DoInput( int key, int scancode, int action, int mods );
extern void DoCursor( double x, double y );
extern void DoMouseClick( int button, int action, int mods );

#endif
