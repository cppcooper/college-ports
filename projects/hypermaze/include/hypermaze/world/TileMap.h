#ifndef _TILEMAP_H_
#define _TILEMAP_H_

#include <GF/game_faculties.h>
#include <hypermaze/core/STL.h>

struct Terrain;
struct coordinate
{
	uint32 x;
	uint32 y;
	coordinate() : x( 0 ), y( 0 ){}
	coordinate( uint32 px, uint32 py ) :x( px ), y( py ){}
};

//TODO: Attach TileChunk to the SceneGraph?
class TileChunk
{
	friend class TileMap;
private:
	enum { width = 32, height = 32 };
	enum { size = width * height };
	std::array<uint16, size> m_Region = {};
	TileSet* m_TileSet = nullptr;

public:
	void SetTiles( Terrain* t, coordinate Chunk_Offset, coordinate Terrain_Offset );
	uint16 GetTile( uint8 x, uint8 y );
	void Draw( const glm::mat4& modelMatrix );
};

class TileMap : public Drawable
{
private:
	std::unordered_map<uint64, TileChunk*> m_Map;
	std::vector<TileChunk*> m_DrawArray;
	coordinate m_Origin;
	coordinate m_Upper;

protected:
	TileSet* m_TileSet = nullptr;

public:
	void Init();
	uint16 TileWidth() const {
		if ( m_TileSet )
			return m_TileSet->Width();
		else
			return 0;
	}
	uint16 TileHeight() const {
		if ( m_TileSet )
			return m_TileSet->Height();
		else
			return 0;
	}
	uint16 GetTile( uint32 tile_x, uint32 tile_y );
	//The coordinate system is Bottom-Left oriented and so is Place()
	void Place( Terrain* t, uint32 tile_x, uint32 tile_y );
	void Scroll( const glm::vec2& dir );
	void Draw( const glm::mat4& modelMatrix ) final override;
	
};

#endif
