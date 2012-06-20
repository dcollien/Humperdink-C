#include "environment.h"
#include "creature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SLEEP_TICKS 16


static void timercall( int value );
static void display( void );
static void initGL( float width, float height );

static Environment simulationEnvironment;
static Creature simulatedCreature;
static int simulationSpeed;
static char message[1024];

static int iterations;

int main( int argc, char *argv[] ) {
	
	simulationSpeed = 1;
	iterations = 0; // infinite
	
	char *filename = NULL;
	
	bool graphics = true;
	
	#ifdef NOGRAPHICS
	graphics = false;
	#endif
	
	for ( int i = 0; i < argc; ++i ) {
		if ( strncmp( argv[i], "-f", 2 ) == 0 && i+1 < argc ) {
			filename = argv[i+1];
		} else if ( strncmp( argv[i], "-s", 2 ) == 0 && i+1 < argc ) {
			simulationSpeed = atoi( argv[i+1] );
		} else if ( strncmp( argv[i], "-i", 2 ) == 0 && i+1 < argc ) {
			iterations = atoi( argv[i+1] );
		} else if ( strncmp( argv[i], "-g", 2 ) == 0 ) {
			graphics = false;
		}
	}
	
	fprintf( stderr, "Simulating with a humperdink from " );
	if ( filename == NULL ) {
		fprintf( stderr, "stdin" );
	} else {
		fprintf( stderr, "%s", filename );
	}
	fprintf( stderr, ": %d iterations. Processing %d iterations at once.\n", iterations, simulationSpeed );
	
	json_t *json;
	json_error_t error;
	
	
	if ( filename == NULL ) {
		json = json_loadf( stdin, &error );
	} else {
		json = json_load_file( filename, &error );
	}
	
	if( !json ) {
		fprintf( stderr, "Error parsing humperdink on line %d:\n", error.line );
		fprintf( stderr, "\t%s\n\n", error.text );
		exit( 0 );
	}
	
	int width = 800;
	int height = 600;
	
	cpInitChipmunk( );
	
	simulationEnvironment = createEnvironment( width, height );
	simulatedCreature = createCreature( json, getEnvironmentSpace( simulationEnvironment ) );
	
	if ( graphics ) {
		
		#ifndef NOGRAPHICS
		glutInit( &argc, (char**)argv );

		glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );

		glutInitWindowSize( width, height );
		glutCreateWindow( "Humperdinks in their natural habitat" );

		initGL( width, height );

		glutDisplayFunc( display );
	//	glutIdleFunc(idle);
		glutTimerFunc( SLEEP_TICKS, timercall, 0 );
	
	
		// antialias
		glEnable( GL_LINE_SMOOTH );
		glEnable( GL_POINT_SMOOTH );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );
		glHint( GL_POINT_SMOOTH_HINT, GL_DONT_CARE );
	
		// go-go gadget GLUT!
		glutMainLoop();
		#endif
		
	} else {
		for ( int i = 0; i < iterations || iterations == 0; ++i ) {
			updateEnvironment( simulationEnvironment );
			printf( "(%lf, %lf)\n", getCreatureX( simulatedCreature ), getCreatureY( simulatedCreature ) );
		}
	}
	
	destroyCreature( simulatedCreature );
	destroyEnvironment( simulationEnvironment );
	
	return 0;
}

static void display( void ) {
	//printf( "(%lf, %lf)\n", getCreatureX( simulatedCreature ), getCreatureY( simulatedCreature ) );
	sprintf( message, "(%lf, %lf)", getCreatureX( simulatedCreature ), getCreatureY( simulatedCreature ) );
	displayEnvironment( simulationEnvironment, message, getCreaturePosition( simulatedCreature ) );
	
	for ( int i = 0; i < simulationSpeed; ++i ) {
		//printCreatureDebug( simulatedCreature );
		updateEnvironment( simulationEnvironment );
		printf( "(%lf, %lf)\n", getCreatureX( simulatedCreature ), getCreatureY( simulatedCreature ) );	
		iterations--;
	}
	
	if ( iterations == 0 ) {
		exit( 0 );
	}
}

static void timercall( int value ) {
	#ifndef NOGRAPHICS
	glutTimerFunc( SLEEP_TICKS, timercall, 0 );
	glutPostRedisplay( );
	#endif
}

static void initGL( float width, float height ) {
	#ifndef NOGRAPHICS
	glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	glOrtho( -(width/2.0f), (width/2.0f), -(height/2.0f), (height/2.0f), -1.0f, 1.0f );
	glTranslatef( 0.5f, 0.5f, 0.0f );
	
	glEnableClientState( GL_VERTEX_ARRAY );
	#endif
}

