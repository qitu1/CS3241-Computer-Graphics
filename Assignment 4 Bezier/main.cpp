// CS3241Lab4.cpp : Defines the entry point for the console application.
// added function:Left mouse drag to move an existing control point
//#include <cmath>
#include "math.h"
#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include "GL/glut.h"
#define M_PI 3.141592654
#elif __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>
#endif

#define MAXPTNO 1000
#define NLINESEGMENT 32
#define NOBJECTONCURVE 8

using namespace std;

// Global variables that you can use
struct Point {
	int x, y;
};

// Storage of control points
int nPt = 0;
Point ptList[MAXPTNO];
// C1 adjusted points storage
Point adjustedPts[MAXPTNO];

// Display options
bool displayControlPoints = true;
bool displayControlLines = true;
bool displayTangentVectors = false;
bool displayObjects = false;
bool C1Continuity = false;

// Dragging state
int selectedPtIdx = -1;
const float SELECTION_RADIUS = 6.0f;

// ========== Bezier Math Functions ==========
// Cubic Bezier interpolation
Point bezierInterpolate(const Point& a, const Point& b, const Point& c, const Point& d, double u) {
	double u2 = u * u;
	double u3 = u2 * u;
	double inv = 1.0 - u;
	double inv2 = inv * inv;
	double inv3 = inv2 * inv;

	Point result;
	result.x = inv3 * a.x + 3 * inv2 * u * b.x + 3 * inv * u2 * c.x + u3 * d.x;
	result.y = inv3 * a.y + 3 * inv2 * u * b.y + 3 * inv * u2 * c.y + u3 * d.y;
	return result;
}

// Compute derivative of cubic Bezier at parameter u
void bezierDerivative(const Point& a, const Point& b, const Point& c, const Point& d,
	double u, double& dx, double& dy) {
	double u2 = u * u;
	double inv = 1.0 - u;
	double inv2 = inv * inv;

	dx = 3 * inv2 * (b.x - a.x) + 6 * inv * u * (c.x - b.x) + 3 * u2 * (d.x - c.x);
	dy = 3 * inv2 * (b.y - a.y) + 6 * inv * u * (c.y - b.y) + 3 * u2 * (d.y - c.y);
}

// Normalize vector
void normalizeVector(double& x, double& y) {
	double mag = sqrt(x * x + y * y);
	if (mag > 1e-6) {
		x /= mag;
		y /= mag;
	}
}

// ========== C1 Continuity Functions ==========
// Compute C1 adjustment for a specific point index
void computeC1ForIndex(int idx) {
	if (idx > 3 && (idx - 1) % 3 == 0) {
		int prevCtrlIdx = idx - 1;  // shared point
		int beforeIdx = idx - 2;    
		adjustedPts[idx].x = 2 * ptList[prevCtrlIdx].x - ptList[beforeIdx].x;
		adjustedPts[idx].y = 2 * ptList[prevCtrlIdx].y - ptList[beforeIdx].y;
	}
	else {
		adjustedPts[idx] = ptList[idx];
	}
}

// Recompute all C1 adjustments
void updateAllC1Adjustments() {
	for (int i = 0; i < nPt; i++) {
		computeC1ForIndex(i);
	}
}

// Get the appropriate point (adjusted or original) based on C1 mode
Point getEffectivePoint(int idx) {
	if (C1Continuity && idx > 3 && (idx - 1) % 3 == 0) {
		return adjustedPts[idx];
	}
	return ptList[idx];
}

// ========== Drawing Functions ==========
// Draw a single cubic Bezier segment
void renderBezierSegment(const Point& p0, const Point& p1, const Point& p2, const Point& p3) {
	glBegin(GL_LINE_STRIP);
	for (int seg = 0; seg <= NLINESEGMENT; seg++) {
		double param = (double)seg / (double)NLINESEGMENT;
		Point pt = bezierInterpolate(p0, p1, p2, p3, param);
		glVertex2f(pt.x, pt.y);
	}
	glEnd();
}

// Draw all Bezier curves
void renderAllCurves() {
	glColor3f(0.0f, 0.0f, 1.0f);
	glLineWidth(1);

	for (int base = 0; base + 3 < nPt; base += 3) {
		Point cp0 = getEffectivePoint(base);
		Point cp1 = getEffectivePoint(base + 1);
		Point cp2 = getEffectivePoint(base + 2);
		Point cp3 = getEffectivePoint(base + 3);

		renderBezierSegment(cp0, cp1, cp2, cp3);
	}
	glLineWidth(1.0f);
}

