#include "environment.h"

#include "creature.h"

#include "chipmunk.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define GROUND_LENGTH 100000

typedef struct creatureListNode *creatureListNode_t;

struct creatureListNode {
	Creature creature;
	creatureListNode_t next;
};

struct environment {
	cpSpace *space;
	cpBody *staticBody;

	int width;
	int height;
	
	cpFloat groundHeight;
};


Environment createEnvironment( int width, int height ) {
	Environment env = malloc( sizeof( struct environment ) );
	assert( env != NULL );
	
	/*
	 * Simulation parameters
	 */	
	env->width = width;
	env->height = height;
	
	/*
	 * Create the Space
	 */
	env->space = cpSpaceNew( );
	env->space->iterations = 10; // physics accuracy
	cpSpaceResizeStaticHash( env->space, 30.0f, 1000 );
	cpSpaceResizeActiveHash( env->space, 30.0f, 1000 );
	env->space->gravity = cpv( 0, -200 );
	

	// create a static physics body (fixed in space)
	env->staticBody = cpBodyNew( INFINITY, INFINITY );

	
	/*
	 * Create the Ground
	 */	
	// space left/right of the center point
	cpFloat halfWidth = width/2.0f * GROUND_LENGTH;
	// space top/bottom of the center point
	cpFloat halfHeight = height/2.0f;
	
	// height of ground
	env->groundHeight = 10.0f;
	
	
	/*
	printf( "%f, %f\n", halfWidth,-halfHeight );
	printf( "%f, %f\n", -halfWidth,-halfHeight );
	printf( "%f, %f\n", -halfWidth,-halfHeight+groundHeight );
	printf( "%f, %f\n", halfWidth,-halfHeight+groundHeight );
	*/
	// polygon for ground shape
	cpVect groundPoly[] = {
		cpv( halfWidth, -halfHeight ),
		cpv( -halfWidth, -halfHeight  ),
		cpv( -halfWidth, -halfHeight+env->groundHeight ),
		cpv( halfWidth, -halfHeight+env->groundHeight )
	};
	
	
	// create a ground shape, attached to the staticBody
	cpShape *ground = cpPolyShapeNew( env->staticBody, 4, groundPoly, cpvzero );
	ground->e = 1.0f; ground->u = 1.0f;
	ground->group = 0;
	
	// add this ground shape to the physics space
	cpSpaceAddStaticShape( env->space, ground );

	return env;
}

void updateEnvironment( Environment env ) {
	cpSpaceStep( env->space, 1.0f/60.0f );
}

void destroyEnvironment( Environment env ) {
	cpBodyFree( env->staticBody );
	cpSpaceFreeChildren( env->space );
	cpSpaceFree( env->space );
	free( env );
}

cpSpace *getEnvironmentSpace( Environment env ) {
	return env->space;
}

void displayEnvironment( Environment env, char *message, cpVect center ) {
	#ifndef NOGRAPHICS
	static double lastX = 0.0;
	double changeRate = 0.95;
	double y = -center.y;
	drawSpaceOptions options = {
		0,
		0,
		1,
		4.0f,
		0.0f,
		1.5f,
	};
	
	glClear( GL_COLOR_BUFFER_BIT );

	
	// space left/right of the center point
	cpFloat halfWidth = env->width/2.0f;
	// space top/bottom of the center point
	cpFloat halfHeight = env->height/2.0f;
	
	
	glPushMatrix(); {
		if ( y > 0.0f ) y = 0.0f;
		glTranslatef(lastX, y, 0.0f);
		lastX = changeRate * lastX + (1.0f-changeRate) * -center.x;
		
		glColor3f( 1.0f, 0.0f, 0.0f );
		
		glBegin(GL_LINES); {
			glVertex2f(0.0f, halfHeight/4.0f);
			glVertex2f(20.0f, halfHeight/4.0f - 10.0f);
			
			glVertex2f(20.0f, halfHeight/4.0f - 10.0f);
			glVertex2f(0.0f, halfHeight/4.0f - 20.0f);
			
			
			glVertex2f(0.0, halfHeight/4.0f);
			glVertex2f(0.0, -halfHeight+env->groundHeight);
		} glEnd();
		
		
		
		drawSpace( env->space, &options );
		
		
		
		
	} glPopMatrix();
	
	glPushMatrix(); {
		glTranslatef(0.0, y, 0.0f);
	
		glColor3f( 0.0f, 0.0f, 0.0f );
		// draw ground line
		glBegin(GL_LINES); {
			glVertex2f(-halfWidth, -halfHeight+env->groundHeight);
			glVertex2f(halfWidth, -halfHeight+env->groundHeight);
		} glEnd();
	} glPopMatrix();
	
	if ( center.x)
	drawString( -300, -210, message );

	glutSwapBuffers();
	#endif
}
