#include "Renderer.h"
//#include "GLRenderer.h"
#include "platform/Toolkit.h"

using namespace mgp;

Renderer* g_rendererInstance;

Renderer* Renderer::cur() {
	/*if (g_rendererInstance == NULL) {
		g_rendererInstance = new GLRenderer();
	}*/
	return g_rendererInstance;
}

unsigned int Renderer::getDpWidth() { return (unsigned int)(getWidth() / Toolkit::cur()->getScreenScale()); }
unsigned int Renderer::getDpHeight() { return (unsigned int)(getHeight() / Toolkit::cur()->getScreenScale()); }

void Renderer::finalize() {
    delete g_rendererInstance;
    g_rendererInstance = NULL;
}
