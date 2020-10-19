// @Author Samuel Simeonov
// überarbeitet von: Dennys-Daniel Vogt, Lukas Hein

#include <stdio.h>
#include <stdlib.h>

#include <GL/freeglut.h>

#include <math.h>
#include <limits.h>
#include <time.h>
#include <string.h>

// #include "obj.h"

// Grafiklistennummerierung
#define RAKETE 1
#define PLANET 2
#define STAR 3
#define CRUISER 4
#define LIFE 5

// Konstanten für die Spiellogik
#define MAX_WAVE 5
#define START_HITS 3


void pause_display(void);

// Abmessungen des Koordinatensystems in Abhaengigkeit zum Bildseitenverhaeltnis
float width = 0.0;
float height = 0.0;

// Variablen zur Animation
GLint rocket_angle = 0;
GLint rockety = 0;
GLint rollangle = 0;
GLint steerx = 0;
GLint steery = 0;

// Variablenarrays fuer den Zufallsgenerator
int is_occupied[30];
int object_properties[30][2];

// Arrays für random stern generation
float rand_x[70];
float rand_y[70];

// Winkel zur Rotation von Monden in Vielfachen von PI
double ALPHA = M_PI/3; //rot um x
double BETA = M_PI/6; //rot um y
double GAMMA = M_PI/4; //rot um z
double PHI1 = 0.0; //Rotation relativ zur Zeit
double PHI2 = 0.0; //Rotation relativ zur Zeit

// Variablen zur lichtposition der Sonne 
float xpossun = 0;
float ypossun = 0;
float zpossun = 0;

// Spielvariablen (zum Verändern Variablen in new_game() ändern)
int hits = START_HITS;
int score = 0;
int difficulty = 1;
int level = 1;

// Variablen zur Ablaufsteuerung
bool collide = false;
bool life_pickup = false;
bool life_spawned = false;

bool gameover = false;
bool change_diff = false;
bool pause_btn = false;
int index1 = 0;	//Hilfvariable für Collide Blinken
int index2 = 0; //Hilfvariable für Pickup Blinken
int level_index = 1;
int prev_difficulty = 0;
double difficulty_scale = 1;

bool first_game_called = false;// Nötig um new_game mit richtiger width und height zu starten (siehe reshape())

unsigned int random_int;
unsigned int iterator = 0;

bool blight4 = false;
bool blight5 = false;
bool blight6 = false;

// Farbtypen der Planeten
typedef enum 
{
	blue,
	red,
	green,
	bronze,
	white,
	sun
}color_t;

// Schwierigkeitsgrade
typedef enum 
{
	easy,
	medium,
	hard,
	korean
}game_mode_t;

// Standard Schwierigkeitsgrad Mittel
game_mode_t gamemode = medium;

// Wechsel zwischen Pause und nicht Pause
void toggle_pause()
{
	if(pause_btn)
	{
		pause_btn = false;
	}else if(!pause_btn)
	{
		pause_btn = true;
	}
}

// Wechselnde Lichter in Level 9
void change_light()
{
	if (blight4 && !blight5 && !blight6)
	{
		glDisable(GL_LIGHT4);
		blight4= false;
		glEnable(GL_LIGHT5);
		blight5 = true;
		glLoadIdentity();
	}else if (!blight4 && blight5 && !blight6)
	{
		glDisable(GL_LIGHT5);
		blight5 = false;
		glEnable(GL_LIGHT6);
		blight6 = true;
		glLoadIdentity();
	}else if (!blight4 && !blight5 && blight6)
	{
		glDisable(GL_LIGHT6);
		blight6 = false;
		glEnable(GL_LIGHT4);
		blight4 = true;
		glLoadIdentity();
	}else if (blight4 && blight5 && blight6)
	{
		glDisable(GL_LIGHT5);
		glDisable(GL_LIGHT6);
		blight5 = false;
		blight6 = false;
		glLoadIdentity();
	}else if (!blight4 && !blight5 && !blight6)
	{
		glEnable(GL_LIGHT4);
		blight4 = true;
		glLoadIdentity();
	}	
}

// Erstellen von 70 Stern-Koordinaten prozentual auf die Bildschirmgroeße angepasst 
void sterne()
{
	srand(time(NULL));
	for (int i = 0; i < 70; i++)
	{
		rand_x[i] = -100 + rand() % 201;
		rand_y[i] = -100 + rand() % 201;
	}	
}

// Berechnung der Position der Monde im Levelhintergrund
void calcmoonpos(double radiusplanet, double radiusmoon, double radiusextent, double richtungsvector[3], float xposplanet, float yposplanet, float zposplanet, double PHI, bool sun)
{
	double posx, posy, posz;
	glTranslatef(xposplanet, yposplanet, zposplanet);
	double r = radiusplanet + radiusmoon + radiusextent;

	// normieren des richtungsvektors, mult mit länge r
	double normfaktor = 1/(sqrt(richtungsvector[0]*richtungsvector[0]+richtungsvector[1]*richtungsvector[1]+richtungsvector[2]*richtungsvector[2]));
	for (int i = 0; i < 3; i++)
	{
		richtungsvector[i] = richtungsvector[i] * normfaktor * r;
	}
	double StaticMatrixALPHA[] = {	1, 0,		   0,
							  		0, cos(ALPHA),-sin(ALPHA),
						      		0, sin(ALPHA), cos(ALPHA)};

	double StaticMatrixBETA[] = {	cos(BETA), 	0, sin(BETA),
							 		0, 			1, 0,
									-sin(BETA), 0, cos(BETA)};

	double StaticMatrixGAMMA[] = {	cos(GAMMA),-sin(GAMMA), 0,
						      		sin(GAMMA), cos(GAMMA), 0,
							  		0, 		  	0, 		  	1};

	double rotationsmatrix[] = {cos(PHI),-sin(PHI), 0,
							    sin(PHI), cos(PHI), 0,
								0, 		    0, 		1};
	// Drehung um x-Achse, deren Drehachse durch die StaticMatrizen geneigt wird
	posx = richtungsvector[0] * rotationsmatrix[0] + richtungsvector[1] * rotationsmatrix[1] + richtungsvector[2] * rotationsmatrix[2];
	posy = richtungsvector[0] * rotationsmatrix[3] + richtungsvector[1] * rotationsmatrix[4] + richtungsvector[2] * rotationsmatrix[5];
	posz = richtungsvector[0] * rotationsmatrix[6] + richtungsvector[1] * rotationsmatrix[7] + richtungsvector[2] * rotationsmatrix[8];

	richtungsvector[0] = posx;
	richtungsvector[1] = posy;
	richtungsvector[2] = posz;

	// Neigung der Drehung um die x-Achse
	posx = richtungsvector[0] * StaticMatrixALPHA[0] + richtungsvector[1] * StaticMatrixALPHA[1] + richtungsvector[2] * StaticMatrixALPHA[2];
	posy = richtungsvector[0] * StaticMatrixALPHA[3] + richtungsvector[1] * StaticMatrixALPHA[4] + richtungsvector[2] * StaticMatrixALPHA[5];
	posz = richtungsvector[0] * StaticMatrixALPHA[6] + richtungsvector[1] * StaticMatrixALPHA[7] + richtungsvector[2] * StaticMatrixALPHA[8];

	richtungsvector[0] = posx;
	richtungsvector[1] = posy;
	richtungsvector[2] = posz;

	// Neigung der Drehung um die y-Achse
	posx = richtungsvector[0] * StaticMatrixBETA[0] + richtungsvector[1] * StaticMatrixBETA[1] + richtungsvector[2] * StaticMatrixBETA[2];
	posy = richtungsvector[0] * StaticMatrixBETA[3] + richtungsvector[1] * StaticMatrixBETA[4] + richtungsvector[2] * StaticMatrixBETA[5];
	posz = richtungsvector[0] * StaticMatrixBETA[6] + richtungsvector[1] * StaticMatrixBETA[7] + richtungsvector[2] * StaticMatrixBETA[8];

	richtungsvector[0] = posx;
	richtungsvector[1] = posy;
	richtungsvector[2] = posz;

	// Neigung der Drehung um die z-Achse
	posx = richtungsvector[0] * StaticMatrixGAMMA[0] + richtungsvector[1] * StaticMatrixGAMMA[1] + richtungsvector[2] * StaticMatrixGAMMA[2];
	posy = richtungsvector[0] * StaticMatrixGAMMA[3] + richtungsvector[1] * StaticMatrixGAMMA[4] + richtungsvector[2] * StaticMatrixGAMMA[5];
	posz = richtungsvector[0] * StaticMatrixGAMMA[6] + richtungsvector[1] * StaticMatrixGAMMA[7] + richtungsvector[2] * StaticMatrixGAMMA[8];

	if(sun) // in Level 10 dreht sich eine Lichtquelle um den Planeten, deren Koordinaten extra gespeichert werden müssen
	{
		xpossun = posx;
		ypossun = posy;
		zpossun = posz;
	}
	glTranslatef(posx, posy, posz);
}

