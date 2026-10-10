#include <rover/core/global.h>
#include <rover/levels/region.h>


Region_Tile Region::Get_Tile( ushort x, ushort y )
{
	if ( x < regionWidth && y < regionHeight )
	{
		__int8 index = ( y * regionWidth ) + x;
		return Region_Tile( &region_tiles, index );
	}
	else
	{
		assert( 0 );
	}
}
