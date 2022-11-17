//#include "InputManager.h"
#include "display.h"
#include "game.h"
#include "glad/include/glad/glad.h"


int main(int argc, char* argv[])
{
	const int DISPLAY_WIDTH = 512; //1600;
	const int DISPLAY_HEIGHT = 512; //800;
	const float CAMERA_ANGLE = 0.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game* scn = new Game(CAMERA_ANGLE, (float)DISPLAY_WIDTH / DISPLAY_HEIGHT, NEAR, FAR);

	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

	//Init(display);

	scn->Init();

	display.SetScene(scn);

	int displaywidth = DISPLAY_WIDTH / 2;

	// texture 0
	scn->SetShapeTex(0, 0);
	glViewport(0, displaywidth, displaywidth, displaywidth);
	scn->Draw(1, 0, scn->BACK, true, false);

	// texture 1
	scn->SetShapeTex(0, 1);
	glViewport(displaywidth, displaywidth, displaywidth, displaywidth);
	scn->Draw(1, 0, scn->BACK, false, false);

	// texture 2
	scn->SetShapeTex(0, 2);
	glViewport(0, 0, displaywidth, displaywidth);
	scn->Draw(1, 0, scn->BACK, false, false);

	// texture 3
	scn->SetShapeTex(0, 3);
	glViewport(displaywidth, 0, displaywidth, displaywidth);
	scn->Draw(1, 0, scn->BACK, false, false);

	display.SwapBuffers();

	while (!display.CloseWindow())
		display.PollEvents();

	delete scn;
	return 0;
}