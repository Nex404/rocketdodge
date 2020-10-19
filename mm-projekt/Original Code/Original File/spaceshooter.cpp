//@Author Samuel Simeonov

#include <stdio.h>
#define _CRT_RAND_S
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

//Grafiklistennummerierung
#define RAKETE 1
#define PLANET 2
#define CRUISER 3

//Abmessungen des Koordinatensystems in Abh�ngigkeit zum Bildseitenverh�ltnis
float width = 0.0;
float height = 0.0;

//Variablen zur Animation
GLint winkel = 0;
GLint rocketx = 0;
GLint rocketz = 0;
GLint rollangle = 0;
GLint steerx = 0;
GLint steerz = -15;

//Variablenarrays f�r den Zufallsgenerator
bool is_occupied[30];
int random_spawn[30];

//Spielvariablen
int hits = 3;
int score = 0;
int difficulty = 1;

//Variablen zur Ablaufsteuerung
bool collide = false;
bool gameover = false;
bool change_diff = false;

//Lichtquellenkonfiguration
void light()
{
  GLfloat light0_position[] = { 10.0, 0.0, 10.0, 1.0 };
  GLfloat spot_direction[] = { -1.0, 0.0, -1.0 };

  GLfloat light1_position[] = { -10.0, 0.0, 10.0, 1.0 };
  GLfloat light1_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light1_diffuse[] = { 0.0, 0.0, 1.0, 1.0 };
  GLfloat light1_specular[] = { 0.0, 0.0, 1.0, 1.0 };
  GLfloat light1_spot_dir[] = { 1.0, 0.0, -1.0 };

  GLfloat light2_position[] = { 10.0, 0.0, -10.0, 1.0 };
  GLfloat light2_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light2_diffuse[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat light2_specular[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat light2_spot_dir[] = { -1.0, 0.0, 1.0 };

  GLfloat light3_position[] = { -10.0, 0.0, -10.0, 1.0 };
  GLfloat light3_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat light3_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat light3_specular[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat light3_spot_dir[] = { 1.0, 0.0, 1.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);// Spotlicht in wei� von unten rechts
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 25.0);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);// Spotlicht in blau von unten links
  glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
  glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0);
  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_spot_dir);

  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);// Spotlicht in gr�n von oben rechts
  glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
  glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 25.0);
  glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light2_spot_dir);

  glLightfv(GL_LIGHT3, GL_POSITION, light3_position);// Spotlicht in rot von oben links
  glLightfv(GL_LIGHT3, GL_AMBIENT, light3_ambient);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
  glLightfv(GL_LIGHT3, GL_SPECULAR, light3_specular);
  glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 25.0);
  glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light3_spot_dir);
}

//callback wenn das Fenster ver�ndert wird, stellt den Viewport und den Sichtk�rper neu ein
void reshape(int w, int h)
{
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
	{
		glOrtho(-20.0, 20.0, -20.0*(GLfloat)h/(GLfloat)w, 20.0*(GLfloat)h/(GLfloat)w, -50.0, 50.0);
		width = 20.0;
		height = 20.0*(float)h/(float)w;
	}
	else
	{
		glOrtho (-20.0*(GLfloat)w/(GLfloat)h, 20.0*(GLfloat)w/(GLfloat)h, -20.0, 20.0, -50.0, 50.0);
		width = 20.0*(float)w/(float)h;
		height = 20.0;
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//Hilfsfunktion zum prozeduralen erstellen mehrerer Raketen
void spawn_rocket(int x)
{
	glPushMatrix();
	  glLoadIdentity();
	  glRotatef(-90.0,1.0,0.0,0.0);
	  glPushMatrix();
	    glTranslatef(x,0.0,-(((GLfloat)rocketz)/(10.0-(float)difficulty/4.0)-25.0));
	    glRotatef(winkel,0.0,0.0,1.0);
	    glCallList(RAKETE);
	  glPopMatrix();
    glPopMatrix();
}

//GAME OVER Schriftzug
 void game_over()
{
	glRasterPos3f(-4.0,0.0,20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 71); //G
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 65); //A
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 77); //M
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 69); //E
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32); //space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 79); //O
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 86); //V
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 69); //E
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 82); //R
}

