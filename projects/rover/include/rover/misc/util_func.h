#ifndef _DEBUG_FUNCS_H_
#define _DEBUG_FUNCS_H_
#define LOG_CUSTOM_LEVEL

#include <iostream>

extern std::string execution_path;
extern std::string& get_exec_path( int directories_to_cut );
extern inline std::string coords_ToString(std::pair<int, int> position);

#endif