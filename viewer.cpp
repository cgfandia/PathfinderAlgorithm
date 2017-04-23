#include <iostream>
#include "FPGA.h"
#include <GL/glut.h>

#ifndef __GNUC__
#pragma comment(lib, "glut.lib")
#pragma comment(lib, "glut32.lib")
#endif

extern PATHFINDER fpga;
int mainWindow;
void initGL() {
	// Set "clearing" or background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black and opaque
}
void updateFPGA(){
	glutSetWindow(mainWindow);
	glutPostRedisplay();
}

inline void channelsGradientBtoR(const float& currentWeight, const float& maxWeight)
{
	float ratio = currentWeight / maxWeight;
	float RGB[3];
	if (ratio < 0.25f){
		RGB[0] = 0;
		RGB[1] = 4 * ratio;
		RGB[2] = 1;
	}
	else if (ratio >= 0.25f && ratio < 0.5f){
		RGB[0] = 0;
		RGB[1] = 1;
		RGB[2] = 2 - ratio * 4;
	}
	else if (ratio >= 0.5f && ratio < 0.75f){
		RGB[0] = 1.3333f * ratio;
		RGB[1] = 1;
		RGB[2] = 0;
	}
	else if (ratio >= 0.75f && ratio <= 1.0f){
		RGB[0] = 1;
		RGB[1] = 4 - 4 * ratio;
		RGB[2] = 0;
	}
	else if (ratio > 1.0f){
		RGB[0] = 1;
		RGB[1] = 0;
		RGB[2] = 0;
	}
	else{
		RGB[0] = 0;
		RGB[1] = 0;
		RGB[2] = 1;
	}

	glColor3f(RGB[0], RGB[1], RGB[2]);
}

inline void channelsGradientWarm(const float& currentWeight, const float& maxWeight)
{
	float ratio = currentWeight / maxWeight;
	float RGB[3];

	if (ratio < 1.0f / 3){
		RGB[0] = 3 * ratio;
		RGB[1] = 0;
		RGB[2] = 0;
	}
	else if (ratio >= 1.0f / 3 && ratio < 2.0f / 3){
		RGB[0] = 1;
		RGB[1] = 3 * ratio;
		RGB[2] = 0;
	}
	else if (ratio >= 2.0f / 3 && ratio < 1){
		RGB[0] = 1;
		RGB[1] = 1;
		RGB[2] = 3 * ratio;
	}
	else if (ratio > 1.0f){
		RGB[0] = 1;
		RGB[1] = 1;
		RGB[2] = 1;
	}
	else{
		RGB[0] = 0;
		RGB[1] = 0;
		RGB[2] = 0;
	}

	glColor3f(RGB[0], RGB[1], RGB[2]);
}

/* Handler for window re-size event. Called back when the window first appears and
whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
	// Compute aspect ratio of the new window
	if (height == 0) height = 1;                // To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping area to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset the projection matrix
	if (width >= height) {
		// aspect >= 1, set the height from -1 to 1, with larger width
		gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
	}
	else {
		// aspect < 1, set the width to -1 to 1, with larger height
		gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
	}
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT);   // Clear the color buffer with current clearing color

	for (size_t i = 0; i < fpga.blocksCount; i++)
	{
		float div = fpga.maxWH / 1.8;
		float currentBlockX = (fpga.blocksArray[i].coords[0]) / div - 0.9;
		float currentBlockY = (fpga.blocksArray[i].coords[1]) / div - 0.9;
		float wh = 0.5f / div;
		glBegin(GL_QUADS);
		if (fpga.blocksArray[i].type == blockType::CLB)
		{
			glColor3f(0.0f, 0.0f, 0.0f);
		}
		else if (fpga.blocksArray[i].type == blockType::INPUT){
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		glVertex2f(currentBlockX - wh, currentBlockY - wh);
		glVertex2f(currentBlockX + wh, currentBlockY - wh);
		glVertex2f(currentBlockX + wh, currentBlockY + wh);
		glVertex2f(currentBlockX - wh, currentBlockY + wh);
		glEnd();
	}

	for (size_t i = 0; i < fpga.channels2DArrayWH; i++)
	{
		for (size_t j = 0; j < fpga.channels2DArrayWH; j++)
		{
			if (fpga.channels2DArray[i][j] != nullptr)
			{
				float div = fpga.maxWH / 1.8;
				float currentBlockX = (fpga.channels2DArray[i][j]->coords[0]) / div - 0.9;
				float currentBlockY = (fpga.channels2DArray[i][j]->coords[1]) / div - 0.9;
				float wh = 0.5f / div;
				glBegin(GL_QUADS);
				channelsGradientBtoR(fpga.channels2DArray[i][j]->channelOccupancy, fpga.currentMaxOccupancy);
				glVertex2f(currentBlockX - wh, currentBlockY - wh);
				glVertex2f(currentBlockX + wh, currentBlockY - wh);
				glVertex2f(currentBlockX + wh, currentBlockY + wh);
				glVertex2f(currentBlockX - wh, currentBlockY + wh);
				glEnd();
			}
		}
	}
	glFlush();  // Render now
}

void idleDisplay(){
	if (fpga.update) display();
}

int runViewer(int argc, char** argv){
	glutInit(&argc, argv);          // Initialize GLUT
	glutInitWindowSize(800, 800);   // Set the window's initial width & height - non-square
	glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
	mainWindow = glutCreateWindow("Viewport Transform");  // Create window with the given title
	glutDisplayFunc(display);       // Register callback handler for window re-paint event
	glutIdleFunc(idleDisplay);       // Register callback handler for window re-paint event
	glutReshapeFunc(reshape);       // Register callback handler for window re-size event
	initGL();                       // Our own OpenGL initialization
	glutMainLoop();                 // Enter the infinite event-processing loop
	return 0;
}