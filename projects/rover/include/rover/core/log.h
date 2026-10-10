///We define the LOG_MAX_LEVEL because we don't want the default levels enabled
#define LOG_MAX_LEVEL (dbg_log::logFATAL )///+ dbg_log::logERROR + dbg_log::logWARNING + dbg_log::logINFO + dbg_log::logDEBUG1 + dbg_log::logDEBUG2)/// + dbg_log::logDEBUG3 + dbg_log::logDEBUG4 )
#include <logtools.h>
using namespace dbg_log;