#include "creature.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// physics simulation
#include "chipmunk.h"


// thickness of creature limbs
#define SHAPE_RADIUS 4.0f

// mass per 1 unit of limb length
#define MASS_PER_LENGTH 0.25f

static int groupCount = 1;

/*
 * Node ADT data that defines a single limb of a creature
 *  - array of connecting limbs
 *  - parameters for this limb
 *  - size of this limb sub-tree
 */
struct creatureNode {
	CreatureNode *connections;
	int numConnections;
	
	parameters_t parameters;
	int treeSize;
	
	cpBody *body;
	
	// end of limb, where new limbs connect
	cpVect endPoint;
	
	cpFloat length;
	cpFloat angle;
	
	CreatureNode parent;
};

/*
 * Creature ADT data
 * root node of limb-tree, and the corresponding genome 
 */
struct creature {
	CreatureNode root;
	json_t *json;
};

/*
 * Private helper function prototypes
 */
static void destroyCreatureNodeTree( CreatureNode node );
static CreatureNode createNodesFromJSON( json_t *jsonObject, CreatureNode parent, cpSpace *space );




static CreatureNode createNodesFromJSON( json_t *jsonObject, CreatureNode parent, cpSpace *space ) {
	assert( json_is_object( jsonObject ) );
	
	// Borrowed references, no need for memory collection
	json_t *jsonConnections = json_object_get( jsonObject, "connections" );
	assert( json_is_array( jsonConnections ) );
	
	json_t *jsonAngle     = json_object_get( jsonObject, "angle" );
	json_t *jsonLength    = json_object_get( jsonObject, "length" );
	json_t *jsonFrequency = json_object_get( jsonObject, "frequency" );
	json_t *jsonAmplitude = json_object_get( jsonObject, "amplitude" );
	json_t *jsonPhase     = json_object_get( jsonObject, "phase" );
	
	parameters_t parameters;
	
	parameters.numConnections = json_array_size( jsonConnections );
	parameters.angle          = json_real_value( jsonAngle );
	parameters.length         = json_real_value( jsonLength );
	parameters.frequency      = json_real_value( jsonFrequency );
	parameters.amplitude      = json_real_value( jsonAmplitude );
	parameters.phase          = json_real_value( jsonPhase );
	
	CreatureNode node = createCreatureNode( parameters, parent, space );
	node->treeSize = 1;
	
	for ( int i = 0; i < parameters.numConnections; ++i ) {
		json_t *jsonChildObject = json_array_get( jsonConnections, i );
		
		node->connections[i] = createNodesFromJSON( jsonChildObject, node, space );
		node->treeSize += node->connections[i]->treeSize;
	}
	
	return node;
}


/*
 * ADT implementation
 */

CreatureNode createCreatureNode( parameters_t parameters, CreatureNode parent, cpSpace *space ) {
	// create memory for node
	CreatureNode node = malloc( sizeof( struct creatureNode ) );
	assert( node != NULL );
	
	// copy parameters into the node
	node->parameters = parameters;
	
	// find how many connections this node has
	node->numConnections = node->parameters.numConnections;
	
	// create memory (array) for references to the connecting nodes
	node->connections = malloc( sizeof( CreatureNode ) * node->numConnections );
	assert( node->connections != NULL );
	
	
	// create physics objects
	
	node->parent = parent;
	
	cpVect position;
	cpFloat baseAngle;
	
	if ( parent == NULL ) {
		position = cpvzero;
		baseAngle = M_PI/2.0;
	} else {
		position = parent->endPoint;
		baseAngle = parent->angle;
	}


	// angle (create unit vector)
	node->angle = baseAngle + node->parameters.angle;
	cpVect limbVec = cpvforangle( node->angle );

	// limb vector (multiply unit vector by length)
	node->length = node->parameters.length;
	limbVec = cpvmult( limbVec, node->length );

	// endpoint (add limb vector to starting position)
	node->endPoint = cpvadd( limbVec, position );
	
	cpFloat mass = node->parameters.length * MASS_PER_LENGTH;

	node->body = cpSpaceAddBody( 
		space, 
		cpBodyNew( mass, cpMomentForSegment( mass, cpvzero, limbVec ) )
	);
	
	// move 
	node->body->p = position;

	// shape (starting position to endpoint, relative to body)
	cpShape *shape = cpSegmentShapeNew( node->body, cpvzero, limbVec, SHAPE_RADIUS );
	shape->group = groupCount++;
	shape->u = FRICTION;
	
	cpSpaceAddShape(
		space, 
		shape
	);
	
	
	if ( parent != NULL ) {
		//printf( "  connecting to parent\n" );
		// joint (connect this limb to its parent)
		cpSpaceAddConstraint(
			space, 
			cpPivotJointNew( node->body, parent->body, position )
		);
	
	
		cpFloat amplitude = node->parameters.amplitude;
		cpFloat frequency = node->parameters.frequency;
		cpFloat phaseShift = node->parameters.phase;
	
		cpConstraint *motor = cpSpaceAddConstraint( 
			space,
			cpOscillatingMotorNew( node->body, parent->body, frequency, amplitude, phaseShift )
		);
		
		motor->maxForce = MAX_FORCE;
	}
	
	return node;
}

