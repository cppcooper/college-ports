#ifndef MAZEGRAPH_H
#define MAZEGRAPH_H

#include <GFX/OpenGL_Headers.h>
#include <hypermaze/core/STL.h>

#define PI 3.1415
namespace Graph
{
	struct Node;
	struct NodeLink;

	enum GraphDirection { GD_L, GD_R, GD_U, GD_D, GD_INVALID };

	using uchar = unsigned char;
	using ushort = unsigned short;

	using GraphSet = std::set < Node* > ;

	extern Node* Generate( GraphSet& g, ushort N = 2, float C = 0.5f );
	extern GraphDirection InvertDir( GraphDirection alignment );

	struct Node
	{
		ushort m_index = 0;
		uchar LW_Doors = 0; //Left Wall
		uchar RW_Doors = 0; //Right Wall
		uchar TW_Doors = 0; //Top Wall
		uchar BW_Doors = 0; //Bottom Wall
		std::vector<NodeLink*> Doors;

		void push_back( NodeLink* NC );
		NodeLink* GetLink( ushort index, GraphDirection alignment );
		glm::vec2 GetLinkCoords( NodeLink* NC, GraphDirection NDir );
	};

	struct NodeLink
	{
		Node* N1 = nullptr;
		Node* N2 = nullptr;
		ushort Length = 5;
	private:
		GraphDirection m_N1Dir = GD_INVALID;
		GraphDirection m_N2Dir = GD_INVALID;
	public:
		void SetAlignment( GraphDirection Alignment );
		GraphDirection N1Dir() const { return m_N1Dir; }
		GraphDirection N2Dir() const { return m_N2Dir; }
		GraphDirection GetNodeDir( Node* N );
		bool IsLooped();
		glm::vec2 GetNodeCoords( Node* N );
	};

}
#endif