//Kollisionsabfrage via Bounding Box
void collision_check()
{
	if((-((float)rocketz/(10.0-(float)difficulty/4.0)-25.0-2.5) > steerz-2)&&(-((float)rocketz/(10.0-(float)difficulty/4.0)-25.0+2.5) < steerz+3))
	{
		for(int i = 0; i < difficulty; i++)
		{
			if((-(random_spawn[i]*3)+1.5 > steerx-(3-((float)abs(rollangle%180)/45.0)))&&(-(random_spawn[i]*3)-1.5 < steerx+(3-((float)abs(rollangle%180)/45.0))))
			{
				//printf("Kollision z: %f > %i && %f < %i\nKollision x: %f > %f && %f < %f\n",-((float)rocketz/10.0-25.0-2.5),steerz-2,-((float)rocketz/10.0-25.0+2.5),steerz+3,-(random_spawn[i]*3)+1.5,steerx-(3-((float)abs(rollangle%180)/45.0)),-(random_spawn[i]*3)-1.5,steerx+(3-((float)abs(rollangle%180)/45.0)));
				hits--;
				if (hits == 0) gameover = true;
				collide = true; //maximal eine Kollision pro Welle
				break; //sobald erste Kollision gefunden abbruch
			}
		}
	}
}

//Schriftzug Score, Hits sowie dazugehoerige Zahl
void overlay()
{
	int temp_score = score;
	glColor3f(1,1,1);
	glRasterPos3f(-width,height-2.0,20.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 83); //S
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 99); //c
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 111); //o
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 114); //r
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 101); //e
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 58); //:
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32); //space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1000000)); //Millionenstelle
	temp_score%=1000000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/100000)); //Hunderttausenderstelle
	temp_score%=100000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/10000)); //Zehntausenderstelle
	temp_score%=10000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1000)); //Tausenderstelle
	temp_score%=1000;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/100)); //Hunderterstelle
	temp_score%=100;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/10)); //Zehnerstelle
	temp_score%=10;
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+(temp_score/1)); //Einerstelle

	glRasterPos3f(width-5.0,height-2.0,10.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 72); //H
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 105); //i
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 116); //t
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 115); //s
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 58); //:
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 32); //space
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 48+hits); //uebrige Trefferpunkte
}

//R�cksetzung aller spielrelevanter Variablen auf Standardwerte
void new_game()
{
	winkel = 0;
	rocketx = 0;
	rocketz = 0;
	rollangle = 0;

	hits = 3;
	score = 0;
	difficulty = 1;

	collide = false;
	gameover = false;

	steerx = 0;
	steerz = -15;

	unsigned int random;
	int temp;
	for(int i = 0; i < (int)width*2/3; i++) is_occupied[i] = false;
	for(int i = 0; i < (int)width*2/3-1; i++)
	{
		do
		{
			rand_s(&random);
			temp = (int)((double)random/(double)UINT_MAX*(width*2.0/3.0));
		}
		while (is_occupied[temp] == true);
		random_spawn[i] = temp-((int)width/3);
		is_occupied[temp] = true;
	}
}

//Hauptfunktion, stellt alle grafischen Inhalte dar
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	light();
	overlay();
	glTranslatef(0.0,0.0,-10.0);
	glCallList(PLANET);
	glLoadIdentity();
	if (!gameover)
	{
		glRotatef(180.0,0.0,1.0,0.0);
		glRotatef(-90.0,1.0,0.0,0.0);
	
		glTranslatef(steerx,0.0,steerz);
		glRotatef(rollangle,0.0,0.0,1.0);

		glCallList(CRUISER);

		for(int i = 0; i < difficulty; i++) spawn_rocket(random_spawn[i]*3);
	
		if (!collide) collision_check();
	}
	else game_over();
	glutSwapBuffers(); //double buffering
}

