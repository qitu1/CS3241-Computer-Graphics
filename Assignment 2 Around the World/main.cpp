// CS3241Lab1.cpp : Defines the entry point for the console application.
#include <cmath>
#include <iostream>
#include <time.h>
#include <iostream>
#include <thread> // Required for std::this_thread::sleep_for
#include <chrono> // Required for std::chrono::milliseconds
#define GL_SILENCE_DEPRECATION

#ifdef _WIN32
#include <Windows.h>
#include "GL/glut.h"
#define M_PI 3.141592654
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>
#endif

using namespace std;

#define numStars 200
#define numPlanets 9

class planet
{
public:
	float distFromRef;
	float angularSpeed;
	GLfloat color[3];
	float size;
	float angle;
	float alpha;

	planet()
	{
		distFromRef = 0;
		angularSpeed = 0;
		color[0] = color[1] = color[2] = 0;
		size = 0;
		angle = 0;
		alpha = 1.0;
	}
};

GLfloat PI = 3.14;
float alpha = 0.0, k = 1;
float tx = 0.0, ty = 0.0;
planet planetList[numPlanets];

bool clockMode = false;

time_t seconds = 0;
struct tm* timeinfo;
float timer = 0.1; // this is a dumb way to control the speed of rotations

float starRotation = 0.0f;
float starBrightness = 0.0f;
float starPositions[numStars][2];
bool starsInitialized = false;

float idaAngle = 0.0f;    
float idaSize = 0.26f;     
float walkCycle = 0.0f; 
bool legForward = true;

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10, 10, -10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.3, 1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// draw a disk (planet)
void drawDisk(float radius, float r, float g, float b, float alpha, bool shaded = true)
{
	int circle_points = 50;
	glBegin(GL_POLYGON);
	for (int i = 0; i < circle_points; i++)
	{
		float angle = 2 * PI * i / circle_points;
		if (shaded && i > circle_points / 2)
			glColor4f(r, g, b, alpha);
		else if (shaded)
			glColor4f(r - 0.2f, g - 0.2f, b - 0.2f, alpha);
		else
			glColor4f(r, g, b, alpha);
		glVertex2f(radius * cos(angle), radius * sin(angle));
	}
	glEnd();
}


// draw planet at elliptical position
void drawPlanetElliptical(int planetIndex, float a, float b, float angleOffset = 0)
{
	glPushMatrix();
	float currentAngle = planetList[planetIndex].angle + angleOffset;
	float x = a * cos(currentAngle);
	float y = b * sin(currentAngle);
	glTranslatef(x, y, 0);
	drawDisk(planetList[planetIndex].size,
		planetList[planetIndex].color[0],
		planetList[planetIndex].color[1],
		planetList[planetIndex].color[2],
		planetList[planetIndex].alpha);
	glPopMatrix();
}

//draw planet at circular position
void drawPlanetCircular(int planetIndex, float angleOffset = 0)
{
	glPushMatrix();
	glRotatef(planetList[planetIndex].angle + angleOffset, 0, 0, 1);
	glTranslatef(0, planetList[planetIndex].distFromRef, 0);
	drawDisk(planetList[planetIndex].size,
		planetList[planetIndex].color[0],
		planetList[planetIndex].color[1],
		planetList[planetIndex].color[2],
		planetList[planetIndex].alpha);
	glPopMatrix();
}

void drawGlowingSun(float radius, float r, float g, float b, float alpha)
{
	int circle_points = 50;

	// glow layers
	for (int layer = 5; layer >= 1; layer--)
	{
		float glowRadius = radius * (1.0f + layer * 0.12f);
		float glowAlpha = alpha * 0.5f / layer;

		glBegin(GL_POLYGON);
		glColor4f(r, g * 0.7, b * 0.3, glowAlpha);
		for (int i = 0; i < circle_points; i++)
		{
			float angle = 2 * PI * i / circle_points;
			glVertex2f(glowRadius * cos(angle), glowRadius * sin(angle));
		}
		glEnd();
	}

	glBegin(GL_POLYGON);
	glColor4f(r, g, b, alpha);
	for (int i = 0; i < circle_points; i++)
	{
		float angle = 2 * PI * i / circle_points;
		glVertex2f(radius * cos(angle), radius * sin(angle));
	}
	glEnd();
}

