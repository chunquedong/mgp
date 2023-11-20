#include "Renderer.h"
//#include "GLRenderer.h"

using namespace mgp;

Renderer* g_rendererInstance;

Renderer* Renderer::cur() {
	/*if (g_rendererInstance == NULL) {
		g_rendererInstance = new GLRenderer();
	}*/
	return g_rendererInstance;
}

void Renderer::finalize() {
    delete g_rendererInstance;
    g_rendererInstance = NULL;
}
