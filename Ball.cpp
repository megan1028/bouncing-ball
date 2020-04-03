
/*
 * reference: https://www.ntu.edu.sg/home/ehchua/programming/opengl/CG_Introduction.html
 */
#include <windows.h>  // for MS Windows
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <Math.h>     // Needed for sin, cos
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <iostream>
#include <thread>
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/tick_count.h"
using namespace tbb;
using namespace std;
#define PI 3.14159265f


float WINDOW_SIZE = 700;

int const NUM = 3;			//total number of circles (10 by default)
float radius = 0.1;			//circle radius
int waitTime = 33;			//milliseconds between steps
//coordinates
float x[NUM];
float y[NUM];
//velocity
float vx[NUM];
float vy[NUM];
//new velocity
float newvx[NUM];
float newvy[NUM];

//gravity
float g = 9.8;

const size_t vec_size = 1000000;
const size_t grain_size = 1000;
float glowAlph[NUM];


//creates a gl triangle fan circle of indicated radius and segments
void flatCircle(float cx, float cy, float radius, int segments) {
	float phi, x1, y1;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy);					//center vertex
	for (int j = 0; j <= segments; j++) {	//for every segment,
		phi = 2 * PI * j / segments;	//calculate the new vertex
		x1 = radius * cos(phi) + cx;
		y1 = radius * sin(phi) + cy;
		glVertex2f(x1, y1);
	}
	glEnd();
} //end circle


//initializes all circle posiitons, colors, and velocities
void initCircles(void) {
	srand(time(NULL));							// seed the random number generator
	for (int i = 0; i < NUM; i++) {				// for each circle,
		//current position
		x[i] = ((rand() % (int)(200 - (radius * 200))) / 100.0) - (1.0 - radius); //  random number between
		y[i] = ((rand() % (int)(200 - (radius * 200))) / 100.0) - (1.0 - radius); //    -0.9 and 0.9 (to account for radius size)

		//velocity
		vx[i] = ((rand() % 200) / 10000.0) - 0.01; 	//	random velocities between
		vy[i] = ((rand() % 200) / 10000.0) - 0.01; 	//	  -0.02 and 0.02

		glowAlph[i] = 0.0;
	}
}

class vector_mult {
	float  *v1, *v2, *v3;    // multiply these vectors
	                   // put the result here
public:
	//// constructor copies the arguments into local storage
	vector_mult(float *vec1, float *vec2)
		: v1(vec1), v2(vec1){ }
	//// overload () so it does a vector multiply
	void operator() (const blocked_range<int> &r) const {
		for (int i = r.begin(); i != r.end(); i++)
		{
			glColor3f(1.0, 0.0, 0.0);
			flatCircle(v1[i] + v1[i] / 15, v2[i] + (v2[i] - 1.0) / 20, radius, 30);
		}

	}
};

/* Callback handler for window re-paint event */
void display() {

	glClear(GL_COLOR_BUFFER_BIT);  // Clear the color buffer
	float *v1, *v2, *v3;
	v1 = (float*)malloc(vec_size * sizeof(float));
	v2 = (float*)malloc(vec_size * sizeof(float));
	v3 = (float*)malloc(vec_size * sizeof(float));

	for (int i = 0; i < NUM; i++) {
			v1[i] == x[i];
			v2[i] == y[i];
	}

	task_scheduler_init init;
	parallel_for(blocked_range<int>(0, NUM), vector_mult(v1, v2), simple_partitioner());

	glFlush();
}


/* Call back when the windows is re-sized */
void reshape(int w, int h) {
	float aspectRatio = 1.0;

	//Compute the aspect ratio of the resized window
	aspectRatio = (float)h / (float)w;

	// Adjust the clipping box
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (h >= w)
		gluOrtho2D(-1.0, 1.0, -aspectRatio, aspectRatio);
	else
		gluOrtho2D(-1.0 / aspectRatio, 1.0 / aspectRatio, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	//adjust the viewport
	glViewport(0, 0, w, h);
}


/* Called back when the timer expired */
void timer(int value) {
	//Actually move the circles
	for (int i = 0; i < NUM; i++) {
		x[i] += vx[i];
		y[i] += vy[i];
	}
	for (int i = 0; i < NUM; i++) {
		vy[i] = vy[i] - g * 0.0001 * value;
	}

	//resolve collisions
	for (int i = 0; i < NUM; i++) {	//for each ball,
		// Reverse direction when you reach edges
		if (x[i] > 1.0 - radius) {		//right edge
			x[i] = 1.0 - radius;				//to prevent balls from sticking
			vx[i] = -vx[i];					//change velocity

		}
		else if (x[i] < -1.0 + radius) {	//left edge
			x[i] = -1.0 + radius;				///to prevent balls from sticking
			vx[i] = -vx[i];					//change velocity

		}

		if (y[i] > 1.0 - radius) {		//top edge
			y[i] = 1.0 - radius;			//to prevent balls from sticking
			vy[i] = -vy[i];					//change velocity

		}
		else if (y[i] < -1.0 + radius) {	//bottom edge
			y[i] = -1.0 + radius;			//to prevent balls from sticking
			vy[i] = -vy[i];					//change velocity

		}
	}

	glutPostRedisplay();
	glutTimerFunc(waitTime, timer, 1);
}



/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char **argv) {
	initCircles();		//initialize circle values

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA | GLUT_ALPHA);

	glutInitWindowPosition(0, 0);					//window position
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);	//window size
	glutCreateWindow("Bouncing balls");				//window name
	glClearColor(0.0, 0.0, 0.0, 0.0);				//background color
	glClear(GL_COLOR_BUFFER_BIT);

	//The four following statements set up the viewing rectangle
	glMatrixMode(GL_PROJECTION);					// use proj. matrix
	glLoadIdentity();								// load identity matrix
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);				// set orthogr. proj.
	glMatrixMode(GL_MODELVIEW);						// back to modelview m.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutDisplayFunc(display);
	glutTimerFunc(waitTime, timer, 1);
	glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;

}
