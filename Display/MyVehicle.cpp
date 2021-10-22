#include "MyVehicle.hpp"

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>

	#pragma warning(disable : 4996)
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
#endif


void drawUGV(double steerAngle)
{
	const float red = .8, green = .1, blue = .2;
	const float width = .4; //.4
	const float wheel_height = .4; //.4
	const float wheel_width = 2; //.1
	const float vertices[10][3] = {
		// left-hand side
		{ .5,   wheel_height -0.1,    width},
		{-.5,   wheel_height -0.1,    width},
		{-.5,   wheel_height +0.3,    width},
		{ .4,   wheel_height +0.3,    width},
		{ .5,   wheel_height +0.15,   width},
		// right-hand side
		{ .5,   wheel_height -0.1,   -width},
		{-.5,   wheel_height -0.1,   -width},
		{-.5,   wheel_height +0.3,   -width},
		{ .4,   wheel_height +0.3,   -width},
		{ .5,   wheel_height +0.15,  -width}
	};

	// quad faces
	glBegin(GL_QUADS);
		// bottom		
		glColor3f(red * .3, green, blue);
		glVertex3fv(vertices[0]);
		glVertex3fv(vertices[5]);
		glVertex3fv(vertices[6]);
		glVertex3fv(vertices[1]);

		// back
		glColor3f(red* .5, green, blue);
		glVertex3fv(vertices[1]);
		glVertex3fv(vertices[6]);
		glVertex3fv(vertices[7]);
		glVertex3fv(vertices[2]);

		// top
		glColor3f(red * .7, green, blue);
		glVertex3fv(vertices[2]);
		glVertex3fv(vertices[7]);
		glVertex3fv(vertices[8]);
		glVertex3fv(vertices[3]);

		// angled 
		glColor3f(red * .6, green, blue);
		glVertex3fv(vertices[3]);
		glVertex3fv(vertices[8]);
		glVertex3fv(vertices[9]);
		glVertex3fv(vertices[4]);

		// front
		glColor3f(red * .5, green, blue);
		glVertex3fv(vertices[4]);
		glVertex3fv(vertices[9]);
		glVertex3fv(vertices[5]);
		glVertex3fv(vertices[0]);
	glEnd();

	// left
	glBegin(GL_POLYGON);
		glColor3f(red, green, blue);
		glVertex3fv(vertices[0]);
		glVertex3fv(vertices[1]);
		glVertex3fv(vertices[2]);
		glVertex3fv(vertices[3]);
		glVertex3fv(vertices[4]);
	glEnd();

	// right
	glBegin(GL_POLYGON);
		glVertex3fv(vertices[5]);
		glVertex3fv(vertices[6]);
		glVertex3fv(vertices[7]);
		glVertex3fv(vertices[8]);
		glVertex3fv(vertices[9]);
	glEnd();
	/*
	glBegin(GL_QUADS);
	glColor3f(red * .3, green, blue);

	glVertex3f(0, 0, 0);// front(my mental co-or)
	glVertex3f(-5, 0, 0);
	glVertex3f(-5, 1, 0);
	glVertex3f(0, 1, 0);

	glVertex3f(0, 0, 1);// back
	glVertex3f(-5, 0, 1);
	glVertex3f(-5, 1, 1);
	glVertex3f(0, 1, 1);

	glVertex3f(0, 1, 1);// top/above
	glVertex3f(0, 1, 0);
	glVertex3f(-5, 1, 0);
	glVertex3f(-5, 1, 1);

	glEnd();
	*/
	
	
	// draw wheels
	glPushMatrix();
		glColor3f(1,1,1);
		static GLUquadric *quad = gluNewQuadric();
		glTranslatef(0, wheel_height*.5, -wheel_width*.5);

		// front
		glPushMatrix();
			glTranslatef(vertices[0][0], 0, - (width+wheel_width) );
			glPushMatrix();
				glRotatef(-steerAngle, 0, 1, 0);
				gluCylinder(quad, wheel_height*.5, wheel_height*.5, wheel_width, 12, 1);		
			glPopMatrix();
	
			glTranslatef(0, 0, (width+wheel_width) * 2);
			glPushMatrix();
				glRotatef(-steerAngle, 0, 1, 0);
				gluCylinder(quad, wheel_height*.5, wheel_height*.5, wheel_width, 12, 1);
			glPopMatrix();
		glPopMatrix();

		// back
		glPushMatrix();
			glTranslatef(vertices[1][0], 0, -(width+wheel_width));
			gluCylinder(quad, wheel_height*.5, wheel_height*.5, wheel_width, 12, 1);	

			glTranslatef(0, 0, (width+wheel_width) * 2);
			gluCylinder(quad, wheel_height*.5, wheel_height*.5, wheel_width, 12, 1);
		glPopMatrix();
	glPopMatrix();
}

MyVehicle::MyVehicle(int AmountOfRanges, double x[], double y[]) //Constructor's job is to store the laser scan's values
{
	NumOfRanges = AmountOfRanges;
	for (int i = 0; i < AmountOfRanges; i++) {
		xRange[i] = x[i];
		yRange[i] = y[i];
	}
}
void MyVehicle::drawLaserScans()
{
	//glPushMatrix();
	//glTranslatef(0.5, 0.3, 0); ??
	glColor3f(1, 1, 1);
	//glLineWidth(1.5);
	for (int i = 0; i < NumOfRanges; i++) {
		glBegin(GL_LINES);
		glVertex3f(xRange[i] / 1000, 0, -yRange[i]/ 1000);
		glVertex3f(xRange[i] / 1000, 1, -yRange[i] / 1000);
		glEnd();
	}
	//glPopMatrix();
}
void MyVehicle::draw()
{
	glPushMatrix();
	positionInGL();
	
	drawUGV(steering);
	drawLaserScans();

	glPopMatrix();
}
