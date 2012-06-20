#include "display.h"

void drawString( int x, int y, char *str ) {
	glColor3f( 0.0f, 0.0f, 0.0f );
	glRasterPos2i( x, y );
	
	for ( int i=0, len=strlen(str); i<len; i++ ) {
		if ( str[i] == '\n' ) {
			y -= 16;
			glRasterPos2i( x, y );
		} else {
			glutBitmapCharacter( GLUT_BITMAP_HELVETICA_10, str[i] );
		}
	}
}
