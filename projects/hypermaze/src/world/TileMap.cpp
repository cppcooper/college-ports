#include <hypermaze/core/Debugger.h>
#include <hypermaze/world/Terrain.h>
#include <hypermaze/world/TileMap.h>

#include <GFX/gl_engine.h>
#include <tools_utilities.h>
#include <iostream>

#define LOWBITS 5
#define terrain_index( a, b )  ( ( ( b ) * terrain_width ) + ( a ) )
#define chunk_index( a, b )  ( ( ( b ) * width ) + ( a ) )

inline uint32 ChunkLocalCoord( uint32 value )
{
	return GetLowBits<uint32>( value, LOWBITS );
}

inline uint32 MapLocalCoord( uint32 value )
{
	return GetHighBits<uint32>( value, 32 - LOWBITS );
}

//TODO: Reimplement
void TileChunk::SetTiles( Terrain* t, coordinate Chunk_Offset, coordinate Terrain_Offset )
{
	auto &terrain_width = t->width;
	auto &TValues = t->m_TileValues;
	uint16 c_endX = width - Chunk_Offset.x - 1;
	uint16 t_endX = t->width - Terrain_Offset.x - 1;
	uint16 c_endY = height - Chunk_Offset.y - 1;
	uint16 t_endY = t->height - Terrain_Offset.y - 1;

	uint16 endX = t_endX < c_endX ? t_endX : c_endX;
	uint16 endY = t_endY < c_endY ? t_endY : c_endY;
	for ( uint16 y = 0; y <= endY; ++y )
	{
		for ( uint16 x = 0; x <= endX; ++x )
		{
			m_Region[chunk_index( Chunk_Offset.x + x, Chunk_Offset.y + y )]
				= TValues[terrain_index( Terrain_Offset.x + x, Terrain_Offset.y + y )];
		}
	}
}

uint16 TileChunk::GetTile( uint8 x, uint8 y )
{
	return m_Region[chunk_index( x, y )];
}

void TileChunk::Draw( const glm::mat4& modelMatrix )
{
	TileSet& tileset = *m_TileSet;
	for ( uint16 y = 0; y < height; ++y )
	{
		for ( uint16 x = 0; x < width; ++x )
		{
			glm::mat4 matrix = glm::translate( modelMatrix, glm::vec3( x * tileset.Width(), y * tileset.Height(), 0 ) );
			tileset[m_Region[chunk_index( x, y )]].Draw( matrix );
		}
	}
}

#define VALUE 1
void TileMap::Init()
{
	const uint16 width = (uint16)( Screen::Width() / (float)TileWidth() );
	const uint16 height = (uint16)( Screen::Height() / (float)TileHeight() );
	m_Origin.x = 1 - ( width / 2 );
	m_Origin.y = 1 - ( height / 2 );
	m_Upper = m_Origin;
	m_Upper.x += width + VALUE;
	m_Upper.y += height + VALUE;
}

uint16 TileMap::GetTile( uint32 x, uint32 y )
{
	uint32 chunk_x = MapLocalCoord( x );
	uint32 chunk_y = MapLocalCoord( y );
	uint64 key = MergeIntegers( chunk_x, chunk_y );
	auto it = m_Map.find( key );
	if ( it != m_Map.end() )
	{
		return it->second->GetTile( ( x - chunk_x ), ( y - chunk_y ) );
	}
	return -1;
}

