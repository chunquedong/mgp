/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef Toolkit_H_
#define Toolkit_H_

#include "math/Rectangle.h"
#include "base/System.h"
#include <functional>

namespace mgp
{
    class Properties;
/**
 * Defines a interface to be scheduled and called back at a later time using Application::schedule().
 *
 * @script{ignore}
 */
class TimeListener
{
public:

    /**
     * Callback method that is called when the scheduled event is fired.
     * 
     * @param timeDiff The time difference between the current game time and the target time.
     *                 The time differences will always be non-negative because scheduled events will not fire early.
     * @param cookie The cookie data that was passed when the event was scheduled.
     */
    virtual void timeEvent(long timeDiff, void* cookie) = 0;
};
/**
 * Platform or system desktop API.
 */
class Toolkit
{
protected:
    static Toolkit* g_instance;
public:
    static Toolkit* cur();
    
    /**
     * Gets the game current viewport.
     *
     * The default viewport is Rectangle(0, 0, Application::getWidth(), Application::getHeight()).
     */
    virtual float getScreenScale() = 0;

    /**
     * Shows or hides the virtual keyboard (if supported).
     *
     * @param display true when virtual keyboard needs to be displayed; false otherwise.
     */
     virtual void displayKeyboard(bool display) = 0;

     /**
     * Schedules a time event to be sent to the given TimeListener a given number of game milliseconds from now.
     * Application time stops while the game is paused. A time offset of zero will fire the time event in the next frame.
     * 
     * @param timeOffset The number of game milliseconds in the future to schedule the event to be fired.
     * @param timeListener The TimeListener that will receive the event.
     * @param cookie The cookie data that the time event will contain.
     * @script{ignore}
     */
    virtual void schedule(int64_t timeOffset, TimeListener* timeListener, void* cookie = 0) = 0;
    virtual void setTimeout(int64_t timeMillis, std::function<void()> callback) = 0;

    /**
     * Clears all scheduled time events.
     */
    virtual void clearSchedule() = 0;

    /**
     * Gets whether mouse input is currently captured.
     *
     * @return is the mouse captured.
     */
    virtual bool isMouseCaptured() = 0;

    /**
    * Request next frame to render.
    * Do nothing in game main loop mode.
    */
    virtual void requestRepaint() = 0;

    /**
     * Gets the total game time (in milliseconds). This is the total accumulated game time (unpaused).
     *
     * You would typically use things in your game that you want to stop when the game is paused.
     * This includes things such as game physics and animation.
     * 
     * @return The total game time (in milliseconds).
     */
    virtual double getGameTime() = 0;
};

}

#endif
