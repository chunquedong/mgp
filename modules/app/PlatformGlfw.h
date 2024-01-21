#ifndef PLATFORM_GLFW_H_
#define PLATFORM_GLFW_H_

#include "Platform.h"

namespace mgp
{
class PlatformGlfw : public Platform {
public:
    PlatformGlfw();
    
    virtual void init(const char* title, int w, int h) override;
    virtual int enterMessagePump() override;
    virtual void swapBuffers() override;

    virtual void requestRepaint() override;
    virtual void signalShutdown() override;
    virtual bool canExit() override;
    virtual unsigned int getDisplayWidth();
    virtual unsigned int getDisplayHeight();
    virtual float getScreenScale() override;
    virtual bool isVsync() override;
    virtual void setVsync(bool enable) override;
    virtual void setMultiSampling(bool enabled) override;
    virtual bool isMultiSampling() override;
    virtual bool hasMouse() override;
    virtual void setMouseCaptured(bool captured) override;
    virtual bool isMouseCaptured() override;
    virtual void setCursorVisible(bool visible) override;
    virtual bool isCursorVisible() override;
    virtual bool hasAccelerometer() override;
    virtual void getAccelerometerValues(float* pitch, float* roll) override;
    virtual void getSensorValues(float* accelX, float* accelY, float* accelZ, float* gyroX, float* gyroY, float* gyroZ) override;
    virtual void getArguments(int* argc, char*** argv) override;
    virtual void displayKeyboard(bool display) override;
    virtual bool launchURL(const char* url) override;
    virtual std::string displayFileDialog(size_t mode, const char* title, const char* filterDescription, const char* filterExtensions, const char* initialDirectory) override;
};
}

#endif