// Erzeugung von Planeten, Monden und der Sonne in Level 10
void createPlanet(float r, color_t color)
{
	GLfloat ambient[3] = {0,0,0};
	GLfloat diffuse[3] = {0,0,0};
	GLfloat specular[3] = {0,0,0};
	GLfloat shininess[1] = {50};

	switch (color)
	{
	case red:
		ambient[0] = 0.25;
		diffuse[0] = 0.5;
		specular[0] = 0.5;
		ambient[1] = 0.1;
		diffuse[1] = 0.1;
		specular[1] = 0.1;
		ambient[2] = 0.1;
		diffuse[2] = 0.1;
		specular[2] = 0.1;
		break;
	
	case green:
		ambient[1] = 0.25;
		diffuse[1] = 0.5;
		specular[1] = 0.5;
		ambient[0] = 0.1; 
		diffuse[0] = 0.1;
		specular[0] = 0.1;
		ambient[2] = 0.1;
		diffuse[2] = 0.1;
		specular[2] = 0.1;
		break;

	case blue:
		ambient[2] = 0.25;
		diffuse[2] = 0.5;
		specular[2] = 0.5;
		ambient[0] = 0.1;
		diffuse[0] = 0.1;
		specular[0] = 0.1;
		ambient[1] = 0.1;
		diffuse[1] = 0.1;
		specular[1] = 0.1;
		break;

	case bronze:
		ambient[0] = 0.21;
		ambient[1] = 0.13;
		ambient[2] = 0.05;
		diffuse[0] = 0.71;
		diffuse[1] = 0.43;
		diffuse[2] = 0.18;
		specular[0] = 0.39;
		specular[1] = 0.27;
		specular[2] = 0.17;
		shininess[0] = 26;
		break;

	case sun:
		ambient[0] = 1;
		ambient[1] = 1;
		ambient[2] = 0;
		diffuse[0] = 1;
		diffuse[1] = 1;
		diffuse[2] = 0;
		specular[0] = 1;
		specular[1] = 1;
		specular[2] = 0;
		shininess[0] = 100;
		break;
	
	default:
		// weiss
		for (int i = 0; i < 3; i++)
		{
			ambient[i] = 0.25;
			diffuse[i] = 0.5;
			specular[i] = 0.5;
		}
		break;
	}
	
	glNewList(PLANET,GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	  glutSolidSphere(r,64,64);
	glEndList();

	glCallList(PLANET);
	glLoadIdentity(); // es wird kein zweiter Planet an die selbe Stelle gezeichnet, daher Codeersparnis im Levelswitch (display()) 
}

//Lichtquellenkonfiguration
void light()
{
  GLfloat light0_position[] = { -(GLfloat)steerx, (GLfloat)(steery-2), 4.0, 1.0};
  GLfloat spot_direction[] = { 0.0, 1.0, -1.0 };

  GLfloat light1_position[] = {-(GLfloat)steerx, (GLfloat)(steery-2), 4.0, 1.0};
  GLfloat light1_ambient[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat light1_specular[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat light1_spot_dir[] = { 0.0, 1.0, -1.0 };

  GLfloat light2_position[] = {xpossun,ypossun,zpossun, 1.0};
  GLfloat light2_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat light2_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat light2_specular[] = { 0.0, 0.0, 0.0, 1.0 };

  GLfloat light3_position[] = {-(GLfloat)steerx, (GLfloat)(steery-2), 4.0, 1.0};
  GLfloat light3_ambient[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat light3_diffuse[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat light3_specular[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat light3_spot_dir[] = { 0.0, 1.0, -1.0 };
  
  GLfloat light4_position[] = { width-2, -height+2, 25.0, 1.0 };
  GLfloat light4_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light4_diffuse[] = { 0.0, 0.0, 1.0, 0.5 };
  GLfloat light4_specular[] = { 0.0, 0.0, 1.0, 0.5 };
  GLfloat light4_spot_dir[] = { -1.0, 1.0, -1.0 };

  GLfloat light5_position[] = { 0.0, height, 20.0, 1.0 };
  GLfloat light5_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light5_diffuse[] = { 0.0, 1.0, 0.0, 0.5 };
  GLfloat light5_specular[] = { 0.0, 1.0, 0.0, 0.5 };
  GLfloat light5_spot_dir[] = { 0.0, -1.0, -1.0 };

  GLfloat light6_position[] = { -width, -height, 20.0, 1.0 };
  GLfloat light6_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light6_diffuse[] = { 1.0, 0.0, 0.0, 0.5 };
  GLfloat light6_specular[] = { 1.0, 0.0, 0.0, 0.5 };
  GLfloat light6_spot_dir[] = { 1.0, 1.0, -1.0 };

  GLfloat light7_position[] = { 0, 0 , 40.0, 1.0};
  GLfloat light7_ambient[] = { 0.20, 0.20, 0.20, 1.0 };
  GLfloat light7_diffuse[] = { 0.05, 0.05, 0.05, 1.0 };
  GLfloat light7_specular[] = { 0.0, 0.0, 0.0, 1.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);// Spotlicht in weiss auf Cruiser
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60.0);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);// Spotlicht in rot bei Kollision auf Cruiser
  glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
  glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0);
  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_spot_dir);

  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);// Sonne als Kugelfoermige Lichtquelle
  glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);

  glLightfv(GL_LIGHT3, GL_POSITION, light3_position);// Spotlicht in gruen bei life pickup von Cruiser
  glLightfv(GL_LIGHT3, GL_AMBIENT, light3_ambient);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
  glLightfv(GL_LIGHT3, GL_SPECULAR, light3_specular);
  glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 60.0);
  glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light3_spot_dir);

  glLightfv(GL_LIGHT4, GL_POSITION, light4_position);// Spotlicht in blau von unten rechts
  glLightfv(GL_LIGHT4, GL_AMBIENT, light4_ambient);
  glLightfv(GL_LIGHT4, GL_DIFFUSE, light4_diffuse);
  glLightfv(GL_LIGHT4, GL_SPECULAR, light4_specular);
  glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 50.0);
  glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, light4_spot_dir);

  glLightfv(GL_LIGHT5, GL_POSITION, light5_position);// Spotlicht in gruen von oben mitte
  glLightfv(GL_LIGHT5, GL_AMBIENT, light5_ambient);
  glLightfv(GL_LIGHT5, GL_DIFFUSE, light5_diffuse);
  glLightfv(GL_LIGHT5, GL_SPECULAR, light5_specular);
  glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 50.0);
  glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, light5_spot_dir);

  glLightfv(GL_LIGHT6, GL_POSITION, light6_position);// Spotlicht in rot von unten links
  glLightfv(GL_LIGHT6, GL_AMBIENT, light6_ambient);
  glLightfv(GL_LIGHT6, GL_DIFFUSE, light6_diffuse);
  glLightfv(GL_LIGHT6, GL_SPECULAR, light6_specular);
  glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 50.0);
  glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, light6_spot_dir);

  glLightfv(GL_LIGHT7, GL_POSITION, light7_position);// Schwaches Licht in weiss auf gesamte Szene
  glLightfv(GL_LIGHT7, GL_AMBIENT, light7_ambient);
  glLightfv(GL_LIGHT7, GL_DIFFUSE, light7_diffuse);
  glLightfv(GL_LIGHT7, GL_SPECULAR, light7_specular);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT7);

}


// Hilfsfunktion zum prozeduralen erstellen mehrerer Raketen
void spawn_rocket(int i)
{
	if(abs(object_properties[i][1]) == 1)
	{
		int rotation = object_properties[i][1];
		int x = object_properties[i][2]*3;
		glPushMatrix();
			glLoadIdentity();
			glRotatef(-90.0*rotation,1.0,0.0,0.0);
			glPushMatrix();
				// rockety wird für die z-Ordinate genutzt, da die Rakete in die Ebene fliegend als Grafikliste vorliegt
			    glTranslatef(x,0.0,-(((GLfloat)rockety)/(10.0-(float)difficulty/4.0)-25.0));
			    glRotatef(rocket_angle,0.0,0.0,1.0);
	    		glCallList(RAKETE);
	  		glPopMatrix();
    	glPopMatrix();
    }
}

// Hilfsfunktion zum Erstellen von Leben
void spawn_life(int i)
{
	int rotation;
	if (abs(object_properties[i][1]) == 1)
	{
		rotation = object_properties[i][1];
		object_properties[i][1] = 2 * object_properties[i][1];
	} 
	else rotation = object_properties[i][1]/2;
	int x = object_properties[i][2]*3;
	glPushMatrix();
	  glLoadIdentity();
	  glRotatef(-90.0*rotation,1.0,0.0,0.0);
	  glPushMatrix();
	  	// rockety wird für die z-Ordinate genutzt, da das Extra-Leben in die Ebene fliegend als Grafikliste vorliegt
	    glTranslatef(x,0.0,-(((GLfloat)rockety)/(10.0-(float)difficulty/4.0)-25.0));
		glRotatef(rocket_angle,0.0,0.0,1.0);
		glRotatef(90,0,1,0);
	    glCallList(LIFE);
	  glPopMatrix();
    glPopMatrix();
}

