Humperdink Simulator
---------------------
Version 0.2.0 Alpha
=====================

A "humperdink" is a virtual creature simulated in a 2D physics environment.

The humperdink is parameterised as a set of limbs connected by oscillating joints.

The parameters for each limb are:
 - angle (direction relative to the limb's parent)
 - length
 - frequency (how fast to move the limb)
 - amplitude (in radians, how far to turn on an oscillation)
 - phase (in modulo 2*pi, the phase shift of the sinusoid - e.g. a phase shift of pi would place the oscillation completely out of phase, turning in the opposite direction)


These parameters, and the connectivity of limbs are specified as a tree in JSON format, for example:

{
	"angle":3.14,
	"length":40.0,
	"connections": [
		{
			"angle":1.57,
			"length":20.0,
			"frequency":3.0,
			"amplitude":3.14,
			"phase":2.10,
			"connections": [
				{
					"angle":0.0,
					"length":40.0,
					"frequency":6.0,
					"amplitude":0.4,
					"phase":0.0,
					"connections": []
				}
			]
		},
		{
			"angle":-1.57,
			"length":20.0,
			"frequency":3.0,
			"amplitude":3.14,
			"phase":4.20,
			"connections": [
				{
					"angle":0.0,
					"length":40.0,
					"frequency":6.0,
					"amplitude":0.4,
					"phase":0.0,
					"connections": []
				}
			]
		}
	]
}


(whitespace non-essential)

Runtime Flags:

-f filename
specify the JSON file to read from (without this flag it defaults to stdin)

-i iterations
specify the number of iterations before termination

-s speed
specify the number of iterations to process before displaying a frame

-g
suppress graphical output


Compilation:

run:
"make"


Requires jansson JSON library, provided in tar.gz

Typical installation:
""""
./configure
make
sudo make install
""""

Compile with -DNOGRAPHICS
to make a non-graphical executable that does not require OpenGL libraries

requires Open GL and GLUT for graphical display.

on Ubuntu this may require:
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev

Nvidia hardware might prefer:
sudo apt-get install nvidia-glx-180-dev
in which case you'll need to change the Makefile's CFLAGS line to have:
-I/usr/share/doc/nvidia-glx-180-dev/include/



Compiling your program with the humperdink simulator:

The following shows a basic example:

#include "environment.h"
#include "creature.h"
#include <stdlib.h>
#include <stdio.h>

int main( int argc, char *argv[] ) {
	json_t *json;
	json_error_t error;
	
	char humperdink[] = "{\"angle\":0.0,\"length\":50.0,\"connections\": []}";
	
	// see also json_loadf and json_load_file
	// http://www.digip.org/jansson/doc/dev/tutorial.html
	json = json_loads( humperdink, &error );
	
	if( !json ) {
		fprintf( stderr, "Error parsing humperdink on line %d:\n", error.line );
		fprintf( stderr, "\t%s\n\n", error.text );
		exit( 0 );
	}
	
	
	Environment simulationEnvironment = createEnvironment( 800, 600 );
	Creature simulatedCreature = createCreature( json, getEnvironmentSpace( simulationEnvironment ) );
	
	int iterations = 100;
	
	for ( int i = 0; i < iterations; ++i ) {
		updateEnvironment( simulationEnvironment );
		double x = getCreatureX( simulatedCreature );
		double y = getCreatureY( simulatedCreature );
		printf( "(%lf, %lf)\n", x, y );
	}
	
	destroyCreature( simulatedCreature );
	destroyEnvironment( simulationEnvironment );
	
	return 0;
}