// Draw control polyline
void renderControlPolyline() {
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINE_STRIP);

	for (int i = 0; i < nPt; i++) {
		Point pt = getEffectivePoint(i);
		glVertex2f(pt.x, pt.y);
	}

	glEnd();
}

// Draw control points with appropriate colors
void renderControlDots() {
	glPointSize(5.0f);
	glBegin(GL_POINTS);

	for (int i = 0; i < nPt; i++) {
		bool isC1Affected = (i > 3 && (i - 1) % 3 == 0);

		if (C1Continuity && isC1Affected) {
			// Draw adjusted point in red
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(adjustedPts[i].x, adjustedPts[i].y);

			// Draw original point in grey
			glColor3f(0.7f, 0.7f, 0.7f);
			glVertex2f(ptList[i].x, ptList[i].y);
		}
		else {
			// Normal black point
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2f(ptList[i].x, ptList[i].y);
		}
	}

	glEnd();
	glPointSize(1.0f);
}

void drawRightArrow()
{
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0, 0);
	glVertex2f(100, 0);
	glVertex2f(95, 5);
	glVertex2f(100, 0);
	glVertex2f(95, -5);
	glEnd();
}

// Draw tangent vectors along curves
void renderTangentArrows() {
	for (int base = 0; base + 3 < nPt; base += 3) {
		Point cp0 = getEffectivePoint(base);
		Point cp1 = getEffectivePoint(base + 1);
		Point cp2 = getEffectivePoint(base + 2);
		Point cp3 = getEffectivePoint(base + 3);

		for (int obj = 0; obj <= NOBJECTONCURVE; obj++) {
			double param = (double)obj / (double)NOBJECTONCURVE;

			Point pos = bezierInterpolate(cp0, cp1, cp2, cp3, param);
			double tanX, tanY;
			bezierDerivative(cp0, cp1, cp2, cp3, param, tanX, tanY);
			normalizeVector(tanX, tanY);

			double angleRad = atan2(tanY, tanX);
			double angleDeg = angleRad * 180.0 / M_PI;

			glPushMatrix();
			glTranslatef(pos.x, pos.y, 0.0f);
			glRotatef(angleDeg, 0.0f, 0.0f, 1.0f);
			drawRightArrow();
			glPopMatrix();
		}
	}
}

void drawEllipse(float radiusX, float radiusY, int segments) {
	glBegin(GL_POLYGON);
	for (int i = 0; i < segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		float x = cos(angle) * radiusX;
		float y = sin(angle) * radiusY;
		glVertex2f(x, y);
	}
	glEnd();
}

// Simple flower object
void drawObject() {
	for (int i = 0; i < 6; i++) {
		glPushMatrix();
		glRotatef(60.0f * i, 0, 0, 1);
		float t = i / 6.0f; 
		float r, g, b;
		
		if (t < 0.5f) {
			float localT = t * 2.0f; 
			r = 1.0f - 0.6f * localT;
			g = 0.4f + 0.3f * localT;
			b = 0.7f + 0.3f * localT;
		}
		else {
			float localT = (t - 0.5f) * 2.0f; 
			r = 0.4f + 0.6f * localT;
			g = 0.7f - 0.3f * localT;
			b = 1.0f - 0.3f * localT;
		}
		glColor3f(r, g, b);
		// Draw paddle
		drawEllipse(5.5f, 12.0f, 20);
		glPopMatrix();
	}
	// Yellow center circle
	glColor3f(1.0f, 0.9f, 0.2f);
	drawEllipse(4.0f, 4.0f, 16);
}

// Draw objects along curves
void renderObjectsAlongCurves() {
	for (int base = 0; base + 3 < nPt; base += 3) {
		Point cp0 = getEffectivePoint(base);
		Point cp1 = getEffectivePoint(base + 1);
		Point cp2 = getEffectivePoint(base + 2);
		Point cp3 = getEffectivePoint(base + 3);

		for (int obj = 0; obj <= NOBJECTONCURVE; obj++) {
			double param = (double)obj / (double)NOBJECTONCURVE;

			Point pos = bezierInterpolate(cp0, cp1, cp2, cp3, param);
			double tanX, tanY;
			bezierDerivative(cp0, cp1, cp2, cp3, param, tanX, tanY);
			normalizeVector(tanX, tanY);

			double angleRad = atan2(tanY, tanX);
			double angleDeg = angleRad * 180.0 / M_PI;

			glPushMatrix();
			glTranslatef(pos.x, pos.y, 0.0f);
			glRotatef(angleDeg, 0.0f, 0.0f, 1.0f);
			drawObject();
			glPopMatrix();
		}
	}
}



