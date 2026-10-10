#include <hypermaze/world/Graph.h>
#include <tools_logger.h>
using namespace logger;
namespace Graph
{

	Node* Generate( GraphSet& graph, ushort N, float C )
	{
		assert( graph.empty() );
		assert( N >= 2 );
		assert( C >= 0 && C <= 1.0 );

		//Mathematical function to help determine probability fields
		auto W = [&]( ushort X )
		{
			double Slope = cos( C * PI );
			return 5 + ( Slope * sin( X*( PI / 5 ) ) );
		};

#pragma region "RNG Setup"
		std::vector<double> intervals1{3, 4, 5, 6, 7, 8};
		std::vector<double> intervals2{2, 3};
		std::vector<double> weights1{W( 3 ) * 1, W( 4 ) * 2, W( 3 ) * 2, W( 5 ) * 3, W( 7 ) * 3, W( 6 ) * 2};
		std::vector<double> weights2{W( 3 ) * 10, W( 7 ) * 5};

		std::piecewise_linear_distribution<double> BigRoom_DoorGen( intervals1.begin(), intervals1.end(), weights1.begin() );
		std::piecewise_linear_distribution<double> SmallRoom_DoorGen( intervals2.begin(), intervals2.end(), weights2.begin() );
		std::bernoulli_distribution BIGROOM_Chance( 0.1 + ( 0.5f * C ) );
		std::bernoulli_distribution DEADEND_Chance( 0.87 - ( 0.62 * C ) );
		std::bernoulli_distribution LOOP_Chance( 0.23 + ( 0.42f * C ) );
		std::uniform_int_distribution<int> DirectionGen( 0, 3 );
		std::default_random_engine RNG( (unsigned int)time( NULL ) );
		srand( (unsigned int)time( NULL ) );
		
		intervals1.resize( 0 );
		intervals2.resize( 0 );
		weights1.resize( 0 );
		weights2.resize( 0 );
#pragma endregion


		std::deque<Node*> ExpansionQ;
		Node* Root = new Node();
		ExpansionQ.push_back( Root );
		Root->Doors.resize( 1 );

		ushort iN = N;
		ushort Parents = 0;
		ushort Children = 1;
		auto AbortChildDecision = [&]()
		{
			return DEADEND_Chance( RNG ) && Children > 3;
		};

		// Loops for N family generations
		do
		{
			assert( Children > 0 );
			Parents = Children;
			Children = 0;

			for ( int p = 0; p < Parents; ++p )
			{
				Node* PNode = ExpansionQ.front(); //Parent Node
				ExpansionQ.pop_front();
				graph.emplace( PNode );

				ushort PNode_Children = PNode->Doors.size();
				Children += PNode_Children;
				for ( int c = 0; c < PNode_Children; ++c )
				{
					if ( AbortChildDecision() )
					{
						Children -= 1;
					}
					else
					{
#pragma region "Doorway->N2 Assignment"
						NodeLink* Doorway = new NodeLink();
						Doorway->N1 = PNode;
						Doorway->Length = 4 + ( rand() % 5 );
						Doorway->SetAlignment( (GraphDirection)DirectionGen( RNG ) );
						PNode->push_back( Doorway );

						if ( !graph.empty() && !ExpansionQ.empty() && LOOP_Chance( RNG ) )
						{
							Children -= 1;
							Node* LNode = nullptr;
#pragma region "Create Loopback NodeLink"
							ushort X = rand() % graph.size();
							auto iter = graph.cbegin();
							while ( X-- != 0 )
							{
								iter = std::next( iter );
							}
							LNode = *iter;
							Doorway->N2 = LNode;
#pragma endregion
							LNode->push_back( Doorway );
						}
						else
						{
							Node* CNode = new Node(); //Child Node
							Doorway->N2 = CNode;
							CNode->push_back( Doorway );//Shrink
							
							ushort Door_Count = (ushort)( BIGROOM_Chance( RNG ) ? BigRoom_DoorGen( RNG ) : SmallRoom_DoorGen( RNG ) );
							CNode->Doors.resize( Door_Count );//Grow / Prepare

							ExpansionQ.push_back( CNode );
						}
#pragma endregion
					}
				}
			}
		} while ( --iN > 0 );


		// Whatever is in Queued must be added to the GraphSet
		while ( !ExpansionQ.empty() )
		{
			graph.emplace( ExpansionQ.front() );
			ExpansionQ.pop_front();
		}

		return Root;
	}

	GraphDirection InvertDir( GraphDirection alignment )
	{
		assert( alignment != GD_INVALID );
		switch ( alignment )
		{
			case GD_L:
			{
				return GD_R;
			}
			case GD_R:
			{
				return GD_L;
			}
			case GD_U:
			{
				return GD_D;
			}
			case GD_D:
			{
				return GD_U;
			}
		}
		return GD_INVALID;
	}


	void Node::push_back( NodeLink* NC )
	{
		assert( NC != nullptr );
		GraphDirection alignment = NC->GetNodeDir( this );
		LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG3;
		if ( NC->IsLooped() )
		{
			alignment = InvertDir( alignment );
		}
		LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG4;
		assert( alignment != GD_INVALID );
		switch ( alignment )
		{
			case GD_L:
			{
				LW_Doors += 1;
			}
			break;

			case GD_R:
			{
				RW_Doors += 1;
			}
			break;

			case GD_U:
			{
				TW_Doors += 1;
			}
			break;

			case GD_D:
			{
				BW_Doors += 1;
			}
			break;
		}
		Doors.resize( m_index + 1 );
		Doors[m_index++] = NC;
	}

