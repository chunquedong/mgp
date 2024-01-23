#include "EventTimer.h"

using namespace mgp;

EventTimer::EventTimer() {
    _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >();
}

EventTimer::~EventTimer() {
    SAFE_DELETE(_timeEvents);
}

void EventTimer::schedule(int64_t timeOffset, TimeListener* timeListener, void* cookie)
{
    GP_ASSERT(_timeEvents);
    SPtr<TimeListener> listener;
    listener = timeListener;
    uint64_t time = System::millisTicks() + timeOffset;
    TimeEvent timeEvent(time, listener, cookie);
    _scheduleLock.lock();
    _timeEvents->push(timeEvent);
    _scheduleLock.unlock();
}

void EventTimer::schedule(int64_t timeOffset, const char* function)
{
#if undeclared
    _scriptController->schedule(timeOffset, function);
#endif
}

void EventTimer::clearSchedule()
{
    _scheduleLock.lock();
    SAFE_DELETE(_timeEvents);
    _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >();
    _scheduleLock.unlock();
}

void EventTimer::fireTimeEvents()
{
    int64_t frameTime = System::millisTicks();
    std::vector<TimeEvent> toFire;
    _scheduleLock.lock();
    while (_timeEvents->size() > 0)
    {
        const TimeEvent timeEvent = _timeEvents->top();
        if (timeEvent.time > frameTime)
        {
            break;
        }
        //if (timeEvent.listener)
        //{
        toFire.push_back(timeEvent);
        //}
        _timeEvents->pop();
    }
    _scheduleLock.unlock();

    for (auto timeEvent : toFire) {
        SPtr<TimeListener>& listener = timeEvent.listener;
        if (!listener.isNull()) {
            listener->timeEvent(frameTime - timeEvent.time, timeEvent.cookie);
        }
    }
}

EventTimer::TimeEvent::TimeEvent(int64_t time, SPtr<TimeListener> timeListener, void* cookie)
    : time(time), listener(timeListener), cookie(cookie)
{
}

bool EventTimer::TimeEvent::operator<(const TimeEvent& v) const
{
    // The first element of std::priority_queue is the greatest.
    return time > v.time;
}