void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	renderAllCurves();
	if (displayControlLines) 
	{
		renderControlPolyline();
	}

	if (displayControlPoints) {
		renderControlDots();
	}

	if (displayTangentVectors) {
		renderTangentArrows();
	}

	if (displayObjects) {
		renderObjectsAlongCurves();
	}

	glPopMatrix();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void readFile()
{
	std::ifstream file;
	file.open("savefile.txt");
	file >> nPt;

	if (nPt > MAXPTNO)
	{
		cout << "Error: File contains more than the maximum number of points." << endl;
		nPt = MAXPTNO;
	}

	for (int i = 0; i < nPt; i++)
	{
		file >> ptList[i].x;
		file >> ptList[i].y;
	}
	file.close();// is not necessary because the destructor closes the open file by default
	updateAllC1Adjustments();
}

void writeFile()
{
	std::ofstream file;
	file.open("savefile.txt");
	file << nPt << endl;

	for (int i = 0; i < nPt; i++)
	{
		file << ptList[i].x << " ";
		file << ptList[i].y << endl;
	}
	file.close();// is not necessary because the destructor closes the open file by default
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'r':
	case 'R':
		readFile();
		break;

	case 'w':
	case 'W':
		writeFile();
		break;

	case 'T':
	case 't':
		displayTangentVectors = !displayTangentVectors;
		break;

	case 'o':
	case 'O':
		displayObjects = !displayObjects;
		break;

	case 'p':
	case 'P':
		displayControlPoints = !displayControlPoints;
		break;

	case 'L':
	case 'l':
		displayControlLines = !displayControlLines;
		break;

	case 'C':
	case 'c':
		C1Continuity = !C1Continuity;
		if (C1Continuity) {
			updateAllC1Adjustments();
		}
		break;

	case 'e':
	case 'E':
		nPt = 0;
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

// Check if click is near a control point
int findNearestPoint(int mx, int my) {
	for (int i = 0; i < nPt; i++) {
		float dx = ptList[i].x - mx;
		float dy = ptList[i].y - my;
		float distSq = dx * dx + dy * dy;

		if (distSq <= SELECTION_RADIUS * SELECTION_RADIUS) {
			return i;
		}
	}
	return -1;
}

void mouse(int button, int state, int x, int y)
{
	enum
	{
		MOUSE_LEFT_BUTTON = 0,
		MOUSE_MIDDLE_BUTTON = 1,
		MOUSE_RIGHT_BUTTON = 2,
		MOUSE_SCROLL_UP = 3,
		MOUSE_SCROLL_DOWN = 4
	};

	if (button == MOUSE_LEFT_BUTTON && state == GLUT_DOWN) {
		// Try to select existing point
		selectedPtIdx = findNearestPoint(x, y);
	}
	else if (button == MOUSE_LEFT_BUTTON && state == GLUT_UP) {
		if (selectedPtIdx == -1) {
			// No point was being dragged, add new point
			if (nPt >= MAXPTNO) {
				cout << "Error: Exceeded the maximum number of points." << endl;
				return;
			}

			ptList[nPt].x = x;
			ptList[nPt].y = y;
			nPt++;

			// Update C1 for the newly added point
			computeC1ForIndex(nPt - 1);
		}

		// Release selection
		selectedPtIdx = -1;
	}

	glutPostRedisplay();
}

void motion(int x, int y)
{
	if (selectedPtIdx != -1) {
		// Update dragged point position
		ptList[selectedPtIdx].x = x;
		ptList[selectedPtIdx].y = y;

		// Recalculate all C1 adjustments
		updateAllC1Adjustments();

		glutPostRedisplay();
	}
}

int main(int argc, char** argv)
{
	cout << "CS3241 Lab 4" << endl << endl;
	cout << "Left mouse click: Add a control point" << endl;
	cout << "Left mouse drag: Move an existing control point" << endl;
	cout << "Q: Quit" << endl;
	cout << "P: Toggle displaying control points" << endl;
	cout << "L: Toggle displaying control lines" << endl;
	cout << "E: Erase all points (Clear)" << endl;
	cout << "C: Toggle C1 continuity" << endl;
	cout << "T: Toggle displaying tangent vectors" << endl;
	cout << "O: Toggle displaying objects" << endl;
	cout << "R: Read in control points from \"savefile.txt\"" << endl;
	cout << "W: Write control points to \"savefile.txt\"" << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("CS3241 Assignment 4");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}