// GAME OVER Schriftzug
 void game_over()
{
	glDisable(GL_LIGHTING);
	glColor3f(1,0,0);
	glRasterPos3f(-3.0,0.0,40.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 71); // G
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 65); // A
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 77); // M
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 69); // E
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32); // space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 79); // O
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 86); // V
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 69); // E
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 82); // R
	glLoadIdentity();
	glEnable(GL_LIGHTING);
	steerx = width*5; // Lichtquelle 1 aus dem Bildschirm bewegen
	steery = height*5;
}

// Änderung des Schwierigkeitsgrades
void toggle_gamemode(char Gamemode)
{
	if(Gamemode == 'e')
		gamemode = easy;
	else if (Gamemode == 'm')
		gamemode = medium;
	else if (Gamemode == 'h')
		gamemode = hard;
	else if (Gamemode == 'k')
		gamemode = korean;
}

// PAUSED Schriftzug
void pause_display()
{
	glDisable(GL_LIGHTING);
	glColor3f(0,0.8,0);
	glRasterPos3f(-1.75,0.0,40.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 80); //P
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 65); //A
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 85); //U
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 83); //S
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 69); //E
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 68); //D

	glLoadIdentity();
	glEnable(GL_LIGHTING);
}

// YOU WON! Schriftzug
void won()
{
	glDisable(GL_LIGHTING);
	glColor3f(0,1,0);
	glRasterPos3f(-3.0,0.0,20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 89); //Y
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 79); //O
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 85); //U
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32); //space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 87); //W
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 79); //O
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 78); //N
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 33); //!
	glEnable(GL_LIGHTING);
}

//Kollisionsabfrage via Bounding Box
void collision_check_rocket()
{
	for(int i = 0; i < difficulty; i++)
	{
		if (abs(object_properties[i][1]) == 1)
		{
			if(((-((float)rockety/(10.0-(float)difficulty/4.0)-25.0-2.5) > steery-2) && (-((float)rockety/(10.0-(float)difficulty/4.0)-25.0+2.5) < steery+3) && object_properties[i][1] == 1)
				|| ((((float)rockety/(10.0-(float)difficulty/4.0)-25.0-2.5) < steery+3) && (((float)rockety/(10.0-(float)difficulty/4.0)-25.0+2.5) > steery-2) && object_properties[i][1] == -1))
			{
				if((-(object_properties[i][2]*3)+1.5 > steerx-(3-((float)abs(rollangle%180)/45.0)))&&(-(object_properties[i][2]*3)-1.5 < steerx+(3-((float)abs(rollangle%180)/45.0))))
				{
					hits--;
					collide = true;	// maximal eine Kollision pro Welle
					if (hits == 0) gameover = true;
					break; // sobald erste Kollision gefunden: Abbruch
				}	 
			}
		}
	}
}

// Pickupabfrage via Bounding Box
void pickup_life()
{
	if(((-((float)rockety/(10.0-(float)difficulty/4.0)-25.0-1.5) > steery-2) && (-((float)rockety/(10.0-(float)difficulty/4.0)-25.0+1.5) < steery+3) && object_properties[difficulty][1] == 2)
		|| ((((float)rockety/(10.0-(float)difficulty/4.0)-25.0-1.5) < steery+3) && (((float)rockety/(10.0-(float)difficulty/4.0)-25.0+1.5) > steery-2) && object_properties[difficulty][1] == -2))
	{
		if((-(object_properties[difficulty][2]*3)+1.5 > steerx-(3-((float)abs(rollangle%180)/45.0)))&&(-(object_properties[difficulty][2]*3)-1.5 < steerx+(3-((float)abs(rollangle%180)/45.0))) && (hits < 9))
		{
			hits++;
			life_pickup = true;	// maximal ein Pickup pro Welle
		}
	}
}

// Schriftzuege Score, Hits und Level sowie dazugehoerige Zahlen
void overlay()
{
	glDisable(GL_LIGHTING);
	int temp_score = score;
	int temp_level = level;
	glColor3f(1,1,1);
	glRasterPos3f(-width,height-2.0,20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 83);  // S
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 99);  // c
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 111); // o
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 114); // r
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 58);  // :
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32);  // space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1000000)); // Millionenstelle
	temp_score%=1000000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/100000));  // Hunderttausenderstelle
	temp_score%=100000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/10000));   // Zehntausenderstelle
	temp_score%=10000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1000));    // Tausenderstelle
	temp_score%=1000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/100));     // Hunderterstelle
	temp_score%=100;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/10));      // Zehnerstelle
	temp_score%=10;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1));       // Einerstelle

	glColor3f(1,1,1);
	glRasterPos3f(-width,height-3.0,20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 72);  // H
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 105); // i
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 116); // t
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 115); // s
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 58);  // :
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32);  // space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+hits); //übrige Trefferpunkte (maximal 9)

	glColor3f(1,1,1);
	glRasterPos3f(-width, height-4.0, 20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 76);  // L
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 118); // v
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 108); // l
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 58);  // :
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32);  // space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_level/10)); // Zehnerstelle
	temp_level%=10;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_level/1));  // Einerstelle

	// Schwierigkeitsgrad Overlay
	glColor3f(1,1,1);
	glRasterPos3f(width-3, height-2, 20.0);
	if (gamemode == easy)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 97);  // a
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 115); // s
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 121); // y
	}
	else if (gamemode == medium)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 109); // m
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 100); // d
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 105); // i
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 117); // u
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 109); // m
	}
	else if (gamemode == hard)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 104); // h
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 97);  // a
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 114); // r
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 100); // d
	}
	else if (gamemode == korean)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 107); // k
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 111); // o
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 114); // r
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); // e
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 97);  // a
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 110); // n
	}
	glEnable(GL_LIGHTING);
}

// Ruecksetzung aller spielrelevanter Variablen auf Standardwerte
void new_game()
{
	switch(gamemode)
	{
	case easy:
		difficulty_scale = 0.75;
		difficulty = 1;
		prev_difficulty= 0;
		break;
	case hard:
		difficulty_scale = 1.1;
		difficulty = 3;
		prev_difficulty= 2;
		break;
	case korean:
		difficulty_scale = 1;
		difficulty = (int)width*2/3-1;
		prev_difficulty= (int)width*2/3-1;
		break;
	default:
		difficulty_scale = 1;
		difficulty = 1;
		prev_difficulty = 0;
	}
	rocket_angle = 0;
	rockety = 0;
	rollangle = 0;

	hits = START_HITS;
	score = 0;
	level = 1;
	level_index = 1;

	collide = false;
	gameover = false;
	change_diff = false;
	life_pickup = false;
	life_spawned = false;
	index1 = 0;
	index2 = 0;

	steerx = 0;
	steery = -15;
	
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);
	blight4 = false;
	blight5 = false;
	blight6 = false;

	// Neuer Sternenhintergrund
	sterne();

	int random;
	int temp;
	int random_rocket_spawn;
	// Zuruecksetzen der Nutzbaren Felder
	for(int i = 0; i < (int)width*2/3; i++) is_occupied[i] = 0;
	// Unbenutzbare Felder außerhalb des Spielbildschirms
	for(int i = (int)width*2/3; i < 30; i++) is_occupied[i] = 3;
	for(int i = 0; i < (int)width*2/3; i++)
	{
		do
		{
			random = rand();
			temp = (int)((double)random/(double)INT_MAX*(width*2.0/3.0));
		}
		while (is_occupied[temp] != 0); // jede Zufallsvariable maximal einmal vergeben
		random_rocket_spawn = temp-((int)width/3);
		random = rand()%2;
		if (level <= 3) random = 1; // Raketen in den Leveln 1-3 nur von oben 
		else if(level <= 6) random = 0; // Raketen in den Leveln 4-6 nur von unten
		if (random == 1) is_occupied[temp] = 1;
		else is_occupied[temp] = -1; 
		object_properties[i][1] = is_occupied[temp]; // Flugrichtung
		object_properties[i][2] = random_rocket_spawn; // Position
		random = rand()%5;
		life_spawned = false;
	}
}

