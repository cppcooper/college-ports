#include <hypermaze/core/Debugger.h>
#include <hypermaze/world/HyperMaze.h>
#include <hypermaze/world/Terrain.h>

inline uint8 GenerateWallTile()
{
	return ( rand() % ( 1 + Tile::Wall2 - Tile::Wall1 ) ) + Tile::Wall1;
}

inline uint8 GenerateFloorTile()
{
	return ( rand() % ( 1 + Tile::Tile2 - Tile::Tile1 ) ) + Tile::Tile1;
}

inline glm::vec2 LinkToNode_Offset( Graph::GraphDirection dir )
{
	assert( dir != Graph::GD_INVALID );
	switch ( dir )
	{
		case Graph::GD_L:
		{
			return glm::vec2( -1, 0 );
		}
		case Graph::GD_D:
		{
			return glm::vec2( 0, -1 );
		}
		case Graph::GD_R:
		{
			return glm::vec2( 1, 0 );
		}
		case Graph::GD_U:
		{
			return glm::vec2( 0, 1 );
		}
	}
}

HyperMaze::HyperMaze()
{
	Traversal_X = 0;
	Traversal_Y = 0;
	m_TileSet = Asset_Factory<TileSet>::Instance().GetAsset( "hypermaze.tdat" );
	assert( m_TileSet != nullptr );

	m_Maze = new Graph::GraphSet();
	Graph::Node* N = Graph::Generate( *m_Maze, 14, 1.f );
	EnterNode( N );
}

HyperMaze::~HyperMaze()
{
	//TODO: Delete maze graph
	/* Step 1: Compile an array of NodeLink pointers
	* Step 2: Delete all the Nodes from the Graph::GraphSet
	* Step 3: Delete pointers from array
	*/
}

#pragma region "Public Methods"

TileMap* HyperMaze::GetTileMap() const
{
	return m_TileMap;
}

void HyperMaze::Traverse( const uint32& x, const uint32& y, const Graph::GraphDirection travel_dir )
{
	// X and Y are [TILE MAP] coordinates
	debugger::Instance().Traversal_X = x;
	debugger::Instance().Traversal_Y = y;
	debugger::Instance().Travel_Dir = travel_dir;
	int16 terrain_X = x - Terrain_SX;
	int16 terrain_Y = y - Terrain_SY;
	Traversal_X = x;
	Traversal_Y = y;
	if ( m_CurrentNode == m_CurrentSpace )
	{
		EnterNodeLink( FindNodeLink( terrain_X, terrain_Y, travel_dir ), travel_dir );
	}
	else if ( m_CurrentNodeLink == m_CurrentSpace )
	{
		EnterNode( FindNode( terrain_X, terrain_Y, travel_dir ) );
	}
	else
	{
		assert( 0 );
	}
}

bool HyperMaze::CanMove( const uint32& x, const uint32& y )
{
	return m_CurrentTerrain->canMove( glm::vec2( x - Terrain_SX, y - Terrain_SY ) );
}

bool HyperMaze::InBounds( const uint32& x, const uint32& y )
{
	return m_CurrentTerrain->inBounds( glm::vec2( x - Terrain_SX, y - Terrain_SY ) );
}

#pragma endregion


#pragma region "Protected Methods"

//TODO: Thoroughly test FindNode method
Graph::Node* HyperMaze::FindNode( int16 terrain_x, int16 terrain_y, Graph::GraphDirection travel_dir )
{
	logger::LogStream logLine = logger::LogFile_Manager::Get( 0, _DEBUG3 );
	logLine << "HyperMaze::FindNode.."
		<< newl << "Terrain Coords: " << terrain_x << ", " << terrain_y
		<< newl << "Travel Dir: " << travel_dir;
	// [N1]----> N2
	if ( travel_dir == m_CurrentNodeLink->N1Dir() )
	{
		m_EntryWall = m_CurrentNodeLink->N2Dir();
		logLine << newl << "Entry Wall: " << m_EntryWall;
		return m_CurrentNodeLink->N2;
	}
	// N1 <----[N2]
	m_EntryWall = m_CurrentNodeLink->N1Dir();
	logLine << newl << "Entry Wall: " << m_EntryWall;
	return m_CurrentNodeLink->N1;
}