//callback zur Hintergrundberechnung von Animationsvariablen und Zufallsgenerierung von Raketenspawnpunkten
void idle()
{
	if(!gameover)
	{
		winkel++;
		winkel %= 360;
		rocketz++;
		if (rocketz == (500-(int)(difficulty*12.5)))
		{
			rollangle = 0;
			score += (100*difficulty);
			if (change_diff && difficulty < (int)width*2/3-1)
			{
				change_diff = false;
				difficulty++;
			}
			collide = false;
			rocketz %= (500-(int)(difficulty*12.5));
			unsigned int random;
			int temp;
			for(int i = 0; i < (int)width*2/3; i++) is_occupied[i] = false;
			for(int i = 0; i < (int)width*2/3-1; i++)
			{
				do
				{
					rand_s(&random);
					temp = (int)((double)random/(double)UINT_MAX*(width*2.0/3.0));
				}
				while (is_occupied[temp] == true); //jede Zufallsvariable maximal einmal vergeben
				random_spawn[i] = temp-((int)width/3);
				is_occupied[temp] = true;
			}
		}
	}
	glutPostRedisplay();
}

//callback zur Abfrage von Mausfunktionen
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) rollangle-=15;
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) rollangle+=15;
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) rollangle = 0;
	glutPostRedisplay();
}

//callback zur Abfrage von Standardkeyboardtasten
void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) exit(0); //Escape zum Beenden
	if (key == 13) new_game(); //Return zum Neustart
}

//callback zur Abfrage von speziellen Keyboardtasten
void special(int key, int x, int y)
{
	if (key == GLUT_KEY_UP && steerz < height-3) steerz++;
	if (key == GLUT_KEY_DOWN && steerz > -height+2) steerz--;
	if (key == GLUT_KEY_LEFT && steerx < width-(3-((float)abs(rollangle%180)/45.0))) steerx++;
	if (key == GLUT_KEY_RIGHT && steerx > -width+(3-((float)abs(rollangle%180)/45.0))) steerx--;
	glutPostRedisplay();
}

//timer callback
void timer(int x)
{
	glutTimerFunc(10000,timer,x++);
	change_diff = true;
}

