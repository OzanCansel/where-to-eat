#include <iostream>
#include "placer.h"

int main( int argc , char* argv[] )
{
    if ( argc > 1 )
    {
        std::string command { argv[ 1 ] };

        if ( command == "summary" )
        {
            placer::summary( "." );
        }
        else if ( command == "print" )
        {
            if ( argc == 2 )
            {
                std::cerr << "Placement file wasn't specified." << std::endl;

                return 1;
            }

            placer::print_placement( argv[ 2 ] );
        }
        else if ( command == "peek" )
        {
            placer::peek( "." );
        }
    }
    else
    {
        placer::next( "." );
    }

    return 0;
}