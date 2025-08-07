
#include "WaseWindow.h"

#if __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include "WebTextInput.h"
#endif

#include "openGL/ogl.h"

#ifdef _WIN32
	#include "Win32TextInput.h"
#endif

#include "nanovg.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include "nanovg_gl.h"

#include "NanovgGraphics.h"
#include "Window.h"

#include "app/Platform.h"

using namespace waseGraphics;
using namespace mgp;

extern float g_screenScle;
extern bool g_autoScale;

#ifdef _WIN32
	std::string LocalToUTF8(const std::string& localStr);
	LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

namespace mgp {
    HWND getWin32Window();
}

class MgpWindow : public Window
{
public:
	//GLFWwindow* window = NULL;
	NVGcontext* vg = NULL;
	Graphics* graphics = NULL;
	sric::OwnPtr<waseGraphics::View> _view;
	Size _size;
	sric::OwnPtr<TextInput> _textInput;

	void init(NVGcontext* vg, sric::OwnPtr<waseGraphics::View> view) SC_NOTHROW {
		//this->window = window;
		this->vg = vg;
		graphics = waseGraphics::createNanovgGraphics(vg);
		_view = std::move(view);
		sric::RefPtr<Window> self = sric::rawToRef((Window*)this);
		_view->setHost(self);
	}

	sric::RefPtr<View> view() SC_NOTHROW {
		return _view;
	}

	void repaint(Rect& dirty) SC_NOTHROW {

	}

	Size size() SC_NOTHROW {
		return _size;
	}

	bool hasFocus() SC_NOTHROW {
		return true;
	}

	void focus() SC_NOTHROW {

	}

	sric::OwnPtr<TextInput> textInput(int inputType) SC_NOTHROW {
#ifdef _WIN32
		sric::OwnPtr<Win32TextInput> textInput = sric::new_<Win32TextInput>();
		textInput->custemProc = (LONG_PTR)EditSubclassProc;
		HWND hwnd = mgp::getWin32Window();
		textInput->init(hwnd, inputType);
		_textInput = std::move(textInput);
		return _textInput.share();
#elif defined(__EMSCRIPTEN__)
		sric::OwnPtr<WebTextInput> textInput = sric::new_<WebTextInput>();
		textInput->init(inputType);
		_textInput = std::move(textInput);
		return _textInput.share();
#else
		return sric::OwnPtr<TextInput>();
#endif
	}

	void fileDialog(bool isOpen, const char* accept) SC_NOTHROW {

	}

	void displayKeyboard(bool display) SC_NOTHROW {

	}

	void onResize(int w, int h) SC_NOTHROW {
		_size.w = w;
		_size.h = h;
	}
};

sric::OwnPtr<MgpWindow> g_window;
NVGcontext* g_vg = NULL;


int Window::open(sric::OwnPtr<waseGraphics::View> view, const char* name) SC_NOTHROW {
	if (!g_window.isNull()) {
		return -1;
	}
	//auto size = view->getPrefSize(1024, 768);
	g_window = sric::new_<MgpWindow>();
	//return openWindow(name, size.w, size.h, std::move(view));
	if (g_vg == NULL) {
		printf("Could not found nanovg.\n");
		return -1;
	}
	g_window->init(g_vg, std::move(view));
	return 0;
}

sric::RefPtr<Window> Window::getCur() SC_NOTHROW {
	return g_window;
}

#ifdef _WIN32
extern WNDPROC oldEditProc;

LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = CallWindowProc(oldEditProc, hWnd, uMsg, wParam, lParam);
	static char lastText[1024] = { 0 };
	if (uMsg == WM_CHAR || uMsg == WM_KEYDOWN ||
		uMsg == WM_PASTE || uMsg == WM_CUT ||
		uMsg == WM_CLEAR || uMsg == WM_SETTEXT)
	{
		if (g_window->_textInput != nullptr) {
			Win32TextInput* textInput = dynamic_cast<Win32TextInput*>(g_window->_textInput.get());
			if (textInput && textInput->textInputHandle) {
				char result[256];
				if (GetWindowTextA(textInput->textInputHandle, result, 256)) {
					std::string utf8 = LocalToUTF8(result);
					g_window->_textInput->onTextChange(utf8.c_str());
				}
			}
		}
	}

	return res;
}
#endif



#ifdef __EMSCRIPTEN__
EM_JS(bool, isMobile, (), {
	if (/Mobi|Android|iPhone/i.test(navigator.userAgent)) {
		return true;
	}
	return false;
  });
#else
  bool isMobile() {
	return false;
  }
#endif


namespace waseUI {
    void init() {
		g_vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
		if (g_vg == NULL) {
			printf("Could not init nanovg.\n");
			return;
		}
		g_screenScle = mgp::Platform::cur()->getScreenScale();
		g_autoScale = isMobile();
	}

    void finalize() {
		if (g_vg) {
			nvgDeleteGLES3(g_vg);
			g_vg = NULL;
		}
	}

	bool doFrame() {
		if (g_window.isNull()) return false;

		//GLFWwindow* window = g_window->window;
		NVGcontext* vg = g_window->vg;

		//double mx, my;
		int winWidth = g_window->_size.w;
		int winHeight = g_window->_size.h;
		int fbWidth = winWidth; int fbHeight = winHeight;
		float pxRatio = 1;
		//int i, n;

		fireTimeEvents();

		// Calculate pixel ration for hi-dpi devices.
		//pxRatio = (float)fbWidth / (float)winWidth;

		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);
		glClearColor(0,0,0,0);

		//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
		
		//renderGraph(vg);
		g_window->_size.w = fbWidth;
		g_window->_size.h = fbHeight;
		g_window->paint(*g_window->graphics);

		nvgEndFrame(vg);

		return true;
	}

    void resize(int w, int h) {
		if (g_window.isNull()) return;
		g_window->onResize(w, h);
	}
    bool keyEvent(mgp::Keyboard key) {
		if (g_window.isNull()) return false;
		return false;
	}

	static bool mgpMouseToWase(mgp::Mouse evt, waseGraphics::MotionEvent& env) {
		env.type = (waseGraphics::MotionEventType)evt.type;
		env.delta = evt.wheelDelta;
		env.x = evt.x;
		env.y = evt.y;
		env.button = (waseGraphics::ButtonType)evt.button;
		env.count = evt.count;
		env.time = evt.time;
		env.pressure = evt.pressure;
		env.size = evt.size;
		return true;
	}

    bool mouseEvent(mgp::Mouse evt) {
		if (g_window.isNull()) return false;

		waseGraphics::MotionEvent env;
		mgpMouseToWase(evt, env);

		g_window->view()->onMotionEvent(env);

		return env.consumed;
	}
}