void destroyCreatureNode( CreatureNode node ) {
	// destroy everything malloced in the constructor
	free( node->connections );
	free( node );
}

Creature createCreature( json_t *json, cpSpace *space ) {
	// allocate memory for the creature
	Creature creature = malloc( sizeof( struct creature ) );
	assert( creature != NULL );
	
	// recursively create nodes
	creature->root = createNodesFromJSON( json, NULL, space );
	
	//printf( "Creature Size: %d\n", creature->root->treeSize );
	//printf( "Position: (%lf, %lf)\n", creature->root->body->p.x, creature->root->body->p.y );
	
	return creature;
}

void destroyCreature( Creature creature ) {
	// remove all nodes recursively from the root
	destroyCreatureNodeTree( creature->root );
	
	free( creature );
}


double getCreatureX( Creature creature ) {
	return creature->root->body->p.x;
}

double getCreatureY( Creature creature ) {
	return creature->root->body->p.y;
}

cpVect getCreaturePosition( Creature creature ) {
	return creature->root->body->p;
}

void printCreatureDebug( Creature creature ) {
	fprintf( stderr, "Creature Debug:\n" );
	fprintf( stderr, "--------------\n" );
	fprintf( stderr, "  Root Position: (%lf,%lf)\n", creature->root->body->p.x, creature->root->body->p.y );
	fprintf( stderr, "  Root Velocity: (%lf,%lf)\n", creature->root->body->v.x, creature->root->body->v.y );
	fprintf( stderr, "  Root rotational Velocity: %lf\n", creature->root->body->w );
	fprintf( stderr, "  Total Limbs: %d\n", creature->root->treeSize );
	fprintf( stderr, "  Limb Positions:\n");
	
	cpVect *limbPositions = getCreatureLimbPositions( creature );
	for ( int i = 0; i < creature->root->treeSize; ++i ) {
		fprintf( stderr, "   %d: (%lf,%lf)\n", i, limbPositions[i].x, limbPositions[i].y );
	}
	free( limbPositions );
	
	fprintf( stderr, "--------------\n\n" );
}

int getCreatureNumLimbs( Creature creature ) {
	return creature->root->treeSize;
}

cpVect *getCreatureLimbPositions( Creature creature ) {
	int numLimbs = creature->root->treeSize;
	
	cpVect *positions = malloc( sizeof( cpVect ) * numLimbs );
	assert( positions != NULL );
	
	int stackTop = 0;
	int limbNo = 0;
	CreatureNode limbStack[numLimbs];
	limbStack[0] = creature->root;
	
	while ( limbNo < numLimbs && stackTop >= 0) {
		CreatureNode limb = limbStack[stackTop];
		stackTop--;
		
		positions[limbNo] = limb->body->p;
		limbNo++;
		
		for ( int i = 0; i < limb->numConnections && limbNo+i < numLimbs; ++i ) {
			stackTop++;
			limbStack[stackTop] = limb->connections[i];
		}
	}
	return positions;
}


/*
 * Private helper function implementation
 */
static void destroyCreatureNodeTree( CreatureNode node ) {
	assert( node != NULL );
	
	int numConnections = node->parameters.numConnections;
	
	// destroy all connections recursively
	for( int i = 0; i < numConnections; ++i ) {
		destroyCreatureNodeTree( node->connections[i] );
	}
	
	destroyCreatureNode( node );
}
