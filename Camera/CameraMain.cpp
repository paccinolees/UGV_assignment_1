#include <zmq.hpp>
#include <Windows.h>
#include <iostream>

#include "SMStructs.h"
#include "SMFcn.h"
#include "SMObject.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <turbojpeg.h>

using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec

// Instantiate SM Objects
SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));

// Allocate PM. pointer to pData
ProcessManagement* PMptr;

void display();
void idle();

GLuint tex;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

int main(int argc, char** argv)
{
	// Give access to SM objects and check if there are errors
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	PMptr = (ProcessManagement*)PMObj.pData;

	// Initialize shutdown status
	PMptr->Shutdown.Flags.Camera = 0;


	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	glutMainLoop();
	
	return 0;
}


void display()
{
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle()
{
	// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
	if (PMptr->Heartbeat.Flags.Camera == 0) {
		PMptr->Heartbeat.Flags.Camera = 1;
		PMCounter = 0;
	}
	else {
		PMCounter += 1;

		if (PMCounter > max_waitCount) {
			std::cout << "PM failed, sending shutdown signal..." << std::endl;
			PMptr->Shutdown.Status = 0xFF;
		}
	}

	// Close the Display Module if shutdown flag is set
	if (PMptr->Shutdown.Flags.Camera)
	{
		std::cout << "Camera process terminating..." << std::endl;
		exit(1);
	}


	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char* buffer = new unsigned char[width * height * 3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}

	display();

	Thread::Sleep(25);
}

