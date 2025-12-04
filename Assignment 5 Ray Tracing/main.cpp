// CS3241Lab5.cpp 
// Add stb_image and texture.jpg to the same directory as main.cpp for texture loading
#include <cmath>
#include <iostream>
#include "GL\glut.h"
#include "vector3D.h"
#include <chrono>

using namespace std;

#define WINWIDTH 600
#define WINHEIGHT 400
#define NUM_OBJECTS 5
#define MAX_RT_LEVEL 50
#define NUM_SCENE 2

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float* pixelBuffer = new float[WINWIDTH * WINHEIGHT * 3];

class Ray { // a ray that start with "start" and going in the direction "dir"
public:
	Vector3 start, dir;
};

class RtObject {

public:
	virtual double intersectWithRay(Ray, Vector3& pos, Vector3& normal) = 0; // return a -ve if there is no intersection. Otherwise, return the smallest postive value of t
	virtual void getTextureCoords(Vector3 pos, double& u, double& v) = 0; // calculate texture coordinates

	// Materials Properties
	double ambiantReflection[3];
	double diffusetReflection[3];
	double specularReflection[3];
	double speN = 300;

	double transparency = 0.0;
	double refractiveIndex = 1.5;

	bool hasTexture = false;
	unsigned char* textureData = nullptr;
	int textureWidth = 0;
	int textureHeight = 0;
	int textureChannels = 0;

	bool hasBumpMap = false;
	double bumpStrength = 1.0;
};

Vector3 refract(Vector3 incident, Vector3 normal, double eta) {
	double cosI = -dot_prod(normal, incident);
	double sinT2 = eta * eta * (1.0 - cosI * cosI);

	if (sinT2 > 1.0)
		return Vector3(0, 0, 0);

	double cosT = sqrt(1.0 - sinT2);
	return incident * eta + normal * (eta * cosI - cosT);
};

class Sphere : public RtObject {

	Vector3 center_;
	double r_;
public:
	Sphere(Vector3 c, double r) { center_ = c; r_ = r; };
	Sphere() {};
	void set(Vector3 c, double r) { center_ = c; r_ = r; };
	double intersectWithRay(Ray, Vector3& pos, Vector3& normal);

	void getTextureCoords(Vector3 pos, double& u, double& v) {
		Vector3 d = pos - center_;
		d.normalize();
		u = 0.5 + atan2(d.x[2], d.x[0]) / (2.0 * 3.14159265359);
		v = 0.5 - asin(d.x[1]) / 3.14159265359;
	}
};

class Tetrahedron : public RtObject {
	Vector3 vertices[4];  // 4 vertices of tetrahedron

	// Helper function: Ray-Triangle intersection using Möller-Trumbore algorithm
	double rayTriangleIntersect(Ray ray, Vector3 v0, Vector3 v1, Vector3 v2, Vector3& normal);

public:
	Tetrahedron(Vector3 center, double size);
	Tetrahedron() {};
	void set(Vector3 center, double size);
	double intersectWithRay(Ray, Vector3& pos, Vector3& normal);

	void getTextureCoords(Vector3 pos, double& u, double& v) {
		u = (pos.x[0] + 200.0) / 400.0;
		v = (pos.x[2] + 200.0) / 400.0;
	}
};

RtObject** objList; // The list of all objects in the scene


// Global Variables
// Camera Settings
Vector3 cameraPos(0, 0, -500);

// assume the the following two vectors are normalised
Vector3 lookAtDir(0, 0, 1);
Vector3 upVector(0, 1, 0);
Vector3 leftVector(1, 0, 0);
float focalLen = 500;

// Light Settings

Vector3 lightPos(900, 1000, -1500);
double ambiantLight[3] = { 0.4,0.4,0.4 };
double diffusetLight[3] = { 0.7,0.7, 0.7 };
double specularLight[3] = { 0.5,0.5, 0.5 };


double bgColor[3] = { 0.1,0.1,0.4 };

int sceneNo = 0;

