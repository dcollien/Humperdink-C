#ifndef NOGRAPHICS
#include "display.h"
#endif

#include "chipmunk.h"

typedef struct environment *Environment;

Environment createEnvironment( int width, int height );

void updateEnvironment( Environment env );

void destroyEnvironment( Environment env );

void displayEnvironment( Environment env, char *message, cpVect center );

cpSpace *getEnvironmentSpace( Environment env );