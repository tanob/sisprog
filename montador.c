/*
   montador.c   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "montador_io.h"


int main( int argc, char *argv[] )
{
   getParamInfo( argc, argv );
   showParamInfo();
   loadFile2Memory();
   //showMem();
   //testandoValores();
   walking_into( 0, NULL );
   walking_into( 1, NULL );
   walking_into( 2, NULL );
   getchar();
   return 0;
}