void sampleTexture(RtObject* obj, double u, double v, double& r, double& g, double& b) {
	if (!obj->hasTexture || obj->textureData == nullptr) {
		r = g = b = 1.0;
		return;
	}

	u = u - floor(u);
	v = v - floor(v);

	int x = (int)(u * obj->textureWidth) % obj->textureWidth;
	int y = (int)(v * obj->textureHeight) % obj->textureHeight;

	int index = (y * obj->textureWidth + x) * obj->textureChannels;

	r = obj->textureData[index] / 255.0;
	if (obj->textureChannels >= 2) {
		g = obj->textureData[index + 1] / 255.0;
	}
	else {
		g = r;
	}
	if (obj->textureChannels >= 3) {
		b = obj->textureData[index + 2] / 255.0;
	}
	else {
		b = r;
	}
}

Vector3 perturbNormal(RtObject* obj, Vector3 normal, Vector3 pos, double u, double v) {
	if (!obj->hasBumpMap || obj->textureData == nullptr) {
		return normal;
	}

	double offset = 0.001;

	double r0, g0, b0, r1, g1, b1, r2, g2, b2;
	sampleTexture(obj, u, v, r0, g0, b0);
	sampleTexture(obj, u + offset, v, r1, g1, b1);
	sampleTexture(obj, u, v + offset, r2, g2, b2);

	double h0 = (r0 + g0 + b0) / 3.0;
	double h1 = (r1 + g1 + b1) / 3.0;
	double h2 = (r2 + g2 + b2) / 3.0;

	double du = (h1 - h0) * obj->bumpStrength;
	double dv = (h2 - h0) * obj->bumpStrength;

	Vector3 tangent, bitangent;
	if (fabs(normal.x[0]) > 0.1) {
		tangent = Vector3(normal.x[1], -normal.x[0], 0);
	}
	else {
		tangent = Vector3(0, normal.x[2], -normal.x[1]);
	}
	tangent.normalize();
	bitangent = cross_prod(normal, tangent);
	bitangent.normalize();

	Vector3 perturbedNormal = normal + tangent * du + bitangent * dv;
	perturbedNormal.normalize();

	return perturbedNormal;
}

bool loadTexture(RtObject* obj, const char* filename) {
	cout << "Loading texture: " << filename << " ... ";

	obj->textureData = stbi_load(filename, &obj->textureWidth,
		&obj->textureHeight, &obj->textureChannels, 3);

	if (obj->textureData == nullptr) {
		cout << "Failed! Error: " << stbi_failure_reason() << endl;
		return false;
	}

	obj->hasTexture = true;
	cout << "Success!" << endl;
	return true;
}


double Sphere::intersectWithRay(Ray r, Vector3& intersection, Vector3& normal)
// return a -ve if there is no intersection. Otherwise, return the smallest postive value of t
{
	// Step 1: Ray-Sphere Intersection

	Vector3 oc = r.start - center_;  // O - C

	double a = dot_prod(r.dir, r.dir);
	double b = 2.0 * dot_prod(oc, r.dir);
	double c = dot_prod(oc, oc) - r_ * r_;
	double discriminant = b * b - 4 * a * c;

	if (discriminant < 0)
		return -1;

	double sqrtDisc = sqrt(discriminant);
	double t1 = (-b - sqrtDisc) / (2.0 * a);
	double t2 = (-b + sqrtDisc) / (2.0 * a);

	double t = -1;
	if (t1 > 0.0001)
		t = t1;
	else if (t2 > 0.0001)
		t = t2;
	else
		return -1;

	intersection = r.start + r.dir * t;
	normal = intersection - center_;
	normal.normalize();

	return t;
}

// Tetrahedron implementation
Tetrahedron::Tetrahedron(Vector3 center, double size)
{
	set(center, size);
}