// Hauptfunktion, stellt alle grafischen Inhalte dar
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0,0,0,1);
	light();
	overlay();
	glLoadIdentity();

	glTranslatef(0.0,height-2.0,20.0);
	if((level == 10) && (level_index > MAX_WAVE) && (!gameover)) // won Schriftzug wenn gewonnen
		won();
	glLoadIdentity();

	// Paused anzeigen wenn pausiert
	if (pause_btn)
	{
		pause_display();
	}
	
	// Umwandlung der Prozentualen Sternkoordinaten in Realkoordinaten
	for (int i = 0; i < 70; i++)
	{
		glTranslatef(width*rand_x[i]/100, height*rand_y[i]/100, -50);
		glDisable(GL_LIGHTING);
		glCallList(STAR);
		glEnable(GL_LIGHTING);
		glLoadIdentity();
	}	

	// Vektor mit Betrag 1, der xy-Ebene liegen muss
	double rv1[] = {0,-1,0};
	
	// Levelswitch
	switch (level)
	{
	case 1:
		glTranslatef(0.0,0.0,-40.0);
		createPlanet(5, bronze);	
		break;
	
	case 2:
		// Zusatzlicht in Level 2 aktivieren
		glEnable(GL_LIGHT5);
		blight5 = true;

		glTranslatef(0.0,0.0,-40.0);
		createPlanet(12, bronze);

		glTranslatef(-28.0,20.0,-10.0);
		createPlanet(5, green);
		break;
	
	case 3:
		// Licht aus Level 2 deaktivieren
		glDisable(GL_LIGHT5);
		blight5 = false;
		// Licht fuer Level 3 aktivieren
		glEnable(GL_LIGHT4);
		blight4 = true;

		glTranslatef(-5.0, 10.0, -30.0);
		createPlanet(8, blue);

		glTranslatef(width-3, -height, -25);
		createPlanet(13, blue);
		break;

	case 4:
		// Licht aus Level 3 deaktivieren
		glDisable(GL_LIGHT4);
		blight4 = false;
		// Licht fuer Level 4 aktivieren
		glEnable(GL_LIGHT6);
		blight6 = true;

		glTranslatef(width-5, height-5, -40);
		createPlanet(10, bronze);

		glTranslatef(0.0, -height*2/3, -30);
		createPlanet(4, red);

		glTranslatef(-width/2, 0, -30);
		createPlanet(6, bronze);
		break;
	
	case 5:
		// Lichter fuer Level 5 aktivieren
		glEnable(GL_LIGHT4);
		blight4 = true;
		glEnable(GL_LIGHT6);
		blight6 = true;
		
		glTranslatef(-20, -10, -40);
		createPlanet(9, bronze);

		glTranslatef(30, 5, -30);
		createPlanet(6 ,blue);
		break;

	case 6:
		// Licht aus Level 5 deaktivieren
		glDisable(GL_LIGHT6);
		blight6 = false;
		// Lichter fuer Level 6 aktivieren
		glEnable(GL_LIGHT4);
		blight4 = true;
		glEnable(GL_LIGHT5);
		blight5 = true;

		glTranslatef(-width+5, height-6, -30);
		createPlanet(6, blue);

		glTranslatef(-15, -height+3, -40);
		createPlanet(12, green);

		glTranslatef(width-3, height-3, -30);
		createPlanet(8, blue);
		break;

	case 7:
		// Licht aus Level 6 deaktivieren
		glDisable(GL_LIGHT4);
		blight4 = false;
		// Lichter fuer Level 7 aktivieren
		glEnable(GL_LIGHT5);
		blight5 = true;
		glEnable(GL_LIGHT6);
		blight6 = true;

		glTranslatef(-width+3, -height+3, -30);
		createPlanet(12, blue);

		
		calcmoonpos(12, 2, 3, rv1, -width+3, -height+3, -30, PHI1, false);
		createPlanet(2, white);

		glTranslatef(25, -10, -30);
		createPlanet(8, bronze);

		glTranslatef(35, height+2, -30);
		createPlanet(6, red);
		break;

	case 8:
		// Lichter fuer Level 8 aktivieren
		glEnable(GL_LIGHT4);
		blight4 = true;
		glEnable(GL_LIGHT5);
		blight5 = true;
		glEnable(GL_LIGHT6);
		blight6 = true;

		glTranslatef(-15, height+7, 20);
		createPlanet(15, red);

		glTranslatef(20,-15,-30);
		createPlanet(7, green);
		calcmoonpos(7, 2, 3, rv1, 20,-15,-30, PHI1, false);
		createPlanet(2, white);
		break;
	
	case 9:
		// Wechselnde Lichter durch change_light() in timer()
		glTranslatef(-25, 17, -30);
		createPlanet(5, bronze);

		glTranslatef(-15, 11, -30);
		createPlanet(4, red);

		glTranslatef(-18, -16, -30);
		createPlanet(5.5, white);

		glTranslatef(4,-4,-30);
		createPlanet(8, green);

		glTranslatef(width+4, height+2, -40);
		createPlanet(14, blue);
		break;
	
	default:
		// Lichter aus Level 9 deaktivieren
		glDisable(GL_LIGHT4);
		glDisable(GL_LIGHT5);
		glDisable(GL_LIGHT6);

		ALPHA = (M_PI/2)-(M_PI/40); //rot um x
		BETA = M_PI/12; //rot um y
		GAMMA = 0; //rot um z
		double sunvec[] = {1,1,0};
		calcmoonpos(10, 3, 7, sunvec, 0,0,-30, PHI2, true);
		createPlanet(3, sun);
		glEnable(GL_LIGHT2);

		glTranslatef(0,0,-30);
		createPlanet(10, green);

		ALPHA = M_PI/3; //rot um x
		BETA = M_PI/6; //rot um y
		GAMMA = M_PI/4; //rot um z
		calcmoonpos(10, 2, 3, rv1, 0,0,-30, PHI1, false);
		createPlanet(2, white);
		break;
	}
	
	// Darstellung der Objekte
	if (!gameover)
	{
		// Cruiser Darstellung
		glRotatef(180.0,0.0,1.0,0.0);
		glRotatef(-90.0,1.0,0.0,0.0);
		
		glTranslatef(steerx,0.0,steery);
		glRotatef(rollangle,0.0,0.0,1.0);

		glCallList(CRUISER);

		// Raketen Darstellung
		for(int i = 0; i < difficulty; i++)
		{
				spawn_rocket(i);	
		}

		// Extra-Leben Darstellung 
		if (life_spawned) spawn_life(difficulty);

		//Kollisionabfrage mit Rakete bzw. Extra-Leben
		if (!collide) collision_check_rocket();
		if (!life_pickup && life_spawned) pickup_life();
	}
	// Gameover Bildschirm
	else 
	{
		game_over();
	}
	
	glutSwapBuffers(); //double buffering	
}