// draw Ida (the character walking on Venus)
void drawIda(float walkCycle)
{
	glPushMatrix();

	// body
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 1.0f); 
	glVertex2f(0.0f, idaSize * 1.5f);     
	glVertex2f(-idaSize, 0.0f);           
	glVertex2f(idaSize, 0.0f);           
	glEnd();

	glBegin(GL_POLYGON);
	for (int i = 0; i <= 30; i++) {
		float angle = 2 * PI * i / 30.0f; 
		float x = idaSize * cos(angle);        
		float y = -idaSize * 0.4f * sin(angle); 
		glVertex2f(x, y);
	}
	glEnd();

	// head
	glTranslatef(0.0f, idaSize * 1.8f, 0.0f);
	glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 30; i++) {
		float angle = 2 * PI * i / 30;
		glVertex2f(idaSize * 0.5f * cos(angle),
			idaSize * 0.5f * sin(angle));
	}
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1.0f, 1.0f, 1.0f); 
	for (int i = 0; i < 30; i++) {
		float angle = 2 * PI * i / 30;
		float x = idaSize * 0.32f * cos(angle) - idaSize * 0.15f; 
		float y = idaSize * 0.4f * sin(angle);                  
		glVertex2f(x, y);
	}
	glEnd();

	// hat
	glPushMatrix();
	glTranslatef(idaSize * 0.3f, idaSize * 0.15f, 0.0f);
	glRotatef(-70.0f, 0, 0, 1); 
	glBegin(GL_TRIANGLES);
	glVertex2f(0.0f, idaSize * 1.6f);   
	glVertex2f(-idaSize * 0.4f, 0.0f);   
	glVertex2f(idaSize * 0.4f, 0.0f);    
	glEnd();
	glPopMatrix();

	glPopMatrix(); 

	// left leg
	glPushMatrix();
	glTranslatef(-idaSize * 0.15f, 0.0f, 0.0f); 
	glRotatef(walkCycle, 0, 0, 1);
	glBegin(GL_QUADS); 
	glVertex2f(-idaSize * 0.08f, 0.0f);
	glVertex2f(idaSize * 0.08f, 0.0f);
	glVertex2f(idaSize * 0.08f, -idaSize * 1.3f);
	glVertex2f(-idaSize * 0.08f, -idaSize * 1.3f);
	glEnd();
	glPopMatrix();

	// right leg
	glPushMatrix();
	glTranslatef(idaSize * 0.15f, 0.0f, 0.0f);
	glRotatef(-walkCycle, 0, 0, 1);
	glBegin(GL_QUADS);
	glVertex2f(-idaSize * 0.08f, 0.0f);
	glVertex2f(idaSize * 0.08f, 0.0f);
	glVertex2f(idaSize * 0.08f, -idaSize * 1.3f);
	glVertex2f(-idaSize * 0.08f, -idaSize * 1.3f);
	glEnd();
	glPopMatrix();

	glPopMatrix();
}

void drawStars()
{
	if (!starsInitialized)
	{
		srand(42);
		for (int i = 0; i < numStars; i++)
		{
			starPositions[i][0] = -10 + (rand() % 200) / 10.0f;
			starPositions[i][1] = -10 + (rand() % 200) / 10.0f;
		}
		starsInitialized = true;
	}

	glPushMatrix();
	glRotatef(starRotation, 0, 0, 1);

	glPointSize(2.0);
	glBegin(GL_POINTS);

	for (int i = 0; i < numStars; i++)
	{
		float brightness = 0.5f + 0.5f * sin(starBrightness + i * 0.1f);
		glColor4f(brightness, brightness, brightness, 0.8f * brightness);
		glVertex2f(starPositions[i][0], starPositions[i][1]);
	}
	glEnd();
	glPopMatrix();
}