void Tetrahedron::set(Vector3 center, double size)
{
	double h = size * 0.816;
	vertices[0] = center + Vector3(-size * 0.5, -h * 0.3, size * 0.3);
	vertices[1] = center + Vector3(size * 0.5, -h * 0.3, size * 0.3);
	vertices[2] = center + Vector3(0, -h * 0.3, -size * 0.6);

	// Apex (top vertex)
	vertices[3] = center + Vector3(0, h * 0.7, 0);
}

double Tetrahedron::rayTriangleIntersect(Ray ray, Vector3 v0, Vector3 v1, Vector3 v2, Vector3& normal)
{
	const double EPSILON = 0.0000001;
	Vector3 edge1 = v1 - v0;
	Vector3 edge2 = v2 - v0;
	Vector3 h = cross_prod(ray.dir, edge2);
	double a = dot_prod(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return -1;

	double f = 1.0 / a;
	Vector3 s = ray.start - v0;
	double u = f * dot_prod(s, h);

	if (u < 0.0 || u > 1.0)
		return -1;

	Vector3 q = cross_prod(s, edge1);
	double v = f * dot_prod(ray.dir, q);

	if (v < 0.0 || u + v > 1.0)
		return -1;

	double t = f * dot_prod(edge2, q);

	if (t > EPSILON) {
		normal = cross_prod(edge1, edge2);
		normal.normalize();
		return t;
	}

	return -1;
}

double Tetrahedron::intersectWithRay(Ray ray, Vector3& intersection, Vector3& normal)
{
	double minT = DBL_MAX;
	Vector3 tempNormal, closestNormal;
	bool hit = false;

	double t = rayTriangleIntersect(ray, vertices[0], vertices[1], vertices[2], tempNormal);
	if (t > 0.0001 && t < minT) {
		minT = t;
		closestNormal = tempNormal;
		hit = true;
	}

	t = rayTriangleIntersect(ray, vertices[0], vertices[1], vertices[3], tempNormal);
	if (t > 0.0001 && t < minT) {
		minT = t;
		closestNormal = tempNormal;
		hit = true;
	}

	t = rayTriangleIntersect(ray, vertices[1], vertices[2], vertices[3], tempNormal);
	if (t > 0.0001 && t < minT) {
		minT = t;
		closestNormal = tempNormal;
		hit = true;
	}

	t = rayTriangleIntersect(ray, vertices[2], vertices[0], vertices[3], tempNormal);
	if (t > 0.0001 && t < minT) {
		minT = t;
		closestNormal = tempNormal;
		hit = true;
	}

	if (hit) {
		intersection = ray.start + ray.dir * minT;
		normal = closestNormal;
		return minT;
	}

	return -1;
}


void rayTrace(Ray ray, double& r, double& g, double& b, int fromObj = -1, int level = 0)
{
	// Step 4: Recursive Ray Tracing

	int goBackGround = 1, i = 0;

	Vector3 intersection, normal;
	Vector3 lightV;
	Vector3 viewV;
	Vector3 lightReflectionV;
	Vector3 rayReflectionV;

	Ray newRay;
	double mint = DBL_MAX, t;


	// Step 2: Find closest intersection among all objects
	int closestObj = -1;
	Vector3 closestIntersection, closestNormal;

	for (i = 0; i < NUM_OBJECTS; i++)
	{
		if (i == fromObj) continue;

		if ((t = objList[i]->intersectWithRay(ray, intersection, normal)) > 0)
		{
			if (t < mint)
			{
				mint = t;
				closestObj = i;
				closestIntersection = intersection;
				closestNormal = normal;
			}
		}
	}

	if (closestObj != -1)
	{
		intersection = closestIntersection;
		normal = closestNormal;

		// Step 3: Phong Illumination Model
		double u, v;
		objList[closestObj]->getTextureCoords(intersection, u, v);

		if (objList[closestObj]->hasBumpMap) {
			normal = perturbNormal(objList[closestObj], normal, intersection, u, v);
		}

		// Step 3: Phong Illumination Model
		// Calculate light vector
		lightV = lightPos - intersection;
		double distanceToLight = lightV.length();
		lightV.normalize();

		// Calculate view vector
		viewV = ray.start - intersection;
		viewV.normalize();

		// Calculate reflection vector for specular
		// R = 2(N·L)N - L
		double NdotL = dot_prod(normal, lightV);
		lightReflectionV = normal * (2.0 * NdotL) - lightV;
		lightReflectionV.normalize();

		// 1. Ambient component (always present, even in shadow)
		double ambientR = ambiantLight[0] * objList[closestObj]->ambiantReflection[0];
		double ambientG = ambiantLight[1] * objList[closestObj]->ambiantReflection[1];
		double ambientB = ambiantLight[2] * objList[closestObj]->ambiantReflection[2];

		bool inShadow = false;

		Ray shadowRay;
		shadowRay.start = intersection + normal * 0.001;
		shadowRay.dir = lightV;

		// Check if any object blocks the light
		Vector3 shadowIntersection, shadowNormal;
		for (i = 0; i < NUM_OBJECTS; i++)
		{
			double shadowT = objList[i]->intersectWithRay(shadowRay, shadowIntersection, shadowNormal);
			if (shadowT > 0.0001 && shadowT < distanceToLight)
			{
				inShadow = true;
				break;
			}
		}

		double diffuseR = 0, diffuseG = 0, diffuseB = 0;
		double specularR = 0, specularG = 0, specularB = 0;

		// Only calculate diffuse and specular if NOT in shadow
		if (!inShadow)
		{
			// 2. Diffuse component
			double diffuseFactor = max(0.0, NdotL);
			if (objList[closestObj]->hasTexture) {
				double texR, texG, texB;
				sampleTexture(objList[closestObj], u, v, texR, texG, texB);
				diffuseR = diffusetLight[0] * texR * diffuseFactor;
				diffuseG = diffusetLight[1] * texG * diffuseFactor;
				diffuseB = diffusetLight[2] * texB * diffuseFactor;

				ambientR = ambiantLight[0] * objList[closestObj]->ambiantReflection[0] * texR;
				ambientG = ambiantLight[1] * objList[closestObj]->ambiantReflection[1] * texG;
				ambientB = ambiantLight[2] * objList[closestObj]->ambiantReflection[2] * texB;
			}
			else {
				diffuseR = diffusetLight[0] * objList[closestObj]->diffusetReflection[0] * diffuseFactor;
				diffuseG = diffusetLight[1] * objList[closestObj]->diffusetReflection[1] * diffuseFactor;
				diffuseB = diffusetLight[2] * objList[closestObj]->diffusetReflection[2] * diffuseFactor;
			}

			// 3. Specular component
			double RdotV = dot_prod(lightReflectionV, viewV);
			double specularFactor = pow(max(0.0, RdotV), objList[closestObj]->speN);
			specularR = specularLight[0] * objList[closestObj]->specularReflection[0] * specularFactor;
			specularG = specularLight[1] * objList[closestObj]->specularReflection[1] * specularFactor;
			specularB = specularLight[2] * objList[closestObj]->specularReflection[2] * specularFactor;
		}


		r = ambientR + diffuseR + specularR;
		g = ambientG + diffuseG + specularG;
		b = ambientB + diffuseB + specularB;

		double specularSum = objList[closestObj]->specularReflection[0] +
			objList[closestObj]->specularReflection[1] +
			objList[closestObj]->specularReflection[2];

		if (specularSum > 0.0 && level < MAX_RT_LEVEL)
		{

			double DdotN = dot_prod(ray.dir, normal);
			rayReflectionV = ray.dir - normal * (2.0 * DdotN);
			rayReflectionV.normalize();


			newRay.start = intersection + normal * 0.001;
			newRay.dir = rayReflectionV;


			double reflectR, reflectG, reflectB;
			rayTrace(newRay, reflectR, reflectG, reflectB, closestObj, level + 1);

			r += objList[closestObj]->specularReflection[0] * reflectR;
			g += objList[closestObj]->specularReflection[1] * reflectG;
			b += objList[closestObj]->specularReflection[2] * reflectB;
		}

		if (objList[closestObj]->transparency > 0.0 && level < MAX_RT_LEVEL)
		{
			double eta = 1.0 / objList[closestObj]->refractiveIndex;

			double cosI = dot_prod(normal, ray.dir);
			Vector3 refractNormal = normal;
			if (cosI > 0)
			{
				refractNormal = normal * -1.0;
				eta = objList[closestObj]->refractiveIndex;
			}

			Vector3 refractDir = refract(ray.dir, refractNormal, eta);

			if (refractDir.length() > 0.0001)
			{
				refractDir.normalize();

				newRay.start = intersection - refractNormal * 0.001;
				newRay.dir = refractDir;

				double refractR, refractG, refractB;
				rayTrace(newRay, refractR, refractG, refractB, closestObj, level + 1);

				double trans = objList[closestObj]->transparency;
				r = r * (1.0 - trans) + refractR * trans;
				g = g * (1.0 - trans) + refractG * trans;
				b = b * (1.0 - trans) + refractB * trans;
			}
		}

		r = min(1.0, max(0.0, r));
		g = min(1.0, max(0.0, g));
		b = min(1.0, max(0.0, b));

		goBackGround = 0;
	}

	if (goBackGround)
	{
		r = bgColor[0];
		g = bgColor[1];
		b = bgColor[2];
	}

}


void drawInPixelBuffer(int x, int y, double r, double g, double b)
{
	pixelBuffer[(y * WINWIDTH + x) * 3] = (float)r;
	pixelBuffer[(y * WINWIDTH + x) * 3 + 1] = (float)g;
	pixelBuffer[(y * WINWIDTH + x) * 3 + 2] = (float)b;
}

void renderScene()
{
	int x, y;
	Ray ray;
	double r, g, b;

	cout << "Rendering Scene " << sceneNo << " with resolution " << WINWIDTH << "x" << WINHEIGHT << "........... ";
	__int64 time1 = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count(); // marking the starting time

	ray.start = cameraPos;

	Vector3 vpCenter = cameraPos + lookAtDir * focalLen;  // viewplane center
	Vector3 startingPt = vpCenter + leftVector * (-WINWIDTH / 2.0) + upVector * (-WINHEIGHT / 2.0);
	Vector3 currPt;

	for (x = 0; x < WINWIDTH; x++)
		for (y = 0; y < WINHEIGHT; y++)
		{
			currPt = startingPt + leftVector * x + upVector * y;
			ray.dir = currPt - cameraPos;
			ray.dir.normalize();
			rayTrace(ray, r, g, b);
			drawInPixelBuffer(x, y, r, g, b);
		}

	__int64 time2 = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count(); // marking the ending time

	cout << "Done! \nRendering time = " << time2 - time1 << "ms" << endl << endl;
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_DOUBLEBUFFER);
	glDrawPixels(WINWIDTH, WINHEIGHT, GL_RGB, GL_FLOAT, pixelBuffer);
	glutSwapBuffers();
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


void setScene(int i = 0)
{
	if (i > NUM_SCENE)
	{
		cout << "Warning: Invalid Scene Number" << endl;
		return;
	}

	for (int j = 0; j < NUM_OBJECTS; j++) {
		if (objList[j]->textureData != nullptr) {
			stbi_image_free(objList[j]->textureData);
			objList[j]->textureData = nullptr;
		}
		objList[j]->hasTexture = false;
		objList[j]->hasBumpMap = false;
	}

	if (i == 0)
	{

		((Sphere*)objList[0])->set(Vector3(-130, 80, 120), 100);
		((Sphere*)objList[1])->set(Vector3(130, -80, -80), 100);
		((Sphere*)objList[2])->set(Vector3(-130, -80, -80), 100);
		((Sphere*)objList[3])->set(Vector3(130, 80, 120), 100);
		((Tetrahedron*)objList[4])->set(Vector3(0, -1000, 0), 1); //hided tetrahedron

		objList[0]->ambiantReflection[0] = 0.1;
		objList[0]->ambiantReflection[1] = 0.4;
		objList[0]->ambiantReflection[2] = 0.4;
		objList[0]->diffusetReflection[0] = 0;
		objList[0]->diffusetReflection[1] = 1;
		objList[0]->diffusetReflection[2] = 1;
		objList[0]->specularReflection[0] = 0.2;
		objList[0]->specularReflection[1] = 0.4;
		objList[0]->specularReflection[2] = 0.4;
		objList[0]->speN = 300;

		objList[1]->ambiantReflection[0] = 0.6;
		objList[1]->ambiantReflection[1] = 0.6;
		objList[1]->ambiantReflection[2] = 0.2;
		objList[1]->diffusetReflection[0] = 1;
		objList[1]->diffusetReflection[1] = 1;
		objList[1]->diffusetReflection[2] = 0;
		objList[1]->specularReflection[0] = 0.0;
		objList[1]->specularReflection[1] = 0.0;
		objList[1]->specularReflection[2] = 0.0;
		objList[1]->speN = 50;

		objList[2]->ambiantReflection[0] = 0.1;
		objList[2]->ambiantReflection[1] = 0.6;
		objList[2]->ambiantReflection[2] = 0.1;
		objList[2]->diffusetReflection[0] = 0.1;
		objList[2]->diffusetReflection[1] = 1;
		objList[2]->diffusetReflection[2] = 0.1;
		objList[2]->specularReflection[0] = 0.3;
		objList[2]->specularReflection[1] = 0.7;
		objList[2]->specularReflection[2] = 0.3;
		objList[2]->speN = 650;
		objList[2]->transparency = 0.0;

		objList[3]->ambiantReflection[0] = 0.3;
		objList[3]->ambiantReflection[1] = 0.3;
		objList[3]->ambiantReflection[2] = 0.3;
		objList[3]->diffusetReflection[0] = 0.7;
		objList[3]->diffusetReflection[1] = 0.7;
		objList[3]->diffusetReflection[2] = 0.7;
		objList[3]->specularReflection[0] = 0.6;
		objList[3]->specularReflection[1] = 0.6;
		objList[3]->specularReflection[2] = 0.6;
		objList[3]->speN = 650;

		// Tetrahedron - Purple/Magenta
		objList[4]->ambiantReflection[0] = 0.5;
		objList[4]->ambiantReflection[1] = 0.2;
		objList[4]->ambiantReflection[2] = 0.5;
		objList[4]->diffusetReflection[0] = 1.0;
		objList[4]->diffusetReflection[1] = 0.3;
		objList[4]->diffusetReflection[2] = 1.0;
		objList[4]->specularReflection[0] = 0.4;
		objList[4]->specularReflection[1] = 0.4;
		objList[4]->specularReflection[2] = 0.4;
		objList[4]->speN = 400;

	}

	if (i == 1)
	{

		// Step 5
		((Sphere*)objList[0])->set(Vector3(-150, 50, 150), 150);
		((Sphere*)objList[1])->set(Vector3(130, 10, 150), 100);
		((Sphere*)objList[2])->set(Vector3(20, -10, -80), 40);
		((Sphere*)objList[3])->set(Vector3(0, -850, 450), 700);
		((Tetrahedron*)objList[4])->set(Vector3(-140, -40, -100), 80);

		objList[0]->ambiantReflection[0] = 0.1;
		objList[0]->ambiantReflection[1] = 0.4;
		objList[0]->ambiantReflection[2] = 0.4;
		objList[0]->diffusetReflection[0] = 0;
		objList[0]->diffusetReflection[1] = 1;
		objList[0]->diffusetReflection[2] = 1;
		objList[0]->specularReflection[0] = 0.2;
		objList[0]->specularReflection[1] = 0.4;
		objList[0]->specularReflection[2] = 0.4;
		objList[0]->speN = 300;

		objList[1]->ambiantReflection[0] = 0.6;
		objList[1]->ambiantReflection[1] = 0.6;
		objList[1]->ambiantReflection[2] = 0.2;
		objList[1]->diffusetReflection[0] = 1;
		objList[1]->diffusetReflection[1] = 1;
		objList[1]->diffusetReflection[2] = 0;
		objList[1]->specularReflection[0] = 0.0;
		objList[1]->specularReflection[1] = 0.0;
		objList[1]->specularReflection[2] = 0.0;
		objList[1]->speN = 50;

		objList[2]->ambiantReflection[0] = 0.6;
		objList[2]->ambiantReflection[1] = 0.6;
		objList[2]->ambiantReflection[2] = 0.6;
		objList[2]->diffusetReflection[0] = 0.1;
		objList[2]->diffusetReflection[1] = 1;
		objList[2]->diffusetReflection[2] = 0.1;
		objList[2]->specularReflection[0] = 0.7;
		objList[2]->specularReflection[1] = 0.7;
		objList[2]->specularReflection[2] = 0.7;
		objList[2]->speN = 1000;
		// extra feature: make this sphere transparent
		objList[2]->transparency = 0.5;
		objList[2]->refractiveIndex = 1.15;

		objList[3]->ambiantReflection[0] = 0.3;
		objList[3]->ambiantReflection[1] = 0.3;
		objList[3]->ambiantReflection[2] = 0.3;
		objList[3]->diffusetReflection[0] = 0.7;
		objList[3]->diffusetReflection[1] = 0.7;
		objList[3]->diffusetReflection[2] = 0.7;
		objList[3]->specularReflection[0] = 0.6;
		objList[3]->specularReflection[1] = 0.6;
		objList[3]->specularReflection[2] = 0.6;
		objList[3]->speN = 650;

		// Tetrahedron - Purple/Magenta
		objList[4]->ambiantReflection[0] = 0.5;
		objList[4]->ambiantReflection[1] = 0.2;
		objList[4]->ambiantReflection[2] = 0.5;
		objList[4]->diffusetReflection[0] = 1.0;
		objList[4]->diffusetReflection[1] = 0.3;
		objList[4]->diffusetReflection[2] = 1.0;
		objList[4]->specularReflection[0] = 0.4;
		objList[4]->specularReflection[1] = 0.4;
		objList[4]->specularReflection[2] = 0.4;
		objList[4]->speN = 400;

		if (loadTexture(objList[0], "../texture.jpg")) {
			objList[0]->hasBumpMap = true;
			objList[0]->bumpStrength = 2.0;
		}

	}
}

void keyboard(unsigned char key, int x, int y)
{
	//keys to control scaling - k
	//keys to control rotation - alpha
	//keys to control translation - tx, ty
	switch (key) {
	case 's':
	case 'S':
		sceneNo = (sceneNo + 1) % NUM_SCENE;
		setScene(sceneNo);
		renderScene();
		glutPostRedisplay();
		break;
	case 'q':
	case 'Q':
		exit(0);

	default:
		break;
	}
}

int main(int argc, char** argv)
{

	cout << "<<CS3241 Lab 5>>\n\n" << endl;
	cout << "S to go to next scene" << endl;
	cout << "Q to quit" << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINWIDTH, WINHEIGHT);

	glutCreateWindow("CS3241 Lab 5: Ray Tracing");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutKeyboardFunc(keyboard);

	objList = new RtObject * [NUM_OBJECTS];

	// create four spheres
	objList[0] = new Sphere(Vector3(-130, 80, 120), 100);
	objList[1] = new Sphere(Vector3(130, -80, -80), 100);
	objList[2] = new Sphere(Vector3(-130, -80, -80), 100);
	objList[3] = new Sphere(Vector3(130, 80, 120), 100);

	// extra feature: create a tetrahedron
	objList[4] = new Tetrahedron(Vector3(0, 0, 0), 100);

	setScene(0);
	setScene(sceneNo);
	renderScene();

	glutMainLoop();

	for (int i = 0; i < NUM_OBJECTS; i++)
		delete objList[i];

	delete[] objList;

	delete[] pixelBuffer;

	return 0;
}
