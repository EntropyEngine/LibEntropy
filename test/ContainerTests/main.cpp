#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "doctest_summary.hpp"

int main( int argc, char **argv )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );

	doctest::Context context;

	context.setOption( "no-path-filenames", true );
	context.setOption( "reporters", "console,summary" );
	context.applyCommandLine( argc, argv );

	int res = context.run();

	if ( context.shouldExit() ) return res;

	return res;
}