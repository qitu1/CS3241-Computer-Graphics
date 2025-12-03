

// CS3241 Assignment 3: Let there be light
#include <cmath>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include "GL/glut.h"
#define M_PI 3.141592654
#elif __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>
#endif

using namespace std;

// global variable

bool m_Smooth = FALSE;
bool m_Highlight = FALSE;
GLfloat angle = 0;   /* in degrees */
GLfloat angle2 = 0;   /* in degrees */
GLfloat zoom = 1.0;
int mouseButton = 0;
int moving, startx, starty;

#define NO_OBJECT 4;
int current_object = 0;
float flameFlicker = 1.0f;

double zNear = 1.0, zFar = 80.0, fovy = 40.0;
bool useBestViewingAngle = false;

using namespace std;


void resetCamera()
{
	angle = 0;
	angle2 = 0;
	zoom = 1.0;
	zNear = 1.0;
	zFar = 80.0;
	fovy = 40.0;
	useBestViewingAngle = false;
}

void setupLighting()
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	// Lights, material properties
	GLfloat	ambientProperties[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat	diffuseProperties[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat	specularProperties[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightPosition[] = { -100.0f,100.0f,100.0f,1.0f };

	glClearDepth(1.0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientProperties);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularProperties);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);

	// Default : lighting
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

}

void setMaterialColor(GLfloat r, GLfloat g, GLfloat b, GLfloat shininess = 80.0f)
{
	
	GLfloat ambient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
	GLfloat diffuse[] = { r, g, b, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

	if (m_Highlight) {
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	}
	else {
		GLfloat noSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glMaterialfv(GL_FRONT, GL_SPECULAR, noSpecular);
		glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);
	}
}


