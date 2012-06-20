/*
 * Creature ADT:
 *  For converting genome array to a tree structure
 *  and creating physics simulation objects
 */


#include <stdint.h>
#include <math.h>
#include <jansson.h>
#include "chipmunk.h"

// ADT types
typedef struct creature *Creature;
typedef struct creatureNode *CreatureNode;

/*
 * Each creature is made up of a tree with a maximum of 16 nodes
 * (of arbitrary arrangement)
 * each node is parameterised by 5 parameters (described in the enum below)
 * An array of these parameters, in a depth-first order, defines the creature's
 * 'genome'
 */
#define NUM_PARAMETERS 6

#define MAX_FORCE 100000
#define FRICTION 0.2f

/*
 *  - number of branches that connect to this node
 *  - length of this node's parent-connecting branch
 *  - frequency of the oscillation of this branch (relative to parent)
 *  - the amplitude of the oscillation of this branch
 *  - the rotation angle (starting position) of the oscillation of this branch
 */
typedef struct {
	int numConnections;
	double angle;
	double length;
	double frequency;
	double amplitude;
	double phase;
} parameters_t;

/*
 * ADT interface functions
 */

/*
 * Constructor: Creates a node from a set of parameters
 * Deconstructor: Removes memory allocated for a node
 */
CreatureNode createCreatureNode( parameters_t parameters, CreatureNode parent, cpSpace *space );
void destroyCreatureNode( CreatureNode node );

/*
 * Constructor: Creates a creature and all nodes from a genome
 * Deconstructor: Removes memory for all nodes of the creature
 */
Creature createCreature( json_t *json, cpSpace *space );
void destroyCreature( Creature creature );

double getCreatureX( Creature creature );
double getCreatureY( Creature creature );
cpVect getCreaturePosition( Creature creature );

void printCreatureDebug( Creature creature );

int getCreatureNumLimbs( Creature creature );
cpVect *getCreatureLimbPositions( Creature creature );