//callback zur Hintergrundberechnung von Animationsvariablen und Zufallsgenerierung von Raketenspawnpunkten
void idle()
{
	if(!gameover)
	{
		rocket_angle++;
		rocket_angle %= 360;

		rockety++;

		// Wird gecallt wenn die Raketen den Rand erreichen (Welle vorbei)
		if (rockety >= (500-(int)(difficulty*12.5)))
		{
			rollangle = 0;
			score += (100*difficulty); // Punkte erhöhen
			if(level_index == MAX_WAVE && level < 10) // Level erhöhen
			{	
				level++;
				prev_difficulty = difficulty;
				if (level == 4 && gamemode!=korean)
				{
					prev_difficulty = prev_difficulty - (int)((double)width * difficulty_scale / 4.5);
				}
				else if (level == 7 && gamemode!=korean)
				{
					prev_difficulty = prev_difficulty - (int)((double)width * difficulty_scale / 4.5);
				}
				level_index = 0;
			}
			level_index++;

			if (level >= 10 && level_index >= 6)
			{
				level_index = 100;
			}

			if (change_diff && difficulty < (int)width*2/3 && gamemode!=korean)
			{
				change_diff = false;
				// Level Switch
				switch (level) // Abflachende difficulty in den Levelblöcken (1-3, 4-6, 7-9)
				{
				case 1:
					difficulty = prev_difficulty + (int)((double)width / (6.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 2:
					difficulty = prev_difficulty + (int)((double)width / (9.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);

				case 3:
					difficulty = prev_difficulty + (int)((double)width / (10.5f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 4:
					difficulty = prev_difficulty + (int)((double)width / (7.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 5:
					difficulty = prev_difficulty + (int)((double)width / (10.5f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 6:
					difficulty = prev_difficulty + (int)((double)width / (12.25f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 7:
					difficulty = prev_difficulty + (int)((double)width / (8.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 8:
					difficulty = prev_difficulty + (int)((double)width / (12.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				case 9:
					difficulty = prev_difficulty + (int)((double)width / (14.0f/difficulty_scale) * (double)level_index / (double) MAX_WAVE);
					break;
				default:
					difficulty++;
					break;
				}
				// Absicherung, dass weder zu viele, noch keine Raketen erzeugt
				if (difficulty > (int)width*2/3-1) difficulty = (int)width*2/3-1;
				else if (difficulty < 1) difficulty = 1;
			}
			
			collide = false;
			life_pickup = false;
			rockety = 0;
			int random;
			int temp;
			int random_rocket_spawn;
			// Zuruecksetzen der Nutzbaren Felder
			for(int i = 0; i < (int)width*2/3; i++) is_occupied[i] = 0;
			// Unbenutzbare Felder außerhalb des Spielbildschirms
			for(int i = (int)width*2/3; i < 30; i++) is_occupied[i] = 3;
			for(int i = 0; i < (int)width*2/3; i++)
			{
				do
				{
					random = rand();
					temp = (int)((double)random/(double)INT_MAX*(width*2.0/3.0));
				}
				while (is_occupied[temp] != 0); // jede Zufallsvariable maximal einmal vergeben
				random_rocket_spawn = temp-((int)width/3);
				random = rand()%2;
				if (level <= 3) random = 1; // Raketen in den Leveln 1-3 nur von oben 
				else if(level <= 6) random = 0; // Raketen in den Leveln 4-6 nur von unten
				if (random == 1) is_occupied[temp] = 1;
				else is_occupied[temp] = -1; 
				object_properties[i][1] = is_occupied[temp]; // Flugrichtung
				object_properties[i][2] = random_rocket_spawn; // Position
				
				// Zufallsgenerator fuer Leben
				random = rand()%10;
				switch (gamemode)
				{
				case easy:
					if (random <= 3 && level > 1) life_spawned = true;
					else life_spawned = false;
					break;
				case hard:
					if (random == 0 && level > 1) life_spawned = true;
					else life_spawned = false;
					break;
				default:
					if (random <= 1 && level > 1 && gamemode != korean) life_spawned = true;
					else life_spawned = false;
					break;
				}
			}
		}
	}
	glutPostRedisplay();
}

// Callback wenn das Fenster veraendert wird, stellt den Viewport und den Sichtkoerper neu ein
void reshape(int w, int h)
{
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Boolean, der den wechsel von einem Falschen zu einem richtigen Wert anzeigt
	bool width_false = false;
	if(!first_game_called && (width == 0 || width == 20)) width_false = true;

	//width und hight Neubestimmen	
	if (w <= h)
	{
		glOrtho(-20.0, 20.0, -20.0*(GLfloat)h/(GLfloat)w, 20.0*(GLfloat)h/(GLfloat)w, -50.0, 50.0); // Parallelprojektion
		width = 20.0;
		height = 20.0*(float)h/(float)w;
	}
	else  // Da im Vollbildmodus gespielt wird, sollte width groeßer als height sein, d.h. dieser Fall muss eintreten 
	{
		glOrtho(-20.0*(GLfloat)w/(GLfloat)h, 20.0*(GLfloat)w/(GLfloat)h, -20.0, 20.0, -50.0, 50.0); // Parallelprojektion
		width = 20.0*(float)w/(float)h;
		height = 20.0;
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Erster new_game() Aufruf muss hier stattfinden, da sonst Falsche Werte genutzt werden
	if (!first_game_called && width_false && width != 0 && width != 20)
	{
		first_game_called = true;
		new_game();
	}
}

// callback zur Abfrage von Mausfunktionen
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !pause_btn) rollangle-=15;
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && !pause_btn) rollangle+=15;
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN && !pause_btn) rollangle = 0;
	if (!pause_btn) glutPostRedisplay();

}

// callback zur Abfrage von Standardkeyboardtasten
void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) exit(0); // Escape zum Beenden
	if (key == 13 && !pause_btn) new_game(); // Return zum Neustart
	if (key == 112 && !gameover) toggle_pause(); //Pause Button

	// gamemode change
	if (key == 101 && gameover) toggle_gamemode('e');
	if (key == 109 && gameover) toggle_gamemode('m');
	if (key == 104 && gameover) toggle_gamemode('h');
	if (key == 107 && gameover) toggle_gamemode('k');

	// WASD
	if (key == 119 && steery < height-3 && !pause_btn && !gameover) steery++; 
	if (key == 115 && steery > -height+2 && !pause_btn && !gameover) steery--;
	if (key == 97 && steerx < width-(3-((float)abs(rollangle%180)/45.0))&& !pause_btn && !gameover) steerx++;
	if (key == 100 && steerx > -width+(3-((float)abs(rollangle%180)/45.0))&& !pause_btn && !gameover) steerx--;

	if (!pause_btn)
	{
		glutPostRedisplay();
	}

}

// callback zur Abfrage von speziellen Keyboardtasten
void special(int key, int x, int y)
{
	// Pfeiltasten
	if (key == GLUT_KEY_UP && steery < height-3 && !pause_btn && !gameover) steery++;
	if (key == GLUT_KEY_DOWN && steery > -height+2 && !pause_btn && !gameover) steery--;
	if (key == GLUT_KEY_LEFT && steerx < width-(3-((float)abs(rollangle%180)/45.0)) && !pause_btn && !gameover) steerx++;
	if (key == GLUT_KEY_RIGHT && steerx > -width+(3-((float)abs(rollangle%180)/45.0)) && !pause_btn && !gameover) steerx--;
	if (!pause_btn)
	{
		glutPostRedisplay();
	}
}

// timer callback
void timer(int x)
{
	glutTimerFunc(1000/60,timer,0); //0 from x++ //für konstatnte 60 FPS
	change_diff = true;

	// Waehrend das Spiel pausiert ist
	if (pause_btn)
	{
		glutIdleFunc(NULL);	
	}
	else if (!pause_btn)
	{
		glutIdleFunc(idle);
	}
	
	// Rotation der Monde um Planet
	if(level >= 7 && !pause_btn)
	{
		PHI1 += M_PI/350;
		if(PHI1 >= 2* M_PI)
			PHI1 -= 2* M_PI;
		
		PHI2 += M_PI/800;
		if(PHI2 >= 2* M_PI)
			PHI2 -= 2*M_PI;
	}

	// Kleines blinken zum Hit anzeigen
	if(collide && !pause_btn)
	{
		if(index1 < 15)
		{
			glDisable(GL_LIGHT0);
			glEnable(GL_LIGHT1);
		}
		if (index1 >= 15)
		{
			glEnable(GL_LIGHT0);
			glDisable(GL_LIGHT1);
		}

		index1++;
	}
	if(!collide)
	{
		index1 = 0;
		glDisable(GL_LIGHT1);
		glEnable(GL_LIGHT0);
	}

	// Kleines blinken zum Pickup anzeigen
	if(life_pickup && !pause_btn)
	{
		if(index2 < 15)
		{
			glDisable(GL_LIGHT0);
			glEnable(GL_LIGHT3);
		}
		if (index2 >= 15)
		{
			glEnable(GL_LIGHT0);
			glDisable(GL_LIGHT3);
		}

		index2++;
	}
	if(!life_pickup)
	{
		index2 = 0;
		glDisable(GL_LIGHT3);
		glEnable(GL_LIGHT0);
	}


	// Wechselnde Lichter in lvl 9
	if (level == 9 && !pause_btn)
	{
		glDisable(GL_LIGHT0);

		if (iterator == random_int)
		{
			change_light();
			iterator = 0;
		}
		iterator++;
	}
}

//Hilfsfunktion zur Initialisierung aller genutzter GL-Funktionen und Erstellung von
//Grafiklisten der genutzten Grafikmodelle
void init()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glColor3f(1.0, 1.0, 1.0);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	GLfloat plastik_ambient[] = { 0.0, 0.0, 0.0 };
	GLfloat plastik_diffuse[] = { 0.01, 0.01, 0.01 };
	GLfloat plastik_specular[] = { 0.5, 0.5, 0.5 };
	GLfloat plastik_shininess[] = { 32 };

	GLfloat kupfer_ambient[] = { 0.19, 0.07, 0.02 };
	GLfloat kupfer_diffuse[] = { 0.70, 0.27, 0.08 };
	GLfloat kupfer_specular[] = { 0.26, 0.14, 0.09 };
	GLfloat kupfer_shininess[] = { 13 };

	GLfloat silber_ambient[] = { 0.19, 0.19, 0.19 };
	GLfloat silber_diffuse[] = {0.58, 0.58, 0.58 };
	GLfloat silber_specular[] = { 0.58, 0.58, 0.58 };
	GLfloat silber_shininess[] = { 58 };

	GLfloat green_ambient[] = {0.0, 1.0, 0.0};
	GLfloat green_diffuse[] = {0.0, 1.0, 0.0};
	GLfloat green_specular[] = {0.0, 1.0, 0.0};
	GLfloat green_shininess[] = {50};

	// Raketen
	glShadeModel(GL_SMOOTH);

	glNewList(RAKETE,GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, silber_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, silber_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, silber_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, silber_shininess);

	  glTranslatef(0.0,0.0,-1.5);

	  gluCylinder(gluNewQuadric(),0.5,0.5,3.0,32,32); //K�rper

	  glMaterialfv(GL_FRONT, GL_AMBIENT, plastik_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, plastik_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, plastik_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, plastik_shininess);

	  glColor3f(0.0, 0.0, 0.0);
	  glBegin(GL_TRIANGLES); //Leitwerke
		glNormal3f(0.0,1.0,0.0);
	    glVertex3f(0.5,0.1,2.0); //rechts
	    glVertex3f(1.5,0.1,3.0);
	    glVertex3f(0.5,0.1,3.0);

		glNormal3f(0.0,-1.0,0.0);
		glVertex3f(0.5,-0.1,2.0);
	    glVertex3f(1.5,-0.1,3.0);
	    glVertex3f(0.5,-0.1,3.0);

		glNormal3f(0.0,1.0,0.0);
	    glVertex3f(-0.5,0.1,2.0); //links
	    glVertex3f(-1.5,0.1,3.0);
	    glVertex3f(-0.5,0.1,3.0);

		glNormal3f(0.0,-1.0,0.0);
	    glVertex3f(-0.5,-0.1,2.0);
	    glVertex3f(-1.5,-0.1,3.0);
	    glVertex3f(-0.5,-0.1,3.0);

		glNormal3f(1.0,0.0,0.0);
	    glVertex3f(0.1,0.5,2.0); //oben
	    glVertex3f(0.1,1.5,3.0);
	    glVertex3f(0.1,0.5,3.0);

		glNormal3f(-1.0,0.0,0.0);
	    glVertex3f(-0.1,0.5,2.0);
	    glVertex3f(-0.1,1.5,3.0);
	    glVertex3f(-0.1,0.5,3.0);

		glNormal3f(1.0,0.0,0.0);
	    glVertex3f(0.1,-0.5,2.0); //unten
	    glVertex3f(0.1,-1.5,3.0);
	    glVertex3f(0.1,-0.5,3.0);

		glNormal3f(-1.0,0.0,0.0);
	    glVertex3f(-0.1,-0.5,2.0);
	    glVertex3f(-0.1,-1.5,3.0);
	    glVertex3f(-0.1,-0.5,3.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP); //Leitwerksabdichtungen
	    glVertex3f(0.5,0.1,2.0);
		glVertex3f(0.5,-0.1,2.0);
	    glVertex3f(1.5,0.1,3.0);
		glVertex3f(1.5,-0.1,3.0);

	    glVertex3f(0.5,0.1,3.0);
		glVertex3f(0.5,-0.1,3.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glVertex3f(-0.5,0.1,2.0);
		glVertex3f(-0.5,-0.1,2.0);
	    glVertex3f(-1.5,0.1,3.0);
		glVertex3f(-1.5,-0.1,3.0);

	    glVertex3f(-0.5,0.1,3.0);
	    glVertex3f(-0.5,-0.1,3.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glVertex3f(0.1,0.5,2.0);
		glVertex3f(-0.1,0.5,2.0);
	    glVertex3f(0.1,1.5,3.0);
		glVertex3f(-0.1,1.5,3.0);

	    glVertex3f(0.1,0.5,3.0);
	    glVertex3f(-0.1,0.5,3.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glVertex3f(0.1,-0.5,2.0);
		glVertex3f(-0.1,-0.5,2.0);
	    glVertex3f(0.1,-1.5,3.0);
		glVertex3f(-0.1,-1.5,3.0);

	    glVertex3f(0.1,-0.5,3.0);
	    glVertex3f(-0.1,-0.5,3.0);
	  glEnd();
  
	  glRotatef(180,1.0,0.0,0.0);
	  glTranslatef(0.0,0.0,-3.5);

	  glutSolidCone(0.75,1.0,64,64); //Antrieb

	  glMaterialfv(GL_FRONT, GL_AMBIENT, kupfer_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, kupfer_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, kupfer_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, kupfer_shininess);

	  glTranslatef(0.0,0.0,3.5);

	  glutSolidCone(0.5,1.0,64,64); //Spitze
	  glTranslatef(0.0,0.0,1.5);
	  glRotatef(-180,1.0,0.0,0.0);
	glEndList();

	glNewList(STAR,GL_COMPILE);	  
	  glColor3f(1.0,1.0,1.0);
	  glutSolidSphere(0.1,64,64);
	glEndList();

	// Cruiser
	glNewList(CRUISER,GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, plastik_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, plastik_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, plastik_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, plastik_shininess);

	  glColor3f(1.0, 0.0, 0.0);
	  glBegin(GL_TRIANGLES);
	    glNormal3f(0.0,0.25,0.0);
		glVertex3f(3.0,0.1,-0.5); // Fluegelspitze
		glVertex3f(2.5,0.1,-0.5);
		glVertex3f(2.5,0.1,0.0);

		glNormal3f(0.0,-0.25,0.0);
		glVertex3f(3.0,-0.1,-0.5);
		glVertex3f(2.5,-0.1,-0.5);
		glVertex3f(2.5,-0.1,0.0);

		glNormal3f(0.0,0.25,0.0);
		glVertex3f(-3.0,0.1,-0.5);
		glVertex3f(-2.5,0.1,-0.5);
		glVertex3f(-2.5,0.1,0.0);

		glNormal3f(0.0,-0.25,0.0);
		glVertex3f(-3.0,-0.1,-0.5);
		glVertex3f(-2.5,-0.1,-0.5);
		glVertex3f(-2.5,-0.1,0.0);

		glNormal3f(0.0,0.25,0.0);
		glVertex3f(2.5,0.1,0.0); // Fluegelmitte
		glVertex3f(2.0,0.1,0.0);
		glVertex3f(2.0,0.1,0.5);

		glNormal3f(0.0,-0.25,0.0);
		glVertex3f(2.5,-0.1,0.0);
		glVertex3f(2.0,-0.1,0.0);
		glVertex3f(2.0,-0.1,0.5);

		glNormal3f(0.0,0.25,0.0);
		glVertex3f(-2.5,0.1,0.0);
		glVertex3f(-2.0,0.1,0.0);
		glVertex3f(-2.0,0.1,0.5);

		glNormal3f(0.0,-0.25,0.0);
		glVertex3f(-2.5,-0.1,0.0);
		glVertex3f(-2.0,-0.1,0.0);
		glVertex3f(-2.0,-0.1,0.5);

		glNormal3f(0.0,0.5,0.0);
		glVertex3f(2.0,0.1,0.5); // Vorfl�gel
		glVertex3f(1.0,0.1,0.5);
		glVertex3f(1.0,0.1,1.0);

		glNormal3f(0.0,-0.5,0.0);
		glVertex3f(2.0,-0.1,0.5);
		glVertex3f(1.0,-0.1,0.5);
		glVertex3f(1.0,-0.1,1.0);

		glNormal3f(0.0,0.5,0.0);
		glVertex3f(-2.0,0.1,0.5);
		glVertex3f(-1.0,0.1,0.5);
		glVertex3f(-1.0,0.1,1.0);

		glNormal3f(0.0,-0.5,0.0);
		glVertex3f(-2.0,-0.1,0.5);
		glVertex3f(-1.0,-0.1,0.5);
		glVertex3f(-1.0,-0.1,1.0);

		glNormal3f(0.0,0.5,0.0);
		glVertex3f(1.5,0.1,-2.0); // Schwanz
		glVertex3f(1.0,0.1,-2.0);
		glVertex3f(1.0,0.1,-1.0);

		glNormal3f(0.0,-0.5,0.0);
		glVertex3f(1.5,-0.1,-2.0);
		glVertex3f(1.0,-0.1,-2.0);
		glVertex3f(1.0,-0.1,-1.0);

		glNormal3f(0.0,0.5,0.0);
		glVertex3f(-1.5,0.1,-2.0);
		glVertex3f(-1.0,0.1,-2.0);
		glVertex3f(-1.0,0.1,-1.0);

		glNormal3f(0.0,-0.5,0.0);
		glVertex3f(-1.5,-0.1,-2.0);
		glVertex3f(-1.0,-0.1,-2.0);
		glVertex3f(-1.0,-0.1,-1.0);
 
		glNormal3f(1.2,-2.0,0.4);
		glVertex3f(1.0,-0.1,1.0); // Bug
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.1,3.0);

		glNormal3f(-1.2,-2.0,0.4);
		glVertex3f(-1.0,-0.1,1.0);
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.1,3.0);

		glNormal3f(0.0,0.0,-0.8);
		glVertex3f(1.0,-0.1,-1.0); // Rumpfabschluss unten
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(0.0,-0.5,-1.0);

		glNormal3f(0.0,0.0,-0.36);
		glVertex3f(1.0,0.1,-1.0); // Rumpfabschluss oben
		glVertex3f(0.1,0.1,-1.0);
		glVertex3f(0.1,0.5,-1.0);

		glNormal3f(0.0,0.0,-0.36);
		glVertex3f(-1.0,0.1,-1.0);
		glVertex3f(-0.1,0.1,-1.0);
		glVertex3f(-0.1,0.5,-1.0);

		glNormal3f(0.5,0.0,0.0);
		glVertex3f(0.1,0.5,-1.0); // Leitwerk
		glVertex3f(0.1,1.0,-1.0);
		glVertex3f(0.1,0.5,0.0);

		glNormal3f(-0.5,0.0,0.0);
		glVertex3f(-0.1,0.5,-1.0);
		glVertex3f(-0.1,1.0,-1.0);
		glVertex3f(-0.1,0.5,0.0);

		glNormal3f(0.075,0.25,0.0375);
		glVertex3f(1.0,0.1,1.0); // Bug oben Seite
		glVertex3f(0.75,0.1,1.5);
		glVertex3f(0.5,0.25,1.0);

		glNormal3f(-0.075,0.25,0.0375);
		glVertex3f(-1.0,0.1,1.0);
		glVertex3f(-0.75,0.1,1.5);
		glVertex3f(-0.5,0.25,1.0);

		glNormal3f(0.4,0.0,0.2);
		glVertex3f(0.75,0.5,-2.0); //Triebwerkbegrenzungen links und rechts
		glVertex3f(0.75,0.1,-2.0);
		glVertex3f(0.25,0.1,-1.0);

		glVertex3f(0.75,-0.5,-2.0);
		glVertex3f(0.75,-0.1,-2.0);
		glVertex3f(0.25,-0.1,-1.0);

		glNormal3f(-0.4,0.0,0.2);
		glVertex3f(-0.75,0.5,-2.0);
		glVertex3f(-0.75,0.1,-2.0);
		glVertex3f(-0.25,0.1,-1.0);

		glVertex3f(-0.75,-0.5,-2.0);
		glVertex3f(-0.75,-0.1,-2.0);
		glVertex3f(-0.25,-0.1,-1.0);

		glNormal3f(0.25,0.75,0.425);
		glVertex3f(0.75,0.5,-2.0); // Triebwerk oben
		glVertex3f(0.25,0.1,-1.0);
		glVertex3f(0.0,0.75,-2.0);

		glNormal3f(-0.25,0.75,0.425);
		glVertex3f(-0.75,0.5,-2.0);
		glVertex3f(-0.25,0.1,-1.0);
		glVertex3f(-0.0,0.75,-2.0);

		glNormal3f(0.0,0.5,0.325);
		glVertex3f(0.0,0.75,-2.0);
		glVertex3f(0.25,0.1,-1.0);
		glVertex3f(-0.25,0.1,-1.0);

		glNormal3f(0.25,-0.75,0.425);
		glVertex3f(0.75,-0.5,-2.0); // Triebwerk unten
		glVertex3f(0.25,-0.1,-1.0);
		glVertex3f(0.0,-0.75,-2.0);

		glNormal3f(-0.25,-0.75,0.425);
		glVertex3f(-0.75,-0.5,-2.0);
		glVertex3f(-0.25,-0.1,-1.0);
		glVertex3f(-0.0,-0.75,-2.0);

		glNormal3f(0.0,-0.5,0.325);
		glVertex3f(0.0,-0.75,-2.0);
		glVertex3f(0.25,-0.1,-1.0);
		glVertex3f(-0.25,-0.1,-1.0);

		glNormal3f(0.0,0.0,-0.375);
		glVertex3f(0.0,0.75,-2.0); // Triebwerk hinten
		glVertex3f(0.75,0.5,-2.0);
		glVertex3f(-0.75,0.5,-2.0);

		glVertex3f(0.0,-0.75,-2.0);
		glVertex3f(0.75,-0.5,-2.0);
		glVertex3f(-0.75,-0.5,-2.0);
	  glEnd();

	  glBegin(GL_QUADS);
	    glNormal3f(0.0,1.5,0.0);
	    glVertex3f(2.0,0.1,0.5); // Fluegel am Rumpf
		glVertex3f(1.0,0.1,0.5);
		glVertex3f(1.0,0.1,-1.0);
		glVertex3f(2.0,0.1,-1.0);

		glNormal3f(0.0,-1.5,0.0);
	    glVertex3f(2.0,-0.1,0.5);
		glVertex3f(1.0,-0.1,0.5);
		glVertex3f(1.0,-0.1,-1.0);
		glVertex3f(2.0,-0.1,-1.0);

		glNormal3f(0.0,1.5,0.0);
	    glVertex3f(-2.0,0.1,0.5);
		glVertex3f(-1.0,0.1,0.5);
		glVertex3f(-1.0,0.1,-1.0);
		glVertex3f(-2.0,0.1,-1.0);

		glNormal3f(0.0,-1.5,0.0);
	    glVertex3f(-2.0,-0.1,0.5);
		glVertex3f(-1.0,-0.1,0.5);
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(-2.0,-0.1,-1.0);

		glNormal3f(0.0,0.5,0.0);
	    glVertex3f(2.5,0.1,0.0); // Fluegelmitte
		glVertex3f(2.0,0.1,0.0);
		glVertex3f(2.0,0.1,-1.0);
		glVertex3f(2.5,0.1,-1.0);

		glNormal3f(0.0,-0.5,0.0);
	    glVertex3f(2.5,-0.1,0.0);
		glVertex3f(2.0,-0.1,0.0);
		glVertex3f(2.0,-0.1,-1.0);
		glVertex3f(2.5,-0.1,-1.0);

		glNormal3f(0.0,0.5,0.0);
	    glVertex3f(-2.5,0.1,0.0);
		glVertex3f(-2.0,0.1,0.0);
		glVertex3f(-2.0,0.1,-1.0);
		glVertex3f(-2.5,0.1,-1.0);

		glNormal3f(0.0,-0.5,0.0);
	    glVertex3f(-2.5,-0.1,0.0);
		glVertex3f(-2.0,-0.1,0.0);
		glVertex3f(-2.0,-0.1,-1.0);
		glVertex3f(-2.5,-0.1,-1.0);

		glNormal3f(0.0,0.25,0.0);
	    glVertex3f(3.0,0.1,-0.5); // Fluegel aussen
		glVertex3f(2.5,0.1,-0.5);
		glVertex3f(2.5,0.1,-1.0);
		glVertex3f(3.0,0.1,-1.0);

		glNormal3f(0.0,-0.25,0.0);
	    glVertex3f(3.0,-0.1,-0.5);
		glVertex3f(2.5,-0.1,-0.5);
		glVertex3f(2.5,-0.1,-1.0);
		glVertex3f(3.0,-0.1,-1.0);

		glNormal3f(0.0,0.25,0.0);
	    glVertex3f(-3.0,0.1,-0.5);
		glVertex3f(-2.5,0.1,-0.5);
		glVertex3f(-2.5,0.1,-1.0);
		glVertex3f(-3.0,0.1,-1.0);

		glNormal3f(0.0,-0.25,0.0);
	    glVertex3f(-3.0,-0.1,-0.5);
		glVertex3f(-2.5,-0.1,-0.5);
		glVertex3f(-2.5,-0.1,-1.0);
		glVertex3f(-3.0,-0.1,-1.0);

		glNormal3f(0.8,-2.0,0.0);
		glVertex3f(1.0,-0.1,1.0); // Rumpf unten
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.5,-1.0);
		glVertex3f(1.0,-0.1,-1.0);

		glNormal3f(-0.8,-2.0,0.0);
		glVertex3f(-1.0,-0.1,1.0);
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.5,-1.0);
		glVertex3f(-1.0,-0.1,-1.0);

		glNormal3f(0.0,0.75,0.0);
		glVertex3f(1.0,0.1,-2.0); // Schwanz links
		glVertex3f(1.0,0.1,-1.0);
		glVertex3f(0.25,0.1,-1.0);
		glVertex3f(0.75,0.1,-2.0);

		glNormal3f(0.0,-0.75,0.0);
		glVertex3f(1.0,-0.1,-2.0);
		glVertex3f(1.0,-0.1,-1.0);
		glVertex3f(0.25,-0.1,-1.0);
		glVertex3f(0.75,-0.1,-2.0);

		glNormal3f(0.0,0.75,0.0);
		glVertex3f(-1.0,0.1,-2.0); // Schwanz rechts
		glVertex3f(-1.0,0.1,-1.0);
		glVertex3f(-0.25,0.1,-1.0);
		glVertex3f(-0.75,0.1,-2.0);

		glNormal3f(0.0,-0.75,0.0);
		glVertex3f(-1.0,-0.1,-2.0);
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(-0.25,-0.1,-1.0);
		glVertex3f(-0.75,-0.1,-2.0);

		glNormal3f(0.4,0.9,0.0);
		glVertex3f(1.0,0.1,0.0); // Rumpf oben
		glVertex3f(0.1,0.5,0.0);
		glVertex3f(0.1,0.5,-1.0);
		glVertex3f(1.0,0.1,-1.0);

		glNormal3f(-0.4,0.9,0.0);
		glVertex3f(-1.0,0.1,0.0);
		glVertex3f(-0.1,0.5,0.0);
		glVertex3f(-0.1,0.5,-1.0);
		glVertex3f(-1.0,0.1,-1.0);

		glNormal3f(0.15,0.5,0.0);
		glVertex3f(1.0,0.1,0.0); // Rumpf oben Seiten
		glVertex3f(1.0,0.1,1.0);
		glVertex3f(0.5,0.25,1.0);
		glVertex3f(0.1,0.5,0.0);

		glNormal3f(-0.15,0.5,0.0);
		glVertex3f(-1.0,0.1,0.0);
		glVertex3f(-1.0,0.1,1.0);
		glVertex3f(-0.5,0.25,1.0);
		glVertex3f(-0.1,0.5,0.0);

		glNormal3f(0.225,0.75,0.1125);
		glVertex3f(0.75,0.1,1.5); // Bug oben
		glVertex3f(0.0,0.1,3.0);
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(0.5,0.25,1.0);

		glNormal3f(-0.225,0.75,0.1125);
		glVertex3f(-0.75,0.1,1.5);
		glVertex3f(0.0,0.1,3.0);
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(-0.5,0.25,1.0);

		glNormal3f(0.0,0.0,-1.5);
		glVertex3f(0.75,0.5,-2.0); // Triebwerk hinten Mitte
		glVertex3f(-0.75,0.5,-2.0);
		glVertex3f(-0.75,-0.5,-2.0);
		glVertex3f(0.75,-0.5,-2.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glNormal3f(0.0,-0.2,-0.1);
	    glVertex3f(0.1,0.5,0.0); // Leitwerk Abdichtung
		glVertex3f(-0.1,0.5,0.0);
		glVertex3f(-0.1,1.0,-1.0);
		glVertex3f(0.1,1.0,-1.0);

		glNormal3f(0.0,0.0,-0.18);
		glVertex3f(-0.1,0.1,-1.0);
		glVertex3f(0.1,0.1,-1.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glNormal3f(0.0,0.0,-0.15);
	    glVertex3f(0.75,0.1,-2.0); // Abdichtung rundherum
		glVertex3f(0.75,-0.1,-2.0);
		glVertex3f(1.5,-0.1,-2.0);
		glVertex3f(1.5,0.1,-2.0);

		glNormal3f(0.2,0.0,0.1);
		glVertex3f(1.0,-0.1,-1.0);
		glVertex3f(1.0,0.1,-1.0);

		glNormal3f(0.0,0.0,-0.4);
		glVertex3f(3.0,-0.1,-1.0);
		glVertex3f(3.0,0.1,-1.0);

		glNormal3f(0.1,0.0,0.0);
		glVertex3f(3.0,-0.1,-0.5);
		glVertex3f(3.0,0.1,-0.5);

		glNormal3f(0.1,0.0,0.1);
		glVertex3f(2.5,-0.1,0.0);
		glVertex3f(2.5,0.1,0.0);

		glNormal3f(0.1,0.0,0.1);
		glVertex3f(2.0,-0.1,0.5);
		glVertex3f(2.0,0.1,0.5);

		glNormal3f(0.1,0.0,0.2);
		glVertex3f(1.0,-0.1,1.0);
		glVertex3f(1.0,0.1,1.0);

		glNormal3f(0.4,0.0,0.2);
		glVertex3f(0.0,-0.1,3.0); // Mitte
		glVertex3f(0.0,0.1,3.0);

		glNormal3f(-0.4,0.0,0.2);
		glVertex3f(-1.0,-0.1,1.0);
		glVertex3f(-1.0,0.1,1.0);

		glNormal3f(-0.1,0.0,0.2);
		glVertex3f(-2.0,-0.1,0.5);
		glVertex3f(-2.0,0.1,0.5);

		glNormal3f(-0.1,0.0,0.1);
		glVertex3f(-2.5,-0.1,0.0);
		glVertex3f(-2.5,0.1,0.0);

		glNormal3f(-0.1,0.0,0.1);
		glVertex3f(-3.0,-0.1,-0.5);
		glVertex3f(-3.0,0.1,-0.5);

		glNormal3f(-0.1,0.0,0.0);
		glVertex3f(-3.0,-0.1,-1.0);
		glVertex3f(-3.0,0.1,-1.0);

		glNormal3f(0.0,0.0,-0.4);
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(-1.0,0.1,-1.0);

		glNormal3f(-0.2,0.0,0.1);
		glVertex3f(-1.5,-0.1,-2.0);
		glVertex3f(-1.5,0.1,-2.0);

		glNormal3f(0.0,0.0,-0.15);
		glVertex3f(-0.75,-0.1,-2.0);
		glVertex3f(-0.75,0.1,-2.0);
	  glEnd();

	  glColor3f(1.0, 1.0, 1.0);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, silber_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, silber_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, silber_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, silber_shininess);
	  glBegin(GL_TRIANGLES);
	    glNormal3f(0.25,0.9,0.125);
	    glVertex3f(0.1,0.5,0.0); // Scheibe rechts
		glVertex3f(0.5,0.25,1.0);
		glVertex3f(0.0,0.25,2.0);

		glNormal3f(-0.25,0.9,0.125);
		glVertex3f(-0.1,0.5,0.0); // Scheibe links
		glVertex3f(-0.5,0.25,1.0);
		glVertex3f(0.0,0.25,2.0);

		glNormal3f(0.0,0.4,0.05);
		glVertex3f(0.1,0.5,0.0); // Scheibe mitte
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(-0.1,0.5,0.0);
	  glEnd();
	glEndList();

	// Leben
	glNewList(LIFE, GL_COMPILE);
		glDisable(GL_LIGHTING);

		glMaterialfv(GL_FRONT, GL_AMBIENT, green_ambient);
	    glMaterialfv(GL_FRONT, GL_DIFFUSE, green_diffuse);
	    glMaterialfv(GL_FRONT, GL_SPECULAR, green_specular);
	    glMaterialfv(GL_FRONT, GL_SHININESS, green_shininess);
		
		//front green
		glBegin(GL_QUADS);
		glColor3f(0.0,1.0,0.0);
		glVertex3f(-1,0.33,0.5);
		glVertex3f(-1,-0.33,0.5);
		glVertex3f(1,-0.33,0.5);
		glVertex3f(1,0.33,0.5);
		//back green
		glColor3f(0.0,1.0,0.0);
		glVertex3f(1,-0.33,-0.5);
		glVertex3f(-1,-0.33,-0.5);
		glVertex3f(-1,0.33,-0.5);
		glVertex3f(1,0.33,-0.5);
		//right red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(1,0.33,0.5);
		glVertex3f(1,-0.33,0.5);
		glVertex3f(1,-0.33,-0.5);
		glVertex3f(1,0.33,-0.5);
		//left red 
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-1,0.33,-0.5);
		glVertex3f(-1,-0.33,-0.5);
		glVertex3f(-1,-0.33,0.5);
		glVertex3f(-1,0.33,0.5);
		//top red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-1,0.33,-0.5);
		glVertex3f(-1,0.33,0.5);
		glVertex3f(1,0.33,0.5);
		glVertex3f(1,0.33,-0.5);
		//bottom red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-1,-0.33,-0.5);
		glVertex3f(-1,-0.33,0.5);
		glVertex3f(1,-0.33,0.5);
		glVertex3f(1,-0.33,-0.5);

		glEnd();

		glBegin(GL_QUADS);
		// front green 
		glColor3f(0.0,1.0,0.0);
		glVertex3f(-0.33,1,0.5);
		glVertex3f(-0.33,-1,0.5);
		glVertex3f(0.33,-1,0.5);
		glVertex3f(0.33,1,0.5);
		//back green 
		glColor3f(0.0,1.0,0.0);
		glVertex3f(0.33,1,-0.5);
		glVertex3f(0.33,-1,-0.5);
		glVertex3f(-0.33,-1,-0.5);
		glVertex3f(-0.33,1,-0.5);
		//right red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(0.33,1,0.5);
		glVertex3f(0.33,-1,0.5);
		glVertex3f(0.33,-1,-0.5);
		glVertex3f(0.33,1,-0.5);
		//left red 
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-0.33,1,-0.5);
		glVertex3f(-0.33,-1,-0.5);
		glVertex3f(-0.33,-1,0.5);
		glVertex3f(-0.33,1,0.5);
		//top red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-0.33,1,-0.5);
		glVertex3f(-0.33,1,0.5);
		glVertex3f(0.33,1,0.5);
		glVertex3f(0.33,1,-0.5);
		//bottom red
		glColor3f(1.0,0.0,0.0);
		glVertex3f(-0.33,-1,-0.5);
		glVertex3f(-0.33,-1,0.5);
		glVertex3f(0.33,-1,0.5);
		glVertex3f(0.33,-1,-0.5);

		glEnd();
		
		glEnable(GL_LIGHTING);
	glEndList();
}

//main, registriert alle callbacks und geht in die Haupteventschleife �ber
int main(int argc, char** argv)
{
	// Terminaleingabe zur Bestimmung des Startgamemodes
	if(argc != 1)
	{
		char* Gamemode_str = argv[1];
		if (!strcmp(Gamemode_str, "-e")) gamemode = easy;
		else if (!strcmp(Gamemode_str, "-m")) gamemode = medium;
		else if (!strcmp(Gamemode_str, "-h")) gamemode = hard;
		else if (!strcmp(Gamemode_str, "-k")) gamemode = korean; 
	}
	// zufälliger Zeitabstand des Lichtwechsels in Level 9
	random_int = abs(rand() % 50) + 25;

	// glutMainLoop
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	glutFullScreen();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	
	glutTimerFunc(0,timer,0); // hier: Aufruf der Idle Function
	init();

	glutMainLoop();
	
	return(EXIT_SUCCESS);
}