//The coordinate systems are Bottom-Left oriented and so is Place()
void TileMap::Place( Terrain* t, uint32 starting_tile_x, uint32 starting_tile_y )
{
	debugger::Instance().Terrain = (uint64)t;
	debugger::Instance().Terrain_SX = starting_tile_x;
	debugger::Instance().Terrain_SY = starting_tile_y;
	uint32 increment = IncrementAtBit<uint32>( 0, LOWBITS + 1 );
	const coordinate Starting_Chunk( MapLocalCoord( starting_tile_x ), MapLocalCoord( starting_tile_y ) );
	const coordinate Starting_Chunk_Offset( ChunkLocalCoord( starting_tile_x ), ChunkLocalCoord( starting_tile_y ) );

	const uint8 c_cw = TileChunk::width; //Width of a Chunk
	const uint8 c_ch = TileChunk::height; //Height of a Chunk
	const uint16 c_ChunkColumns = (uint16)ceil( ( Starting_Chunk_Offset.x + t->width ) / (float)c_cw );
	const uint16 c_ChunkRows = (uint16)ceil( ( Starting_Chunk_Offset.y + t->height ) / (float)c_ch );

	coordinate Current_Chunk;
	coordinate Current_Chunk_Offset;
	coordinate Current_Chunk_Offset_Tile;
	coordinate Current_Terrain_Offset;

	TileChunk* chunk = nullptr;
	for ( uint32 r = 0; r < c_ChunkRows; ++r )
	{
		for ( uint32 c = 0; c < c_ChunkColumns; ++c )
		{
			Current_Chunk.x = Starting_Chunk.x + ( c * increment );
			Current_Chunk.y = Starting_Chunk.y + ( r * increment );

			Current_Chunk_Offset.x = ( c == 0 ? Starting_Chunk_Offset.x : 0 );
			Current_Chunk_Offset.y = ( r == 0 ? Starting_Chunk_Offset.y : 0 );

			Current_Chunk_Offset_Tile.x = Current_Chunk.x + Current_Chunk_Offset.x;
			Current_Chunk_Offset_Tile.y = Current_Chunk.y + Current_Chunk_Offset.y;

			Current_Terrain_Offset.x = Current_Chunk_Offset_Tile.x - starting_tile_x;
			Current_Terrain_Offset.y = Current_Chunk_Offset_Tile.y - starting_tile_y;

			uint64 key = MergeIntegers( Current_Chunk.x, Current_Chunk.y );
			auto it = m_Map.find( key );
			if ( it == m_Map.end() )
			{
				chunk = new TileChunk();
				chunk->m_TileSet = m_TileSet;
				it = m_Map.emplace( key, chunk ).first;
			}
			else
			{
				chunk = it->second;
			}
			chunk->SetTiles( t, Current_Chunk_Offset, Current_Terrain_Offset );
		}
	}
}

//TODO: Reimplement TileMap::Scroll( const glm::vec2& dir )
void TileMap::Scroll( const glm::vec2& dir )
{
	static const uint16 width = (uint16)( Screen::Width() / (float)TileWidth() );
	static const uint16 height = (uint16)( Screen::Height() / (float)TileHeight() );
	m_Origin.x += (int16)dir.x;
	m_Origin.y += (int16)dir.y;
	m_Upper = m_Origin;
	m_Upper.x += width + VALUE;
	m_Upper.y += height + VALUE;
	//TODO: Prepare new screen for fast rendering
}

//TODO: Redesign Draw() Loops
void TileMap::Draw( const glm::mat4& modelMatrix )
{
	int32 width = TileWidth();
	int32 height = TileHeight();
	uint32 increment = IncrementAtBit<uint32>( 0, LOWBITS + 1 );
	glm::vec3 offset;
	for ( int32 y = MapLocalCoord( (int32)m_Origin.y ); y < (int32)m_Upper.y; y += increment )
	{
		for ( int32 x = MapLocalCoord( (int32)m_Origin.x ); x < (int32)m_Upper.x; x += increment )
		{
			uint64 key = MergeIntegers( x, y );
			int32 pixel_x = ( x * width ) - ( (int32)m_Origin.x * width );
			int32 pixel_y = ( y * height) - ( (int32)m_Origin.y * height ); 
			//( y - (int32)m_Origin.y )* height;
			glm::mat4 matrix = glm::translate( modelMatrix, glm::vec3( pixel_x, pixel_y, 0.f ) );
			
			auto it = m_Map.find( key );
			if ( it != m_Map.end() )
			{
				it->second->Draw( matrix );
			}
			else
			{
				TileChunk* chunk = new TileChunk();
				chunk->m_TileSet = this->m_TileSet;
				m_Map.emplace( key, chunk );
				chunk->Draw( matrix );
			}
		}
	}
}