//Hilfsfunktion zur Initialisierung aller genutzter GL-Funktionen und Erstellung von
//Grafiklisten der genutzten Grafikmodelle
void init()
{
	glClearColor(0.2, 0.2, 0.2, 0.0);

	glColor3f(1.0, 1.0, 1.0);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	GLfloat plastik_ambient[] = { 0.0, 0.0, 0.0 };
	GLfloat plastik_diffuse[] = { 0.01, 0.01, 0.01 };
	GLfloat plastik_specular[] = { 0.5, 0.5, 0.5 };
	GLfloat plastik_shininess[] = { 32 };

	GLfloat bronze_ambient[] = { 0.21, 0.13, 0.05 };
	GLfloat bronze_diffuse[] = { 0.71, 0.43, 0.18 };
	GLfloat bronze_specular[] = { 0.39, 0.27, 0.17 };
	GLfloat bronze_shininess[] = { 26 };

	GLfloat kupfer_ambient[] = { 0.19, 0.07, 0.02 };
	GLfloat kupfer_diffuse[] = { 0.70, 0.27, 0.08 };
	GLfloat kupfer_specular[] = { 0.26, 0.14, 0.09 };
	GLfloat kupfer_shininess[] = { 13 };

	GLfloat silber_ambient[] = { 0.19, 0.19, 0.19 };
	GLfloat silber_diffuse[] = { 0.51, 0.51, 0.51 };
	GLfloat silber_specular[] = { 0.51, 0.51, 0.51 };
	GLfloat silber_shininess[] = { 51 };

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);

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

	glNewList(PLANET,GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, bronze_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, bronze_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, bronze_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, bronze_shininess);
	  glutSolidSphere(8.0,64,64);
	glEndList();

	glNewList(CRUISER,GL_COMPILE);
	  glMaterialfv(GL_FRONT, GL_AMBIENT, plastik_ambient);
	  glMaterialfv(GL_FRONT, GL_DIFFUSE, plastik_diffuse);
	  glMaterialfv(GL_FRONT, GL_SPECULAR, plastik_specular);
	  glMaterialfv(GL_FRONT, GL_SHININESS, plastik_shininess);

	  glColor3f(0.0, 0.0, 0.0);
	  glBegin(GL_TRIANGLES);
	    glNormal3f(0.0,0.25,0.0);
		glVertex3f(3.0,0.1,-0.5); //Fl�gelspitze
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
		glVertex3f(2.5,0.1,0.0); //Fl�gelmitte
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
		glVertex3f(2.0,0.1,0.5); //Vorfl�gel
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
		glVertex3f(1.5,0.1,-2.0); //Schwanz
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
		glVertex3f(1.0,-0.1,1.0); //Bug
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.1,3.0);

		glNormal3f(-1.2,-2.0,0.4);
		glVertex3f(-1.0,-0.1,1.0);
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.1,3.0);

		glNormal3f(0.0,0.0,-0.8);
		glVertex3f(1.0,-0.1,-1.0); //Rumpfabschluss unten
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(0.0,-0.5,-1.0);

		glNormal3f(0.0,0.0,-0.36);
		glVertex3f(1.0,0.1,-1.0); //Rumpfabschluss oben
		glVertex3f(0.1,0.1,-1.0);
		glVertex3f(0.1,0.5,-1.0);

		glNormal3f(0.0,0.0,-0.36);
		glVertex3f(-1.0,0.1,-1.0);
		glVertex3f(-0.1,0.1,-1.0);
		glVertex3f(-0.1,0.5,-1.0);

		glNormal3f(0.5,0.0,0.0);
		glVertex3f(0.1,0.5,-1.0); //Leitwerk
		glVertex3f(0.1,1.0,-1.0);
		glVertex3f(0.1,0.5,0.0);

		glNormal3f(-0.5,0.0,0.0);
		glVertex3f(-0.1,0.5,-1.0);
		glVertex3f(-0.1,1.0,-1.0);
		glVertex3f(-0.1,0.5,0.0);

		glNormal3f(0.075,0.25,0.0375);
		glVertex3f(1.0,0.1,1.0); //Bug oben Seite
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
		glVertex3f(0.75,0.5,-2.0); //Triebwerk oben
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
		glVertex3f(0.75,-0.5,-2.0); //Triebwerk unten
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
		glVertex3f(0.0,0.75,-2.0); //Triebwerk hinten
		glVertex3f(0.75,0.5,-2.0);
		glVertex3f(-0.75,0.5,-2.0);

		glVertex3f(0.0,-0.75,-2.0);
		glVertex3f(0.75,-0.5,-2.0);
		glVertex3f(-0.75,-0.5,-2.0);
	  glEnd();

	  glBegin(GL_QUADS);
	    glNormal3f(0.0,1.5,0.0);
	    glVertex3f(2.0,0.1,0.5); //Fl�gel am Rumpf
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
	    glVertex3f(2.5,0.1,0.0); //Fl�gelmitte
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
	    glVertex3f(3.0,0.1,-0.5); //Fl�gel au�en
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
		glVertex3f(1.0,-0.1,1.0); //Rumpf unten
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.5,-1.0);
		glVertex3f(1.0,-0.1,-1.0);

		glNormal3f(-0.8,-2.0,0.0);
		glVertex3f(-1.0,-0.1,1.0);
		glVertex3f(0.0,-0.5,1.0);
		glVertex3f(0.0,-0.5,-1.0);
		glVertex3f(-1.0,-0.1,-1.0);

		glNormal3f(0.0,0.75,0.0);
		glVertex3f(1.0,0.1,-2.0); //Schwanz links
		glVertex3f(1.0,0.1,-1.0);
		glVertex3f(0.25,0.1,-1.0);
		glVertex3f(0.75,0.1,-2.0);

		glNormal3f(0.0,-0.75,0.0);
		glVertex3f(1.0,-0.1,-2.0);
		glVertex3f(1.0,-0.1,-1.0);
		glVertex3f(0.25,-0.1,-1.0);
		glVertex3f(0.75,-0.1,-2.0);

		glNormal3f(0.0,0.75,0.0);
		glVertex3f(-1.0,0.1,-2.0); //Schwanz rechts
		glVertex3f(-1.0,0.1,-1.0);
		glVertex3f(-0.25,0.1,-1.0);
		glVertex3f(-0.75,0.1,-2.0);

		glNormal3f(0.0,-0.75,0.0);
		glVertex3f(-1.0,-0.1,-2.0);
		glVertex3f(-1.0,-0.1,-1.0);
		glVertex3f(-0.25,-0.1,-1.0);
		glVertex3f(-0.75,-0.1,-2.0);

		glNormal3f(0.4,0.9,0.0);
		glVertex3f(1.0,0.1,0.0); //Rumpf oben
		glVertex3f(0.1,0.5,0.0);
		glVertex3f(0.1,0.5,-1.0);
		glVertex3f(1.0,0.1,-1.0);

		glNormal3f(-0.4,0.9,0.0);
		glVertex3f(-1.0,0.1,0.0);
		glVertex3f(-0.1,0.5,0.0);
		glVertex3f(-0.1,0.5,-1.0);
		glVertex3f(-1.0,0.1,-1.0);

		glNormal3f(0.15,0.5,0.0);
		glVertex3f(1.0,0.1,0.0); //Rumpf oben Seiten
		glVertex3f(1.0,0.1,1.0);
		glVertex3f(0.5,0.25,1.0);
		glVertex3f(0.1,0.5,0.0);

		glNormal3f(-0.15,0.5,0.0);
		glVertex3f(-1.0,0.1,0.0);
		glVertex3f(-1.0,0.1,1.0);
		glVertex3f(-0.5,0.25,1.0);
		glVertex3f(-0.1,0.5,0.0);

		glNormal3f(0.225,0.75,0.1125);
		glVertex3f(0.75,0.1,1.5); //Bug oben
		glVertex3f(0.0,0.1,3.0);
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(0.5,0.25,1.0);

		glNormal3f(-0.225,0.75,0.1125);
		glVertex3f(-0.75,0.1,1.5);
		glVertex3f(0.0,0.1,3.0);
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(-0.5,0.25,1.0);

		glNormal3f(0.0,0.0,-1.5);
		glVertex3f(0.75,0.5,-2.0); //Triebwerk hinten Mitte
		glVertex3f(-0.75,0.5,-2.0);
		glVertex3f(-0.75,-0.5,-2.0);
		glVertex3f(0.75,-0.5,-2.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glNormal3f(0.0,-0.2,-0.1);
	    glVertex3f(0.1,0.5,0.0); //Leitwerk Abdichtung
		glVertex3f(-0.1,0.5,0.0);
		glVertex3f(-0.1,1.0,-1.0);
		glVertex3f(0.1,1.0,-1.0);

		glNormal3f(0.0,0.0,-0.18);
		glVertex3f(-0.1,0.1,-1.0);
		glVertex3f(0.1,0.1,-1.0);
	  glEnd();

	  glBegin(GL_QUAD_STRIP);
	    glNormal3f(0.0,0.0,-0.15);
	    glVertex3f(0.75,0.1,-2.0); //Abdichtung rundherum
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
		glVertex3f(0.0,-0.1,3.0); //Mitte
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
	    glVertex3f(0.1,0.5,0.0); //Scheibe rechts
		glVertex3f(0.5,0.25,1.0);
		glVertex3f(0.0,0.25,2.0);

		glNormal3f(-0.25,0.9,0.125);
		glVertex3f(-0.1,0.5,0.0); //Scheibe links
		glVertex3f(-0.5,0.25,1.0);
		glVertex3f(0.0,0.25,2.0);

		glNormal3f(0.0,0.4,0.05);
		glVertex3f(0.1,0.5,0.0); //Scheibe mitte
		glVertex3f(0.0,0.25,2.0);
		glVertex3f(-0.1,0.5,0.0);
	  glEnd();
	glEndList();
}

//main, registriert alle callbacks und geht in die Haupteventschleife �ber
int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	glutFullScreen();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutTimerFunc(10000,timer,0);
	init();
	new_game();
	glutMainLoop();
	return(0);
}