Graph::NodeLink* HyperMaze::FindNodeLink( int16 terrain_x, int16 terrain_y, Graph::GraphDirection travel_dir )
{
	auto &node = *m_CurrentNode;
	uint8 index = 0;
	switch ( travel_dir )
	{
		case Graph::GD_L:
		{
			index = terrain_y / 2;
			index = ( node.LW_Doors - 1 ) - index;
			break;
		}
		case Graph::GD_R:
		{
			index = terrain_y / 2;
			break;
		}
		case Graph::GD_D:
		{
			index = terrain_x / 2;
			
			break;
		}
		case Graph::GD_U:
		{
			index = terrain_x / 2;
			index = ( node.TW_Doors - 1 ) - index;
			break;
		}
	}
	assert( index >= 0 );
	return node.GetLink( index, travel_dir );
}

//TODO: Verify Completeness EnterNode(n)
void HyperMaze::EnterNode( Graph::Node* n )
{
	assert( n != nullptr );
	debugger::Instance().Node = (uint64)n;
	LOGFILE_1( _DEBUG3 ) << "HyperMaze::EnterNode.."
		<< newl << "Node: " << n;
	m_CurrentSpace = (void*)n;
	
	auto it = m_TerrainStorage.find( (void*)n );
	if ( it != m_TerrainStorage.end() )
	{
		m_CurrentTerrain = it->second;
	}
	else
	{
		m_CurrentTerrain = CreateTerrain( n );
		m_TerrainStorage[(void*)n] = m_CurrentTerrain;
	}
	glm::vec2 DoorCoords( 0, 0 );
	if ( m_CurrentNodeLink )
	{
		DoorCoords = n->GetLinkCoords( m_CurrentNodeLink, m_EntryWall );
	}
	Terrain_SX = Traversal_X - (int)( DoorCoords.x );
	Terrain_SY = Traversal_Y - (int)( DoorCoords.y );

	m_CurrentNode = n;
	Place( m_CurrentTerrain, Terrain_SX, Terrain_SY );
}

void HyperMaze::EnterNodeLink( Graph::NodeLink* l, const Graph::GraphDirection& travel_dir )
{
	assert( l != nullptr );
	debugger::Instance().NodeLink = (uint64)l;
	m_CurrentSpace = (void*)l;
	m_CurrentNodeLink = l;
	auto it = m_TerrainStorage.find( (void*)m_CurrentNodeLink );
	if ( it != m_TerrainStorage.end() )
	{
		m_CurrentTerrain = it->second;
	}
	else
	{
		m_CurrentTerrain = CreateTerrain( m_CurrentNodeLink );
		m_TerrainStorage[(void*)m_CurrentNodeLink] = m_CurrentTerrain;
	}
	//TODO: validate offset calculation
	glm::vec2 offset;
	switch ( travel_dir )
	{
		case Graph::GD_L:
		{
			offset.x = 0.f - ( m_CurrentTerrain->width - 1 );
			offset.y = 0.f - 1;
			break;
		}
		case Graph::GD_R:
		{
			offset.x = 0.f;
			offset.y = 0.f - 1;
			break;
		}
		case Graph::GD_D:
		{
			offset.x = 0.f - 1;
			offset.y = 0.f - ( m_CurrentTerrain->height - 1 );
			break;
		}
		case Graph::GD_U:
		{
			offset.x = 0.f - 1;
			offset.y = 0.f;
			break;
		}
	}
	Terrain_SX = Traversal_X + (int)offset.x;
	Terrain_SY = Traversal_Y + (int)offset.y;
	Place( m_CurrentTerrain, Terrain_SX, Terrain_SY );
}

//CREATE TERRAIN

