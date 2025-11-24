#include "Util.h"


Arguments ParseArgs(int argc, char** argv)
{
    Arguments args;

    for (uint32_t i = 0; i < (uint32_t) argc; ++i)
    {
        args.push_back( Argument{ i, argv[i] } );
    }

    return args;
}
