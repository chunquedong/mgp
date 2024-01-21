#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "math/Vector2.h"
#include "platform/Keyboard.h"
#include "platform/Mouse.h"
#include "base/FileSystem.h"

namespace mgp
{

class Game;

/**
 * Defines a platform abstraction.
 *
 * This class has only a few public methods for creating a platform
 */
class Platform
{
public:
    static Platform* cur;

    /**
     * Creates a platform for the specified game which it will interact with.
     *
     * @script{ignore}
     */
    virtual void init(const char* title, int w, int h) = 0;

    /**
     * Begins processing the platform messages.
     *
     * This method handles all OS window messages and drives the game loop.
     * It normally does not return until the application is closed.
     *
     * If a attachToWindow is passed to Platform::create the message pump will instead attach
     * to or allow the attachToWindow to drive the game loop on the platform.
     *
     * @return The platform message pump return code.
     */
    virtual int enterMessagePump() = 0;

    /**
     * Swaps the frame buffer on the device.
     */
    virtual void swapBuffers() = 0;

public:

    virtual void requestRepaint() = 0;

    /**
     * This method informs the platform that the game is shutting down
     * and anything platform specific should be shutdown as well or halted
     * This function is called automatically when the game shutdown function is called
     */
    virtual void signalShutdown() = 0;

    /**
     * Indicates whether a programmatic exit is allowed on this platform.
     * Some platforms (eg. iOS) do not allow apps to exit programmatically.
     *
     * @return whether a programmatic exit is allowed on this platform.
     */
    virtual bool canExit() = 0;

    /**
     * Gets the display width.
     *
     * @return The display width.
     */
    //virtual unsigned int getDisplayWidth() = 0;

    /**
     * Gets the display height.
     *
     * @return The display height.
     */
    //virtual unsigned int getDisplayHeight() = 0;

    /**
    * Screen density scale
    */
    virtual float getScreenScale() = 0;

    /**
     * Gets whether vertical sync is enabled for the game display.
     *
     * @return true if vsync is enabled; false if not.
     */
    virtual bool isVsync() = 0;

    /**
     * Sets whether vertical sync is enable for the game display.
     *
     * @param enable true if vsync is enabled; false if not.
     */
    virtual void setVsync(bool enable) = 0;

    /**
     * Set if multi-sampling is enabled on the platform.
     *
     * @param enabled true sets multi-sampling to be enabled, false to be disabled.
     */
    virtual void setMultiSampling(bool enabled) = 0;

   /**
    * Is multi-sampling mode enabled.
    */
    virtual bool isMultiSampling() = 0;

    /**
     * Whether the platform has mouse support.
     */
    virtual bool hasMouse() = 0;

    /**
     * Enables or disabled mouse capture.
     *
     * When mouse capture is enabled, the platform cursor is hidden
     * and mouse event points are delivered as position deltas instead
     * of absolute positions.
     *
     * This is useful for games that wish to provide uninhibited mouse
     * movement, such as when implementing free/mouse look in an FPS
     * game.
     *
     * Disabling mouse capture moves the mouse back to the center of the
     * screen and shows the platform cursor.
     *
     * Note that this method does nothing on platforms that do not
     * support a mouse.
     *
     * @param captured True to enable mouse capture, false to disable it.
     */
    virtual void setMouseCaptured(bool captured) = 0;

    /**
     * Determines if mouse capture is currently enabled.
     */
    virtual bool isMouseCaptured() = 0;

    /**
     * Sets the visibility of the platform cursor.
     *
     * On platforms that support a visible cursor, this method
     * toggles the visibility of the cursor.
     *
     * @param visible true to show the platform cursor, false to hide it.
     */
    virtual void setCursorVisible(bool visible) = 0;

    /**
     * Determines whether the platform cursor is currently visible.
     *
     * @return true if the platform cursor is visible, false otherwise.
     */
    virtual bool isCursorVisible() = 0;

    /**
     * Whether the platform has accelerometer support.
     */
    virtual bool hasAccelerometer() = 0;

    /**
     * Gets the platform accelerometer values for use as an indication of device
     * orientation. Despite its name, implementations are at liberty to combine
     * accelerometer data with data from other sensors as well, such as the gyros.
     * This method is best used to obtain an indication of device orientation; it
     * does not necessarily distinguish between acceleration and rotation rate.
     *
     * @param pitch The accelerometer pitch. Zero if hasAccelerometer() returns false.
     * @param roll The accelerometer roll. Zero if hasAccelerometer() returns false.
     */
    virtual void getAccelerometerValues(float* pitch, float* roll) = 0;

    /**
     * Gets sensor values (raw), if equipped, allowing a distinction between device acceleration
     * and rotation rate. Returns zeros on platforms with no corresponding support. See also
     * hasAccelerometer() and getAccelerometerValues().
     *
     * @param accelX The x-coordinate of the raw accelerometer data.
     * @param accelY The y-coordinate of the raw accelerometer data.
     * @param accelZ The z-coordinate of the raw accelerometer data.
     * @param gyroX The x-coordinate of the raw gyroscope data.
     * @param gyroY The y-coordinate of the raw gyroscope data.
     * @param gyroZ The z-coordinate of the raw gyroscope data.
     */
    virtual void getSensorValues(float* accelX, float* accelY, float* accelZ, float* gyroX, float* gyroY, float* gyroZ) = 0;

    /**
     * Gets the command line arguments.
     *
     * @param argc The number of command line arguments.
     * @param argv The array of command line arguments.
     */
    virtual void getArguments(int* argc, char*** argv) = 0;

    /**
     * Shows or hides the virtual keyboard (if supported).
     *
     * @param display true when virtual keyboard needs to be displayed and false otherwise.
     */
    virtual void displayKeyboard(bool display) = 0;

    /**
     * Opens an URL in an external browser, if available.
     *
     * @param url URL to be opened.
     *
     * @return True if URL was opened successfully, false otherwise.
     */
    virtual bool launchURL(const char* url) = 0;

    /**
     * Displays an open or save dialog using the native platform dialog system.
     *
     * @param mode The mode of the dialog. (Ex. OPEN or SAVE)
     * @param title The title of the dialog. (Ex. Select File or Save File)
     * @param filterDescription The file filter description. (Ex. Image Files)
     * @param filterExtensions The semi-colon delimited list of filtered file extensions. (Ex. png;jpg;bmp)
     * @param initialDirectory The initial directory to open or save files from. (Ex. "res") If NULL this will use the executable directory.
     * @return The file that is opened or saved, or an empty string if canceled.
     *
     * @script{ignore}
     */
    virtual std::string displayFileDialog(size_t mode, const char* title, const char* filterDescription, const char* filterExtensions, const char* initialDirectory) = 0;

    /**
     * Internal method used only from static code in various platform implementation.
     *
     * @script{ignore}
     */
    //static void shutdownInternal();

    static int run(const char* title = "MGP Engine", int w = 1920, int h = 1080);
private:

    //Game* _game;                // The game this platform is interfacing with.
};

}

#include "app/Game.h"


#endif