void generatePlanets()
{
	//The Sun
	planetList[0].distFromRef = 0;
	planetList[0].angularSpeed = 0;
	planetList[0].color[0] = 1.0;
	planetList[0].color[1] = 0.78;
	planetList[0].color[2] = 0.05;
	planetList[0].size = 1.0;
	planetList[0].angle = 150;

	//Mercury (circular orbit)
	planetList[1].distFromRef = 2.0;
	planetList[1].angularSpeed = 8.8;
	planetList[1].color[0] = 0.7;
	planetList[1].color[1] = 0.7;
	planetList[1].color[2] = 0.7;
	planetList[1].size = 0.3;
	planetList[1].angle = 90;

	//Venus (circular orbit)
	planetList[2].distFromRef = 3.0;
	planetList[2].angularSpeed = 3.5;
	planetList[2].color[0] = 1.0;
	planetList[2].color[1] = 0.5;
	planetList[2].color[2] = 0.15;
	planetList[2].size = 0.4;
	planetList[2].angle = -50;

	//Earth (circular orbit)
	planetList[3].distFromRef = 4.5;
	planetList[3].angularSpeed = 18.0;
	planetList[3].color[0] = 0.0;
	planetList[3].color[1] = 0.5;
	planetList[3].color[2] = 1.0;
	planetList[3].size = 0.4;

	//Mars (elliptical orbit)
	planetList[4].distFromRef = 6.0; // semi-major axis
	planetList[4].angularSpeed = 60.0;
	planetList[4].color[0] = 1.0;
	planetList[4].color[1] = 0.3;
	planetList[4].color[2] = 0.0;
	planetList[4].size = 0.35;

	//Jupiter (elliptical orbit)
	planetList[5].distFromRef = 7.8; // semi-major axis
	planetList[5].angularSpeed = 0.8;
	planetList[5].color[0] = 0.8;
	planetList[5].color[1] = 0.5;
	planetList[5].color[2] = 0.2;
	planetList[5].size = 0.7;

	//Saturn (circular orbit)
	planetList[6].distFromRef = 9.2;
	planetList[6].angularSpeed = 2.0;
	planetList[6].color[0] = 0.9;
	planetList[6].color[1] = 0.8;
	planetList[6].color[2] = 0.5;
	planetList[6].size = 0.6;

	//Moon (orbiting Earth)
	planetList[7].distFromRef = 0.8;
	planetList[7].angularSpeed = 60.0;
	planetList[7].color[0] = 0.8;
	planetList[7].color[1] = 0.8;
	planetList[7].color[2] = 0.8;
	planetList[7].size = 0.15;

	//Satellite (orbiting Mars)
	planetList[8].distFromRef = 0.5;
	planetList[8].angularSpeed = 200.0;
	planetList[8].color[0] = 0.6;
	planetList[8].color[1] = 0.6;
	planetList[8].color[2] = 0.6;
	planetList[8].size = 0.1;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//controls transformation
	glScalef(k, k, k);
	glTranslatef(tx, ty, 0);
	glRotatef(alpha, 0, 0, 1);

	// Draw star background
	drawStars();

	// Draw the Sun with glow effect
	glPushMatrix();
	drawGlowingSun(planetList[0].size,
		planetList[0].color[0],
		planetList[0].color[1],
		planetList[0].color[2],
		planetList[0].alpha);
	glPopMatrix();

	// Draw planets
	// Mercury (circular)
	drawPlanetCircular(1);

	//// Venus (circular)
	glPushMatrix();
	glRotatef(planetList[2].angle, 0, 0, 1);
	glTranslatef(0, planetList[2].distFromRef, 0);
	drawDisk(planetList[2].size,
		planetList[2].color[0],
		planetList[2].color[1],
		planetList[2].color[2],
		planetList[2].alpha);

	// draw Ida walking on Venus
	glPushMatrix();
	idaAngle += 30.0f * timer;
	glRotatef(idaAngle, 0, 0, 1);
	glTranslatef(planetList[2].size + 1.2f * idaSize, 0.0f, 0.0f);
	glRotatef(-90.0f, 0, 0, 1);
	drawIda(walkCycle);
	glPopMatrix();

	glPopMatrix();

	// Earth (circular) with Moon
	glPushMatrix();
	glRotatef(planetList[3].angle, 0, 0, 1);
	glTranslatef(0, planetList[3].distFromRef, 0);
	drawDisk(planetList[3].size, planetList[3].color[0], planetList[3].color[1], planetList[3].color[2], planetList[3].alpha);

	// Moon orbiting Earth
	glRotatef(planetList[7].angle, 0, 0, 1);
	glTranslatef(0, planetList[7].distFromRef, 0);
	drawDisk(planetList[7].size, planetList[7].color[0], planetList[7].color[1], planetList[7].color[2], planetList[7].alpha);
	glPopMatrix();

	// Mars (elliptical) with satellite
	glPushMatrix();
	glRotatef(planetList[4].angle, 0, 0, 1);
	glTranslatef(0, planetList[4].distFromRef, 0);
	drawDisk(planetList[4].size, planetList[4].color[0], planetList[4].color[1], planetList[4].color[2], planetList[4].alpha);

	// Satellite orbiting Mars
	glRotatef(planetList[8].angle, 0, 0, 1);
	glTranslatef(0, planetList[8].distFromRef, 0);
	drawDisk(planetList[8].size, planetList[8].color[0], planetList[8].color[1], planetList[8].color[2], planetList[8].alpha);
	glPopMatrix();

	// Jupiter (elliptical)
	drawPlanetElliptical(5, planetList[5].distFromRef, planetList[5].distFromRef * 0.85);

	// Saturn (circular) with rings
	glPushMatrix();
	glRotatef(planetList[6].angle, 0, 0, 1);
	glTranslatef(0, planetList[6].distFromRef, 0);

	// Draw Saturn's rings
	float ringAlpha = clockMode ? 0.4 : 0.8;
	glBegin(GL_LINE_LOOP);
	glColor4f(0.7, 0.7, 0.7, ringAlpha);
	for (int i = 0; i < 50; i++)
	{
		float angle = 2 * PI * i / 50;
		glVertex2f((planetList[6].size + 0.3) * cos(angle), (planetList[6].size + 0.3) * sin(angle) * 0.3);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor4f(0.7, 0.7, 0.7, ringAlpha);
	for (int i = 0; i < 50; i++)
	{
		float angle = 2 * PI * i / 50;
		glVertex2f((planetList[6].size + 0.4) * cos(angle), (planetList[6].size + 0.4) * sin(angle) * 0.3);
	}
	glEnd();

	// Draw Saturn
	drawDisk(planetList[6].size, planetList[6].color[0], planetList[6].color[1], planetList[6].color[2], planetList[6].alpha);
	glPopMatrix();

	glPopMatrix();
	glutSwapBuffers();
}

void idle()
{
	if (!clockMode)
	{
		for (int i = 0; i < numPlanets; i++)
		{
			planetList[i].alpha = 1;
			planetList[i].angle += planetList[i].angularSpeed * timer;
		}
	}
	else
	{
		seconds = time(NULL);
		timeinfo = localtime(&seconds);

		float hourAngle = 0 - (((timeinfo->tm_hour % 12) * 30) + (timeinfo->tm_min * 0.5));
		planetList[1].angle = hourAngle;
		planetList[1].alpha = 1;

		float minuteAngle = 0 - (timeinfo->tm_min * 6);
		planetList[2].angle = minuteAngle;
		planetList[2].alpha = 1;

		float secondAngle = 0 - (timeinfo->tm_sec * 6);
		planetList[3].angle = secondAngle;
		planetList[3].alpha = 1;

		planetList[7].angle += 10;
		planetList[7].alpha = 1;

		idaAngle += 1.5f * timer;
		if (idaAngle >= 360.0f)
			idaAngle -= 360.0f;

		for (int i = 4; i < numPlanets && i != 7; i++)
		{
			planetList[i].alpha = 0.4;
		}
	}

	// background star animation
	starRotation += 0.3f;
	if (starRotation >= 360.0f) starRotation -= 360.0f;

	starBrightness += 0.2f;
	if (starBrightness >= 2 * PI) starBrightness -= 2 * PI;

	// Ida walking animation
	if (legForward) {
		walkCycle += 3.5f;
		if (walkCycle > 20.0f) legForward = false;
	}
	else {
		walkCycle -= 3.5f;
		if (walkCycle < -20.0f) legForward = true;
	}

	glutPostRedisplay();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {

	case 27:
	case 'q':
	case 'Q':
		exit(0);

	case 't':
	case 'T':
		clockMode = !clockMode;
		if (clockMode)
			cout << "Current Mode: Clock mode." << endl;
		else
			cout << "Current Mode: Solar mode." << endl;
		break;

	default:
		break;
	}
}

int main(int argc, char** argv)
{
	cout << "CS3241 Lab 2\n\n";
	cout << "+++++CONTROL BUTTONS+++++++\n\n";
	cout << "Toggle Time Mode: T\n";
	cout << "Exit: ESC or q/Q\n";
	cout << "Current Mode: Solar mode." << endl;

	generatePlanets();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
