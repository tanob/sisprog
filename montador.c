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
   showMem();
   testandoValores();
   return 0;
}
