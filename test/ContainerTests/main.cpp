#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main( int argc, char **argv )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );

	doctest::Context context;

	context.setOption( "no-path-filenames", true );
	context.applyCommandLine( argc, argv );

	int res = context.run();

	if ( context.shouldExit() ) return res;

	return res;
}