Terrain* HyperMaze::CreateTerrain( Graph::Node* n )
{
	Terrain* T = new Terrain();
	MeasureTerrain( n, *T );
	unsigned char& width = T->width;
	unsigned char& height = T->height;
	std::vector<short>& TValues = T->m_TileValues;
	TValues.resize( width * height, 0 );

	short ix = 0, iy = 0;
	for ( ix = 1; ix + 1 < width; ++ix )
	{
		for ( iy = 1; iy + 1 < height; ++iy )
		{
			TValues[get_index( ix, iy )] = GenerateFloorTile();
		}
	}

	unsigned char door_count = 0;
	//TODO: Possibly rewrite Top & Left Loops to increment

#pragma region "Walls & Doors"

	// Bottom Wall / Doors
	for ( ix = iy = 0; ix < width; ++ix )
	{
		if ( ix % 2 == 1 && ++door_count <= n->BW_Doors )
		{
			//Create Doorway
			TValues[get_index( ix, iy )] = Tile::DoorOpen;
		}
		else
		{
			TValues[get_index( ix, iy )] = GenerateWallTile();;
		}
	}

	door_count = 0;
	// Right Wall / Doors
	for ( ix -= 1; iy < height; ++iy )
	{
		if ( iy % 2 == 1 && ++door_count <= n->RW_Doors )
		{
			//Create Doorway
			TValues[get_index( ix, iy )] = Tile::DoorOpen;
		}
		else
		{
			TValues[get_index( ix, iy )] = GenerateWallTile();;
		}
	}

	door_count = 0;
	// Top Wall / Doors
	for ( iy -= 1; ix >= 0; --ix )
	{
		if ( ix % 2 == 1 && ++door_count <= n->TW_Doors )
		{
			//Create Doorway
			TValues[get_index( ix, iy )] = Tile::DoorOpen;
		}
		else
		{
			TValues[get_index( ix, iy )] = GenerateWallTile();;
		}
	}

	door_count = 0;
	// Left Wall / Doors
	for ( ix = 0; iy >= 0; --iy )
	{
		if ( iy % 2 == 1 && ++door_count <= n->LW_Doors )
		{
			//Create Doorway
			TValues[get_index( ix, iy )] = Tile::DoorOpen;
		}
		else
		{
			TValues[get_index( ix, iy )] = GenerateWallTile();;
		}
	}

#pragma endregion

	return T;
}

Terrain* HyperMaze::CreateTerrain( Graph::NodeLink* l )
{
	Terrain* T = new Terrain();
	MeasureTerrain( l, *T );
	unsigned char& width = T->width;
	unsigned char& height = T->height;
	std::vector<short>& TValues = T->m_TileValues;
	TValues.resize( width * height, 0 );

	short ix, iy;
	//Selects the loop optimized for this NodeLink's alignment
	//TODO: Add tile sprite changes based on state of the loop
	if ( height < width )
	{
		//Loops horizontally
		for ( iy = 0 ; iy < height; ++iy )
		{
			for ( ix = 0; ix < width; ++ix )
			{
				if ( iy != 0 && iy + 1 != height )
				{
					TValues[get_index( ix, iy )] = Tile::PathHorizontal;
				}
				else
				{
					TValues[get_index( ix, iy )] = GenerateWallTile();
				}
			}
		}
	}
	else
	{
		//Loops vertically
		for ( ix = 0; ix < width; ++ix )
		{
			for ( iy = 0; iy < height; ++iy )
			{
				if ( ix != 0 && ix + 1 != width )
				{
					TValues[get_index( ix, iy )] = Tile::PathVertical;
				}
				else
				{
					TValues[get_index( ix, iy )] = GenerateWallTile();
				}
			}
		}
	}

	return T;
}

//MEASURE TERRAIN

void HyperMaze::MeasureTerrain( Graph::Node* n, Terrain& t )
{
	using uchar = unsigned char;
	uchar Left = n->LW_Doors;
	uchar Right = n->RW_Doors;
	uchar Top = n->TW_Doors;
	uchar Bot = n->BW_Doors;

	t.width = Top < Bot ? Bot : Top;
	t.width = t.width != 0 ? t.width * 2 + 1 : 3;
	t.height = Left < Right ? Right : Left;
	t.height = t.height != 0 ? t.height * 2 + 1 : 3;
}

void HyperMaze::MeasureTerrain( Graph::NodeLink* l, Terrain& t )
{
	bool YAxes = ( l->N1Dir() / 2 ) != 0; // X = 0; Y = 1;
	t.width = YAxes ? 3 : l->Length;
	t.height = YAxes ? l->Length : 3;
}

#pragma endregion
