//////////////////////////////////////////////////////////////////////////////////////////////////////
// Name: Feng Ruiqi
// Extra function I used: glutBitmapCharacter(), glRasterPos2f(), glLineWidth(), glPointSize()
//////////////////////////////////////////////////////////////////////////////////////////////////////

// CS3241 Assignment 1: Doodle
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

GLfloat PI = 3.14;
float alpha = 0.0, k = 1;
float tx = 0.0, ty = 0.0;

void drawCircle(float cx, float cy, float r, int segments = 50) {
	glBegin(GL_POLYGON);
	for (int i = 0; i < segments; i++) {
		float theta = 2.0f * PI * float(i) / float(segments);
		float x = r * cos(theta);
		float y = r * sin(theta);
		glVertex2f(cx + x, cy + y);
	}
	glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, int segments = 32) {
	glBegin(GL_POLYGON);
	for (int i = 0; i < segments; i++) {
		float theta = 2.0f * PI * float(i) / float(segments);
		float x = rx * cos(theta);
		float y = ry * sin(theta);
		glVertex2f(cx + x, cy + y);
	}
	glEnd();
}

void drawArc(float centerX, float centerY, float radius, float startAngle, float endAngle, int segments = 30) {
	float startRad = startAngle * M_PI / 180.0f;
	float endRad = endAngle * M_PI / 180.0f;
	if (endRad < startRad) {
		endRad += 2 * M_PI;
	}
	float angleStep = (endRad - startRad) / segments;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= segments; i++) {
		float angle = startRad + i * angleStep;
		float x = centerX + radius * cos(angle);
		float y = centerY + radius * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
}

void drawRect(float centerX, float centerY, float width, float height) {
	float halfWidth = width / 2;
	float halfHeight = height / 2;
	glBegin(GL_POLYGON);
	glVertex2f(centerX - halfWidth, centerY - halfHeight);
	glVertex2f(centerX + halfWidth, centerY - halfHeight);
	glVertex2f(centerX + halfWidth, centerY + halfHeight);
	glVertex2f(centerX - halfWidth, centerY + halfHeight);
	glEnd();
}


void drawMinionBody(float bodyWidth, float bodyHeight, int type = 0) 
{
	float rectHeight = bodyHeight * 0.6f; //middle rectangle height
	float topEllipseHeight = bodyHeight * 0.25f; // upper ellipse height
	float bottomEllipseHeight = bodyHeight * 0.15f; // lower ellipse height

	// determine pants height ratio based on type
	float pantsHeightRatio = (type == 0) ? 0.62f : ((type == 1) ? 0.5f : 0.43f);

	// upper yellow part of the body
	glColor3f(0.96f, 0.85f, 0.20f); // yellow
	drawEllipse(0, rectHeight / 2, bodyWidth / 2, topEllipseHeight, 50);
	float pantsStartY = -rectHeight / 2 + rectHeight * (1 - pantsHeightRatio);
	drawRect(0, (pantsStartY + rectHeight / 2) / 2, bodyWidth, rectHeight / 2 - pantsStartY);

	// blue overalls
	glColor3f(0.34f, 0.53f, 0.7f); // blue
	drawEllipse(0, -rectHeight / 2, bodyWidth / 2, bottomEllipseHeight, 50);
	drawRect(0, (pantsStartY - rectHeight / 2) / 2, bodyWidth, rectHeight / 2 + pantsStartY);


	glColor3f(0.96f, 0.85f, 0.20f); 
	// upper trapezoid 
	glBegin(GL_POLYGON);
	glVertex2f(-bodyWidth / 2, pantsStartY ); 
	glVertex2f(bodyWidth / 2, pantsStartY );  
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY  - 0.15); 
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY  - 0.15); 
	glEnd();

	// left trapezoid (left strap)
	glBegin(GL_POLYGON);
	glVertex2f(-bodyWidth / 2, pantsStartY  - 0.15); 
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY  - 0.30);  
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY  - 0.75); 
	glVertex2f(-bodyWidth / 2, pantsStartY  - 0.75); 
	glEnd();

	// right trapezoid (right strap)
	glBegin(GL_POLYGON);
	glVertex2f(bodyWidth / 2, pantsStartY  - 0.15); 
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY  - 0.30);  
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY  - 0.75); 
	glVertex2f(bodyWidth / 2, pantsStartY  - 0.75); 
	glEnd();


	// outline of the overalls
	glColor3f(0.2f, 0.2f, 0.2f); // black

	glBegin(GL_LINE_STRIP);
	glVertex2f(bodyWidth / 2, pantsStartY );
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY - 0.15);
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY - 0.15);
	glVertex2f(-bodyWidth / 2, pantsStartY);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex2f(-bodyWidth / 2, pantsStartY  - 0.15);
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY - 0.30);
	glVertex2f(-bodyWidth / 2 + 0.35, pantsStartY - 0.75);
	glVertex2f(-bodyWidth / 2, pantsStartY - 0.75);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex2f(bodyWidth / 2, pantsStartY  - 0.15);
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY  - 0.30);
	glVertex2f(bodyWidth / 2 - 0.35, pantsStartY  - 0.75);
	glVertex2f(bodyWidth / 2, pantsStartY - 0.75);
	glEnd();

	glColor3f(0.14f, 0.33f, 0.5f); // darker blue for pocket and buttons
	glBegin(GL_LINE_LOOP);
	glVertex2f(-bodyWidth / 2 + 0.6, pantsStartY - 0.4); 
	glVertex2f(bodyWidth / 2 - 0.6, pantsStartY - 0.4);  
	glVertex2f(bodyWidth / 2 - 0.64, pantsStartY  - 0.75); 
	glVertex2f(-bodyWidth / 2 + 0.64, pantsStartY  - 0.75); 
	glEnd();

	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex2f(bodyWidth / 2 - 0.45, pantsStartY  - 0.26); 
	glVertex2f(-bodyWidth / 2 + 0.45, pantsStartY - 0.26); 
	glEnd();
}


