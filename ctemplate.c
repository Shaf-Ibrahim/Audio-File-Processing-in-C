/*
 * 
 * C template
 *
 * Eric R. Nelson
 * March 1, 2010
 * 
 */
#include <stdio.h>
#include <stdlib.h>

#include "atcs.h"


int main(int argc, char* argv[])
   {
   printf("%s\n", "Hello World!" );
   printf("size of LONG=%ld\n", sizeof(LONG));
   printf("If the above value is not 4, then you need to edit acts.h\n");

   exit(0);
   }
