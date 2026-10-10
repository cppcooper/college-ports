#include <sstream>
#include <string>
#include <winfs_tools.h>

#include <word/misc/util_func.h>
#include <word/systems/managers/file_mgr.h>


std::string execution_path;

//Returns the file path of the executable being run
std::string& get_exec_path(int directories_to_cut )
{
	///directories_to_cut specifies how many directory to cut off the result
	static wchar_t buffer[MAX_PATH];
	static bool done = false;
	if ( !done )
	{
		GetModuleFileName( NULL, buffer, MAX_PATH );
		done = true;
	}
	std::wstring unconverted( buffer );
	execution_path = std::string( unconverted.begin(), unconverted.end() );
	execution_path = execution_path.substr( 0, execution_path.find_last_of( "\\" ) );
	for ( int i = 0; i < directories_to_cut; ++i )
	{
		execution_path = execution_path.substr( 0, execution_path.find_last_of( "\\" ) );
	}

	execution_path += "\\";
	return execution_path;
}

inline std::string coords_ToString(std::pair<int, int> position)
{
	std::stringstream temp;
	temp << " --X:" << position.first << ", Y:" << position.second << "-- ";
	return temp.str();
}