void drawMinionBodyOutline(float bodyWidth, float bodyHeight) 
{
	float rectHeight = bodyHeight * 0.6f;
	float topEllipseHeight = bodyHeight * 0.25f;
	float bottomEllipseHeight = bodyHeight * 0.15f;

	glColor3f(0.2f, 0.2f, 0.2f); 

	glBegin(GL_LINE_STRIP);
	for (int i = 50; i >= 0; i--) {
		float theta = PI + PI * float(i) / 50.0f;
		float x = (bodyWidth / 2) * cos(theta);
		float y = bottomEllipseHeight * sin(theta) - rectHeight / 2;
		glVertex2f(x, y);
	}

	glVertex2f(-bodyWidth / 2, rectHeight / 2);

	for (int i = 50; i >= 0; i--) {
		float theta = PI * float(i) / 50.0f;
		float x = (bodyWidth / 2) * cos(theta);
		float y = topEllipseHeight * sin(theta) + rectHeight / 2;
		glVertex2f(x, y);
	}
	glVertex2f(bodyWidth / 2, rectHeight / 2);
	glVertex2f(bodyWidth / 2, -rectHeight / 2);
	glEnd();
}

void drawLimb(float length, float limbWidth, bool isMirrored = false, bool isFoot = false)
{
	glPushMatrix();
	if (isMirrored) {	
		glScalef(-1, 1, 1); // y-axis mirror
	}

	// if isFoot, apply compression and position adjustment
	float actualLength = isFoot ? length * 0.7f : length;
	float actualWidth = isFoot ? limbWidth * 1.5f : limbWidth;
	float yOffset = isFoot ? -actualWidth / 2 : 0.0f;
	float innerOffset = isFoot ? 0.4f : 0.7f;


	glColor3f(0.2f, 0.2f, 0.2f);
	drawRect(actualLength / 2, yOffset, actualLength, actualWidth);

	if (isFoot == false)	glColor3f(0.96f, 0.85f, 0.20f);
	else glColor3f(0.34f, 0.53f, 0.7f);
	drawRect(actualLength / 2, yOffset, actualLength - innerOffset, actualWidth - (isFoot ? 0.0f : 0.0f));

	// outline of the limb
	glColor3f(0.2f, 0.2f, 0.2f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(actualLength / 2 - actualLength / 2, yOffset - actualWidth / 2);
	glVertex2f(actualLength / 2 + actualLength / 2, yOffset - actualWidth / 2);
	glVertex2f(actualLength / 2 + actualLength / 2, yOffset + actualWidth / 2);
	glVertex2f(actualLength / 2 - actualLength / 2, yOffset + actualWidth / 2);
	glEnd();

	if (isFoot == false)	drawCircle(actualLength - 0.1, 0.1, actualWidth * 0.8);
	else drawCircle(actualLength - 0.08, -0.35, actualWidth * 0.4);
	glPopMatrix();
}

void drawLimbs(float bodyWidth, float bodyHeight, int type)
{
	// parameters for arms
	float armStartX = -bodyWidth / 2 + 0.3f;
	float armStartY = (type == 0) ? 0.2f : ((type == 1) ? 0.4f : 0.5f);
	float armLength = (type == 0) ? bodyHeight * 0.57f : ((type == 1) ? bodyHeight * 0.65f : bodyHeight * 0.7f);
	float armWidth = 0.17f;
	float armRotation = -110.0f;

	// parameters for legs
	float footStartX = -bodyWidth / 2 + 0.83f;
	float footStartY = -bodyHeight * 0.3f;
	float footLength = (type == 0) ? bodyHeight * 0.38f : ((type == 1) ? bodyHeight * 0.4f : bodyHeight * 0.41f);
	//float footLength = 2.5 * 0.4f;
	float footWidth = 0.2f;
	float footRotation = -90.0f;

	// right arm
	glPushMatrix();
	glTranslatef(armStartX, armStartY, 0.0f);
	glRotatef(armRotation, 0.0f, 0.0f, 1.0f);
	drawLimb(armLength, armWidth, false, false);
	glPopMatrix();

	// left arm
	glPushMatrix();
	glTranslatef(-armStartX, armStartY, 0.0f);
	glRotatef(-armRotation, 0.0f, 0.0f, 1.0f);
	drawLimb(armLength, armWidth, true, false);
	glPopMatrix();

	// right leg
	glPushMatrix();
	glTranslatef(footStartX, footStartY, 0.0f);
	glRotatef(footRotation, 0.0f, 0.0f, 1.0f);
	drawLimb(footLength, footWidth, false, true);
	glPopMatrix();

	// left leg
	glPushMatrix();
	glTranslatef(-footStartX, footStartY, 0.0f);
	glRotatef(-footRotation, 0.0f, 0.0f, 1.0f);
	drawLimb(footLength, footWidth, true, true);
	glPopMatrix();
}

void drawGlasses(float bodyWidth, float bodyHeight, int type)
{
	float eyeY = (type == 0) ? bodyHeight * 0.32f : ((type == 1) ? bodyHeight * 0.3f : bodyHeight * 0.32f);

	glColor3f(0.2f, 0.2f, 0.2f);
	// glasses strap
	drawRect(0, eyeY, bodyWidth + 0.18, 0.2);

	switch (type) {
	case 0: // 2 eyes
		drawCircle(bodyWidth * 0.22f, eyeY, 0.42);
		drawCircle(-bodyWidth * 0.22f, eyeY, 0.42);

		glColor3f(1.0f, 1.0f, 1.0f);
		drawCircle(bodyWidth * 0.22f, eyeY, 0.29);
		drawCircle(-bodyWidth * 0.22f, eyeY, 0.29);

		glColor3f(0.2f, 0.2f, 0.2f);
		drawCircle(bodyWidth * 0.2f, eyeY - 0.03, 0.115);
		drawCircle(-bodyWidth * 0.2f, eyeY - 0.03, 0.115);
		break;
	case 1:  // 1 eye
		drawCircle(0, eyeY, 0.57);
		glColor3f(1.0f, 1.0f, 1.0f);
		drawCircle(0, eyeY, 0.44);

		glColor3f(0.2f, 0.2f, 0.2f);
		drawCircle(0, eyeY, 0.18);

		//eyelid
		glColor3f(0.96f, 0.85f, 0.20f);
		glBegin(GL_POLYGON);
		for (int i = 0; i <= 32; i++) {
			float theta = PI * float(i) / 32.0f; 
			float x = 0.44f * cos(theta); 
			float y = 0.44f * sin(theta) + eyeY;

			if (y >= eyeY + 0.07) { 
				glVertex2f(x, y);
			}
		}
		glEnd();

		glBegin(GL_LINES);
		glColor3f(0.2f, 0.2f, 0.2f);
		glVertex2f(0.44f, eyeY + 0.1f);
		glVertex2f(-0.44f, eyeY + 0.1f);
		glEnd();

		break;
	case 2:  // 2 eyes, bigger
		drawCircle(-bodyWidth * 0.22f, eyeY, 0.47);
		drawCircle(bodyWidth * 0.22f, eyeY, 0.47);
		glColor3f(1.0f, 1.0f, 1.0f);
		drawCircle(-bodyWidth * 0.22f, eyeY, 0.34);
		drawCircle(bodyWidth * 0.22f, eyeY, 0.34);

		glColor3f(0.2f, 0.2f, 0.2f);
		drawCircle(-bodyWidth * 0.19f, eyeY - 0.07, 0.15);
		drawCircle(bodyWidth * 0.19f, eyeY - 0.07, 0.15);

		break;
	}
}

void drawHairAndMouth(float bodyWidth, float bodyHeight, int type) 
{
	glColor3f(0.2f, 0.2f, 0.2f); 
	switch (type) {
	case 0:
		glPushMatrix();
		// hair
		glTranslatef(0.0f, 1.98f, 0.0f);
		drawArc(-0.5f, 0.0f, 0.5f, 0.0f, 70.0f);
		drawArc(0.5f, 0.0f, 0.5f, 110.0f, 180.0f);
		drawArc(-1.2, 0.0f, 1.2f, 0.0f, 35.0f);
		drawArc(1.2f, 0.0f, 1.2f, 145.0f, 180.0f);
		// mouth
		glTranslatef(-0.1f, -1.22f, 0.0f);
		drawArc(0.0f, 0.0f, 0.8f, 263.0f, 310.0f);
		glPopMatrix();
		break;

	case 1:
		glPushMatrix();
		glTranslatef(0.0f, 1.648f, 0.0f);
		drawArc(0.25f, -0.3f, 0.4f, 25.0f, 130.0f);
		drawArc(-0.25f, -0.3f, 0.4f, 50.0f, 155.0f);
		glTranslatef(0.0f, -1.92f, 0.0f);
		drawArc(0.0f, 0.0f, 0.4f, 63.0f, 117.0f);
		glPopMatrix();
		break;

	case 2:
		glPushMatrix();
		glTranslatef(0.0f, 0.3f, 0.0f);
		glBegin(GL_POLYGON);
		for (int i = 0; i <= 32; i++) {
			float startAngle = 200.0f * PI / 180.0f;  
			float endAngle = 340.0f * PI / 180.0f;    
			float theta = startAngle + (endAngle - startAngle) * float(i) / 32.0f;
			float x = 0.23f * cos(theta);  
			float y = 0.23f * sin(theta);  
			float boundaryY = 0.23f * sin(200.0f * PI / 180.0f);  
			if (y <= boundaryY) {
				glVertex2f(x, y);
			}
		}
		glEnd();
		glPopMatrix();
		break;
	}
}


void drawMinion(float centerX, float centerY, float scale, int type) 
{
	glPushMatrix();
	glTranslatef(centerX, centerY, 0);
	glScalef(scale, scale, 1);

	// adjust body dimensions based on type
	// type 0: tall, type 1: medium, type 2: short
	float bodyHeight = (type == 0) ? 3.6f : ((type == 1) ? 3.0f : 2.4f);
	float bodyWidth = 1.8f;

	drawLimbs(bodyWidth, bodyHeight, type);
	drawMinionBody(bodyWidth, bodyHeight,type);
	drawMinionBodyOutline(bodyWidth, bodyHeight);
	drawGlasses(bodyWidth, bodyHeight, type);
	drawHairAndMouth(bodyWidth, bodyHeight, type);
	glPopMatrix();
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//controls transformation
	glScalef(k, k, k);
	glTranslatef(tx, ty, 0);
	glRotatef(alpha, 0, 0, 1);

	// upper background color
	glColor3f(0.96f, 0.85f, 0.20f);
	drawRect(0, 7.5, 40, 25);

	// lower background color
	glColor3f(0.34f, 0.53f, 0.7f);
	drawRect(0, -12.5, 40, 15);

	// 3 minions
	drawMinion(-5.8, -0.9, 2.0f, 0);
	drawMinion(5.5, -1.5, 2.0f, 1);
	drawMinion(0, -2.18, 2.0f, 2);

	glColor3f(0.96f, 0.85f, 0.20f);
	glRasterPos2f(-1.32f, -7.8f);  
	const char* text = "MINIONS";
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	glPopMatrix();
	glFlush();
}

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
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glLineWidth(5.0f);
}


