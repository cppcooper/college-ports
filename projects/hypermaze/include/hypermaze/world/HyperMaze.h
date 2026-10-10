#ifndef _HYPERMAZE_H_
#define _HYPERMAZE_H_

#include <hypermaze/core/STL.h>
#include <hypermaze/world/Graph.h>
#include <hypermaze/world/TileMap.h>

class HyperMaze : public TileMap
{
private:
	std::unordered_map<void*, Terrain*> m_TerrainStorage;
	TileMap* m_TileMap = this;
	Terrain* m_CurrentTerrain = nullptr;
	Graph::GraphSet* m_Maze = nullptr;
	void* m_CurrentSpace = nullptr;
	Graph::Node* m_CurrentNode = nullptr;
	Graph::NodeLink* m_CurrentNodeLink = nullptr;
	Graph::GraphDirection m_EntryWall = Graph::GD_INVALID;

	uint32 Terrain_SX;
	uint32 Terrain_SY;
	uint32 Traversal_X;
	uint32 Traversal_Y;

protected:
#define get_index( x, y )  ( ( y * width ) + x )
	Graph::Node* FindNode( int16 terrain_x, int16 terrain_y, Graph::GraphDirection travel_dir );
	Graph::NodeLink* FindNodeLink( int16 terrain_x, int16 terrain_y, Graph::GraphDirection travel_dir );
	void EnterNode( Graph::Node* n );
	void EnterNodeLink( Graph::NodeLink* l, const Graph::GraphDirection& travel_dir );
	Terrain* CreateTerrain( Graph::Node* n );
	Terrain* CreateTerrain( Graph::NodeLink* l );
	void MeasureTerrain( Graph::Node* n, Terrain& t );
	void MeasureTerrain( Graph::NodeLink* l, Terrain& t );

public:
	HyperMaze();
	~HyperMaze();
	TileMap* GetTileMap() const;

	void Traverse( const uint32 &x, const uint32 &y, const Graph::GraphDirection travel_dir );
	bool CanMove( const uint32 &x, const uint32 &y );
	bool InBounds( const uint32 &x, const uint32 &y );
};

#endif
