#include <hypermaze/world/Terrain.h>

bool Terrain::inBounds( const glm::vec2& position ) const
{
	return ( 0 <= position.x && position.x < width && 0 <= position.y && position.y < height );
}

bool Terrain::canMove( const glm::vec2& position )
{
	if ( inBounds( position ) )
	{
		switch ( m_TileValues[( ( (uint32)position.y * width ) + (uint32)position.x )] )
		{
			case Tile::Blank:
			case Tile::Wall1:
			case Tile::Wall2:
			case Tile::DoorClosed:
			case Tile::GateClosed:
				return false;
		}
		return true;
	}
	return false;
}