void keyboard(unsigned char key, int x, int y)
{
	//keys to control scaling - k
	//keys to control rotation - alpha
	//keys to control translation - tx, ty
	switch (key) {

	case 'a':
		alpha += 10;
		glutPostRedisplay();
		break;

	case 'd':
		alpha -= 10;
		glutPostRedisplay();
		break;

	case 'q':
		k += 0.1;
		glutPostRedisplay();
		break;

	case 'e':
		if (k > 0.1)
			k -= 0.1;
		glutPostRedisplay();
		break;

	case 'z':
		tx -= 0.1;
		glutPostRedisplay();
		break;

	case 'c':
		tx += 0.1;
		glutPostRedisplay();
		break;

	case 's':
		ty -= 0.1;
		glutPostRedisplay();
		break;

	case 'w':
		ty += 0.1;
		glutPostRedisplay();
		break;

	default:
		break;
	}
}

int main(int argc, char** argv)
{
	cout << "CS3241 Lab 1\n\n";
	cout << "+++++CONTROL BUTTONS+++++++\n\n";
	cout << "Scale Up/Down: Q/E\n";
	cout << "Rotate Clockwise/Counter-clockwise: A/D\n";
	cout << "Move Up/Down: W/S\n";
	cout << "Move Left/Right: Z/C\n";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