	NodeLink* Node::GetLink( ushort index, GraphDirection alignment )
	{
		int current_index = 0;
		NodeLink* door = nullptr;
		LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG3;
		for ( unsigned int i = 0; i < Doors.size(); ++i )
		{
			door = Doors[i];
			assert( door != nullptr );
			if ( door->IsLooped() )
			{
				if ( door->N1Dir() == alignment || door->N2Dir() == alignment )
				{
					if ( index == current_index++ )
					{
						break;
					}
				}
			}
			else if ( door->GetNodeDir( this ) == alignment )
			{
				if ( index == current_index++ )
				{
					break;
				}
			}
		}
		LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG4;
		LOGFILE_1( _DEBUG4 ) << "Node::GetLink.."
			<< newl << "Node: " << this
			<< newl << "NodeLink: " << door
			<< newl << "NodeLink Alignment: " << alignment
			<< newl << "Current Wall Index: " << --current_index
			<< newl << "Total NodeLinks: " << Doors.size();
		return door;
	}

	glm::vec2 Node::GetLinkCoords( NodeLink* NC, GraphDirection Alignment )
	{
		if ( NC && Alignment != GD_INVALID )
		{
			int current_index = 0;
			LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG3;
			for ( unsigned int i = 0; i < Doors.size(); ++i )
			{
				NodeLink* door = Doors[i];
				GraphDirection DDir = door->GetNodeDir( this );
				assert( door != nullptr );
				if ( DDir == Alignment )
				{
					if ( NC != door )
					{
						current_index++;
					}
					else
					{
						break;
					}
				}
				else if ( door->IsLooped() && InvertDir( DDir ) == Alignment )
				{
					if ( NC != door )
					{
						current_index++;
					}
					else
					{
						break;
					}
				}
			}
			LogFile_Manager::GetLog( 0 ).ReportingLevel() = _DEBUG4;
			
			glm::vec2 door_coords( 0, 0 );
			ushort width = TW_Doors < BW_Doors ? BW_Doors : TW_Doors;
			width = width != 0 ? width * 2 + 1 : 3;
			ushort height = LW_Doors < RW_Doors ? RW_Doors : LW_Doors;
			height = height != 0 ? height * 2 + 1 : 3;
			switch ( Alignment )
			{
				case GD_L:
				{
					door_coords.x = 0.f;
					door_coords.y = ( height - 1 ) - ( ( current_index * 2 ) + 1.f );
					break;
				}
				case GD_R:
				{
					door_coords.x = width - 1.f;
					door_coords.y = ( current_index * 2 ) + 1.f;
					break;
				}
				case GD_D:
				{
					door_coords.x = ( current_index * 2 ) + 1.f;
					door_coords.y = 0.f;
					break;
				}
				case GD_U:
				{
					door_coords.x = (width-1) - ( ( current_index * 2 ) + 1.f );
					door_coords.y = height - 1.f;
					break;
				}
			}
			LOGFILE_1( _DEBUG4 ) << "Node::GetLinkCoords.."
				<< newl << "Node: " << this
				<< newl << "NodeLink: " << NC
				<< newl << "Alignment: " << Alignment
				<< newl << "NodeLink Wall-Index: " << current_index
				<< newl << "TW_Doors: " << (int)TW_Doors
				<< newl << "BW_Doors: " << (int)BW_Doors
				<< newl << "LW_Doors: " << (int)LW_Doors
				<< newl << "RW_Doors: " << (int)RW_Doors
				<< newl << "Width: " << width
				<< newl << "Height: " << height
				<< newl << "Door Coords: " << (int)door_coords.x << ", " << (int)door_coords.y;
			return door_coords;
		}
		else
		{
			return glm::vec2( 0, 0 );
		}
	}



	void NodeLink::SetAlignment( GraphDirection Alignment )
	{
		assert( Alignment != GD_INVALID );
		m_N1Dir = Alignment;
		m_N2Dir = InvertDir( Alignment );
	}

	GraphDirection NodeLink::GetNodeDir( Node* N )
	{
		assert( N != nullptr );
		return N1 == N ? N1Dir() : N2 == N ? N2Dir() : GD_INVALID;
	}

	bool NodeLink::IsLooped()
	{
		LOGFILE_1( _DEBUG4 ) << "NodeLink::IsLooped ?"
			<< newl << "NodeLink: " << this
			<< newl << "Node 1: " << N1
			<< newl << "Node 2: " << N2;
		return N1 == N2;
	}

	glm::vec2 NodeLink::GetNodeCoords( Node* N )
	{
		GraphDirection dir = GetNodeDir( N );
		assert( dir != GD_INVALID );
		glm::vec2 coords( 1, 1 );
		switch ( dir )
		{
			case GD_L:
			{
				coords.x = Length - 1.f;
				break;
			}
			case GD_D:
			{
				coords.y = Length - 1.f;
				break;
			}
			case GD_R:
			{
				coords.x = 0.f;
				break;
			}
			case GD_U:
			{
				coords.y = 0.f;
				break;
			}
		}
		return coords;
	}

}
