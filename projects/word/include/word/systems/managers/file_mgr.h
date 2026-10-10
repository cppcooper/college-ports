#ifndef _FILE_MGR_H_
#define _FILE_MGR_H_

#include <winfs_tools.h>

///C++ STL
#include <unordered_set>
#include <map>
#include <algorithm>

//This class manages all(most) application file look-ups
class File_Manager
{
private:
	//This is our tool for querying directories
	static dir_lister& Get()
	{
		static dir_lister Singleton_Dir_Lister;
		return Singleton_Dir_Lister;
	}

	//A list of loaded directories
	static std::unordered_set<std::string>& Get_Loaded()
	{
		static std::unordered_set<std::string> Singleton_Loaded_List;
		return Singleton_Loaded_List;
	}

public:
	//Register a directory with the File Manager and load its contents into the database
	static void Register_Directory( std::string path )
	{
		///We don't want to load this directory if we've already loaded it
		if ( Get_Loaded().emplace( path ).second )
		{
			///We haven't yet loaded this directory
			Get().Set_Path( path );
			Get().Find_Files();
			///Now we have
		}
	}

	//Retrieve all files with the corresponding extension type
	static file_dir_map& Get_Files( std::string extension )
	{
		///convert the indicated extension to lower case
		std::transform( extension.begin(), extension.end(), extension.begin(), ::tolower );

		///Look it up, find all matching files
		return Get().Get_Files_List( extension );
	}
};

#endif
