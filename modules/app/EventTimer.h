#ifndef EVENT_TIMER_H_
#define EVENT_TIMER_H_

#include "base/Base.h"
#include "base/Ptr.h"
#include "platform/Toolkit.h"
#include <queue>
#include <mutex>

namespace mgp
{
class EventTimer {

    /**
     * TimeEvent represents the event that is sent to TimeListeners as a result of calling Application::schedule().
     */
    class TimeEvent
    {
    public:

        TimeEvent(double time, SPtr<TimeListener> timeListener, void* cookie);
        bool operator<(const TimeEvent& v) const;
        double time;
        SPtr<TimeListener> listener;
        void* cookie;
    };

    std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >* _timeEvents;     // Contains the scheduled time events.
    std::mutex _scheduleLock;
public:
    EventTimer();
    ~EventTimer();

    /**
     * Schedules a time event to be sent to the given TimeListener a given number of game milliseconds from now.
     * Application time stops while the game is paused. A time offset of zero will fire the time event in the next frame.
     *
     * @param timeOffset The number of game milliseconds in the future to schedule the event to be fired.
     * @param timeListener The TimeListener that will receive the event.
     * @param cookie The cookie data that the time event will contain.
     * @script{ignore}
     */
    void schedule(float timeOffset, TimeListener* timeListener, void* cookie = 0);

    /**
     * Schedules a time event to be sent to the given TimeListener a given number of game milliseconds from now.
     * Application time stops while the game is paused. A time offset of zero will fire the time event in the next frame.
     *
     * The given script function must take a single floating point number, which is the difference between the
     * current game time and the target time (see TimeListener::timeEvent). The function will be executed
     * in the context of the script envionrment that the schedule function was called from.
     *
     * @param timeOffset The number of game milliseconds in the future to schedule the event to be fired.
     * @param function The script function that will receive the event.
     */
    void schedule(float timeOffset, const char* function);

    /**
     * Clears all scheduled time events.
     */
    void clearSchedule();
    
    /**
     * Fires the time events that were scheduled to be called.
     * 
     * @param frameTime The current game frame time. Used to determine which time events need to be fired.
     */
    void fireTimeEvents();
};

}

#endif