void drawSphere(double r)
{
	glPushMatrix();
	glScalef(r, r, r);
	int i, j;
	int n = 20;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (i = 0; i < 2 * n; i++) {
		for (j = 0; j < n; j++) {
			if (m_Smooth) {
				glBegin(GL_POLYGON);
				glNormal3d(sin(i * M_PI / n) * sin(j * M_PI / n), cos(i * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glVertex3d(sin(i * M_PI / n) * sin(j * M_PI / n), cos(i * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glNormal3d(sin((i + 1) * M_PI / n) * sin(j * M_PI / n), cos((i + 1) * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glVertex3d(sin((i + 1) * M_PI / n) * sin(j * M_PI / n), cos((i + 1) * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glNormal3d(sin((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glVertex3d(sin((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glNormal3d(sin(i * M_PI / n) * sin((j + 1) * M_PI / n), cos(i * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glVertex3d(sin(i * M_PI / n) * sin((j + 1) * M_PI / n), cos(i * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glEnd();
			}
			else {
				glBegin(GL_POLYGON);
				glNormal3d(sin((i + 0.5) * M_PI / n) * sin((j + 0.5) * M_PI / n), cos((i + 0.5) * M_PI / n) * sin((j + 0.5) * M_PI / n), cos((j + 0.5) * M_PI / n));
				glVertex3d(sin(i * M_PI / n) * sin(j * M_PI / n), cos(i * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glVertex3d(sin((i + 1) * M_PI / n) * sin(j * M_PI / n), cos((i + 1) * M_PI / n) * sin(j * M_PI / n), cos(j * M_PI / n));
				glVertex3d(sin((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((i + 1) * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glVertex3d(sin(i * M_PI / n) * sin((j + 1) * M_PI / n), cos(i * M_PI / n) * sin((j + 1) * M_PI / n), cos((j + 1) * M_PI / n));
				glEnd();
			}
		}
	}
	glPopMatrix();
}

void drawCircle(double r, int n, bool isTop)
{
	int i;
	glBegin(GL_POLYGON);
	for (i = 0; i < n; i++)
	{
		double angle = i * 2 * M_PI / n;
		glNormal3d(0.0, 0.0, isTop ? 1.0 : -1.0);
		glVertex3d(r * cos(angle), r * sin(angle), 0.0);
	}
	glEnd();
}

void drawCylinder(double r, double h, int n = 30)
{
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -h / 2.0f);

	int i, j;

	for (i = 0; i < n; i++)  {
		for (j = 0; j < n; j++)  
		{
			double angle1 = i * 2 * M_PI / n;
			double angle2 = (i + 1) * 2 * M_PI / n;
			double z1 = j * h / n;
			double z2 = (j + 1) * h / n;

			if (m_Smooth)  {
				glBegin(GL_POLYGON);
				glNormal3d(cos(angle1), sin(angle1), 0.0);  
				glVertex3d(r * cos(angle1), r * sin(angle1), z1);
				glNormal3d(cos(angle2), sin(angle2), 0.0);
				glVertex3d(r * cos(angle2), r * sin(angle2), z1);
				glNormal3d(cos(angle2), sin(angle2), 0.0);
				glVertex3d(r * cos(angle2), r * sin(angle2), z2);
				glNormal3d(cos(angle1), sin(angle1), 0.0);
				glVertex3d(r * cos(angle1), r * sin(angle1), z2);
				glEnd();
			}
			else  {
				glBegin(GL_POLYGON);
				double midAngle = (i + 0.5) * 2 * M_PI / n;
				glNormal3d(cos(midAngle), sin(midAngle), 0.0);
				glVertex3d(r * cos(angle1), r * sin(angle1), z1);
				glVertex3d(r * cos(angle2), r * sin(angle2), z1);
				glVertex3d(r * cos(angle2), r * sin(angle2), z2);
				glVertex3d(r * cos(angle1), r * sin(angle1), z2);
				glEnd();
			}
		}
	}

	// draw bottom (z = 0)
	drawCircle(r, n, false);  

	// draw top£¨z = h£©
	glTranslatef(0.0f, 0.0f, h);  
	drawCircle(r, n, true); 
	glPopMatrix();
}

// r: radius of the small circle (related to the thickness of the torus)
// R: rotation radius (distance from the center of the torus to the center of the small circle)
void drawTorus(double r, double R, int n=30) {
	glPushMatrix();
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			double theta1 = 2 * M_PI * i / n;       
			double theta2 = 2 * M_PI * (i + 1) / n; 
			double phi1 = 2 * M_PI * j / n;         
			double phi2 = 2 * M_PI * (j + 1) / n;   

			// vertex1£º(theta1, phi1)
			double x1 = (R + r * cos(phi1)) * cos(theta1);
			double y1 = (R + r * cos(phi1)) * sin(theta1);
			double z1 = r * sin(phi1);

			// vertex2£º(theta2, phi1)
			double x2 = (R + r * cos(phi1)) * cos(theta2);
			double y2 = (R + r * cos(phi1)) * sin(theta2);
			double z2 = r * sin(phi1);

			// vertex3£º(theta2, phi2)
			double x3 = (R + r * cos(phi2)) * cos(theta2);
			double y3 = (R + r * cos(phi2)) * sin(theta2);
			double z3 = r * sin(phi2);

			// vertex4£º(theta1, phi2)
			double x4 = (R + r * cos(phi2)) * cos(theta1);
			double y4 = (R + r * cos(phi2)) * sin(theta1);
			double z4 = r * sin(phi2);

			glBegin(GL_QUADS);
			if (m_Smooth) {
				glNormal3d(cos(theta1) * cos(phi1), sin(theta1) * cos(phi1), sin(phi1));
				glVertex3d(x1, y1, z1);

				glNormal3d(cos(theta2) * cos(phi1), sin(theta2) * cos(phi1), sin(phi1));
				glVertex3d(x2, y2, z2);

				glNormal3d(cos(theta2) * cos(phi2), sin(theta2) * cos(phi2), sin(phi2));
				glVertex3d(x3, y3, z3);

				glNormal3d(cos(theta1) * cos(phi2), sin(theta1) * cos(phi2), sin(phi2));
				glVertex3d(x4, y4, z4);
			}
			else {
				double nx = (cos(theta1) * cos(phi1) + cos(theta2) * cos(phi1) + cos(theta2) * cos(phi2) + cos(theta1) * cos(phi2)) / 4;
				double ny = (sin(theta1) * cos(phi1) + sin(theta2) * cos(phi1) + sin(theta2) * cos(phi2) + sin(theta1) * cos(phi2)) / 4;
				double nz = (sin(phi1) + sin(phi1) + sin(phi2) + sin(phi2)) / 4;
				glNormal3d(nx, ny, nz);
				glVertex3d(x1, y1, z1);
				glVertex3d(x2, y2, z2);
				glVertex3d(x3, y3, z3);
				glVertex3d(x4, y4, z4);
			}
			glEnd();
		}
	}
	glPopMatrix();
}

void drawFrustum(double r1, double r2, double h, int n = 30) {
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -h / 2.0f);

	int i;
	for (i = 0; i < n; i++) {
		double angle1 = i * 2 * M_PI / n;
		double angle2 = (i + 1) * 2 * M_PI / n;

		double x1 = r1 * cos(angle1);
		double y1 = r1 * sin(angle1);
		double x2 = r1 * cos(angle2);
		double y2 = r1 * sin(angle2);

		double x3 = r2 * cos(angle2);
		double y3 = r2 * sin(angle2);
		double x4 = r2 * cos(angle1);
		double y4 = r2 * sin(angle1);

		glBegin(GL_QUADS);
		if (m_Smooth) {
			glNormal3d(cos(angle1), sin(angle1), 0.0);
			glVertex3d(x1, y1, 0.0);
			glNormal3d(cos(angle2), sin(angle2), 0.0);
			glVertex3d(x2, y2, 0.0);

			glNormal3d(cos(angle2), sin(angle2), 0.0);
			glVertex3d(x3, y3, h);
			glNormal3d(cos(angle1), sin(angle1), 0.0);
			glVertex3d(x4, y4, h);
		}
		else {
			double midAngle = (angle1 + angle2) / 2;
			glNormal3d(cos(midAngle), sin(midAngle), 0.0);

			glVertex3d(x1, y1, 0.0);
			glVertex3d(x2, y2, 0.0);
			glVertex3d(x3, y3, h);
			glVertex3d(x4, y4, h);
		}
		glEnd();
	}

	// draw bottom£¨r1£©
	glBegin(GL_POLYGON);
	glNormal3d(0.0, 0.0, -1.0); 
	for (i = 0; i < n; i++) {
		double angle = i * 2 * M_PI / n;
		glVertex3d(r1 * cos(angle), r1 * sin(angle), 0.0);
	}
	glEnd();

	// draw top£¨r2£©
	glBegin(GL_POLYGON);
	glNormal3d(0.0, 0.0, 1.0); 
	for (i = 0; i < n; i++) {
		double angle = i * 2 * M_PI / n;
		glVertex3d(r2 * cos(angle), r2 * sin(angle), h);
	}
	glEnd();

	glPopMatrix();
}

void drawWineBottle()
{
	if (useBestViewingAngle) {
		gluLookAt(
			1.0, 0.0, 0.0,   // position of camera
			0.0, 0.0, 0.0,   // viewpoint
			0.0, 0.5, 1.0    // up
		);
	}
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.3f);
	// body of bottle (black)
	setMaterialColor(0.0f, 0.0f, 0.0f, 80.0f);
	drawCylinder(0.25, 1.3);

	// label (white and red)
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.1f);
	setMaterialColor(0.4f, 0.0f, 0.12f, 40.0f);
	drawCylinder(0.26, 0.15);

	glTranslatef(0.0f, 0.0f, 0.3f);
	setMaterialColor(0.95f, 0.95f, 0.9f,20.0f);
	drawCylinder(0.26, 0.45);  

	glPopMatrix();

	// bottleneck
	setMaterialColor(0.0f, 0.0f, 0.0f, 80.0f);
	glTranslatef(0.0f, 0.0f, 0.65f);
	drawSphere(0.25);
	glTranslatef(0.0f, 0.0f, 0.25f);
	drawCylinder(0.1, 0.7);
	setMaterialColor(0.4f, 0.0f, 0.12f, 40.0f);
	glTranslatef(0.0f, 0.0f, 0.5f);
	drawCylinder(0.1, 0.3);
	glTranslatef(0.0f, 0.0f, 0.06f);
	drawCylinder(0.11, 0.08);
	glPopMatrix();

	glPopMatrix();
}

void updateFlameFlicker()
{
	// Create a slight flickering effect using time + random perturbations
	float t = glutGet(GLUT_ELAPSED_TIME) * 0.005f;
	flameFlicker = 1.0f + 0.15f * sin(t) + 0.08f * cos(2.3f * t);
}

void drawFlame()
{
	glPushMatrix();

	// point light source at the flame's position
	GLfloat flame_light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat flame_light_diffuse[] = { 1.0f, 0.5f, 0.2f, 1.0f };
	GLfloat flame_light_specular[] = { 1.0f, 0.7f, 0.3f, 1.0f };

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, flame_light_position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, flame_light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, flame_light_specular);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.6f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 1.2f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	// layered spheres to simulate the flame
	GLfloat outer_red[] = { 0.9f, 0.2f, 0.1f, 0.25f };
	GLfloat middle_orange[] = { 1.0f, 0.55f, 0.1f, 0.35f };
	GLfloat inner_yellow[] = { 1.0f, 0.9f, 0.3f, 0.6f };
	GLfloat core_white[] = { 1.0f, 1.0f, 0.9f, 0.9f };

	glMaterialfv(GL_FRONT, GL_EMISSION, outer_red);
	glPushMatrix();
	glScalef(0.7 * flameFlicker, 0.7f * flameFlicker, 2.3f * flameFlicker);
	drawSphere(0.03);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_EMISSION, middle_orange);
	glPushMatrix();
	glScalef(0.5f * flameFlicker, 0.5f * flameFlicker, 1.8f * flameFlicker);
	drawSphere(0.02);
	glPopMatrix();

	glMaterialfv(GL_FRONT, GL_EMISSION, inner_yellow);
	drawSphere(0.015f * flameFlicker);

	glMaterialfv(GL_FRONT, GL_EMISSION, core_white);
	drawSphere(0.012f * flameFlicker);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
	glPopMatrix();
}

void drawCandle()
{
	if (useBestViewingAngle) {
		gluLookAt(
			1.0, 0.0, 0.2, 
			0.0, 0.0, 0.0,   
			0.0, 0.0, 1.0    
		);
	}
	glPushMatrix();
	// core of candlestick
	setMaterialColor(0.3f, 0.25f, 0.2f, 100.0f); // Dark brown
	drawCylinder(0.1, 1.5);
	// middle part
	glPushMatrix();
	glScalef(1.0f, 1.0f, 0.1f);
	drawSphere(0.3);
	glScalef(1.0f, 1.0f, 12.0f);
	drawTorus(0.025, 0.3);
	glPopMatrix();

	// lower part
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.12f);
	drawCylinder(0.15, 0.08);
	glTranslatef(0.0f, 0.0f, -0.03f);
	glPushMatrix();
	glScalef(1.0f, 1.0f, 0.4f);
	drawSphere(0.18);
	glPopMatrix();
	glTranslatef(0.0f, 0.0f, -0.3f);
	drawFrustum(0.22, 0.1, 0.3);
	glTranslatef(0.0f, 0.0f, -0.16f);
	glScalef(1.0f, 1.0f, 0.4f);
	drawSphere(0.22);
	glScalef(1.0f, 1.0f, 2.5f);
	glTranslatef(0.0f, 0.0f, -0.15f);
	drawFrustum(0.26, 0.13, 0.08);
	glTranslatef(0.0f, 0.0f, -0.04f);
	drawTorus(0.025, 0.26);
	glPopMatrix();

	// upper part
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.05f);
	drawCylinder(0.13, 0.1); 
	glTranslatef(0.0f, 0.0f, 0.07f);
	drawFrustum(0.13, 0.1, 0.04);
	glTranslatef(0.0f, 0.0f, 0.08f);
	drawFrustum(0.1, 0.18, 0.06);
	glTranslatef(0.0f, 0.0f, 0.15f);
	drawCylinder(0.18, 0.24);
	glTranslatef(0.0f, 0.0f, 0.16f);
	drawFrustum(0.18, 0.2, 0.08);
	glTranslatef(0.0f, 0.0f, 0.06f);
	glScalef(1.0f, 1.0f, 3.0f);
	drawTorus(0.01, 0.2);
	glPopMatrix();

	// candle (white)
	glPushMatrix();
	setMaterialColor(0.95f, 0.92f, 0.85f, 5.0f);
	glTranslatef(0.0f, 0.0f, 0.65f);
	drawCylinder(0.13, 0.3);
	// wick (black)
	glTranslatef(0.0f, 0.0f, 0.15f);
	setMaterialColor(0.0f, 0.0f, 0.0f, 5.0f);
	drawCylinder(0.005, 0.16);
	// flame
	glTranslatef(0.0f, 0.0f, 0.1f);
	drawFlame();
	glPopMatrix();

	glPopMatrix();
}
	
void display(void)
{
	updateFlameFlicker();
	if (current_object != 3) {  
		glDisable(GL_LIGHT1);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();							// reset the projection matrix
	gluPerspective(fovy, 1.0, zNear, zFar);		// apply new projection matrix
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(0, 0, -6);

	glRotatef(angle2, 1.0, 0.0, 0.0);
	glRotatef(angle, 0.0, 1.0, 0.0);

	glScalef(zoom, zoom, zoom);

	switch (current_object) {
	case 0:
		setMaterialColor(0.4f, 0.0f, 0.12f);
		drawSphere(1);
		break;
	case 1:
		// draw your second primitive object here
		setMaterialColor(0.3f, 0.25f, 0.2f);
		drawTorus(0.2, 1);
		break;
	case 2:
		// draw your first composite object here
		drawWineBottle();
		break;
	case 3:
		// draw your second composite object here
		glPushMatrix();
		glScalef(1.3f, 1.3f, 1.3f);
		drawCandle();
		glPopMatrix();
		break;
	default:
		break;
	};
	glPopMatrix();
	glutSwapBuffers();
}




void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'p':
	case 'P':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'w':
	case 'W':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'v':
	case 'V':
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 's':
	case 'S':
		m_Smooth = !m_Smooth;
		break;
	case 'h':
	case 'H':
		m_Highlight = !m_Highlight;
		break;


	case '1':
	case '2':
	case '3':
	case '4':
		current_object = key - '1';
		break;

	case 'n':
		zNear -= 1;
		break;
	case 'N':
		zNear += 1;
		break;
	case 'f':
		zFar -= 1;
		break;
	case 'F':
		zFar += 1;
		break;
	case 'o':
		fovy -= 5;
		break;
	case 'O':
		fovy += 5;
		break;

	case 'r':
		resetCamera();
		break;

	case 'R':
		useBestViewingAngle = !useBestViewingAngle;
		break;

	case 'Q':
	case 'q':
		exit(0);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}



void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		mouseButton = button;
		moving = 1;
		startx = x;
		starty = y;
	}
	if (state == GLUT_UP) {
		mouseButton = button;
		moving = 0;
	}
}

void motion(int x, int y)
{
	if (moving) {
		if (mouseButton == GLUT_LEFT_BUTTON)
		{
			angle = angle + (x - startx);
			angle2 = angle2 + (y - starty);
		}
		else zoom += ((y - starty) * 0.001);
		startx = x;
		starty = y;
		glutPostRedisplay();
	}

}

int main(int argc, char** argv)
{
	cout << "CS3241 Lab 3" << endl << endl;

	cout << "1-4: Draw different objects" << endl;
	cout << "S: Toggle Smooth Shading" << endl;
	cout << "H: Toggle Highlight" << endl;
	cout << "W: Draw Wireframe" << endl;
	cout << "P: Draw Polygon" << endl;
	cout << "V: Draw Vertices" << endl;
	cout << "o/O: Reduce/Increase fovy" << endl;
	cout << "n/N: Reduce/Increase near plane distance" << endl;
	cout << "f/F: Reduce/Increase far plane distance" << endl;
	cout << "r: Reset camera" << endl;
	cout << "R: Switch to best viewing angle" << endl;

	cout << "Q: Quit" << endl << endl;

	cout << "Left mouse click and drag: rotate the object" << endl;
	cout << "Right mouse click and drag: zooming" << endl;

	glutInit(&argc, argv);
	glutIdleFunc(display);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("CS3241 Assignment 3");
	glClearColor(0.82f, 0.71f, 0.55f, 1.0f);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	setupLighting();
	//resetCamera();
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	gluPerspective( /* field of view in degree */ fovy,
		/* aspect ratio */ 1.0,
		/* Z near */ zNear, /* Z far */ zFar);
	glMatrixMode(GL_MODELVIEW);
	glutMainLoop();

	return 0;
}
