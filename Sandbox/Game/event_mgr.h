// 11.04.2026
#pragma once
#include <Engine/engine.h>


namespace Game
{

struct EventData
{
    EntityID enttId;
    float deltaTime;

    union {
        struct { int   ix, iy, iz, iw; };
        struct { float fx, fy, fz, fw; };
    };
};

//---------------------------------------------------------

// typedef for event listeners functions
typedef void (*EventListenerFunc)(Core::Engine* pEngine, const EventData* pData);

 
//---------------------------------------------------------

class GameEvent
{
private:
    static const int MAX_NUM_LISTENERS = 8;

    EventListenerFunc listeners[MAX_NUM_LISTENERS] = { nullptr };
    int numListeners = 0;

public:
    void AddListener(EventListenerFunc func)
    {
        if (numListeners >= MAX_NUM_LISTENERS)
            return;

        assert(func);
        listeners[numListeners++] = func;
    }

    void RemoveListener(EventListenerFunc func)
    {
        for (int i = 0; i < numListeners; ++i)
        {
            if (listeners[i] == func)
            {
                // swap n pop
                listeners[i] = listeners[numListeners-1];
                numListeners--;
            }
        }
    }

    void Notify(Core::Engine* pEngine, const EventData* pData)
    {
        assert(pEngine);

        for (int i = 0; i < numListeners; ++i)
        {
            listeners[i](pEngine, pData);
        }
    }
};

//---------------------------------------------------------

class EventMgr
{
private:
    cvector<std::string> eventNames_;
    cvector<EventID>   ids_;
    cvector<GameEvent> events_;

    EventID eventId_ = 0;

public:
    EventMgr() {}

    void Subscribe(const char* eventName, EventListenerFunc listener)
    {
        assert(eventName && eventName[0] != '\0');

        GameEvent* pEvent = nullptr;
        const index idx = eventNames_.find(eventName);

        // if we currently have no event by input name...
        if (idx == -1)
        {
            // create a new event
            EventID id = eventId_++;
            eventNames_.push_back(eventName);
            ids_.push_back(id);
            events_.push_back(GameEvent());
            pEvent = &events_.back();
        }
        else
        {
            pEvent = &events_[idx];
        }

        pEvent->AddListener(listener);
    }

    void Unsubscribe(const char* eventName, EventListenerFunc listener)
    {
        assert(eventName && eventName[0] != '\0');

        const index idx = eventNames_.find(eventName);
        if (idx == -1)
        {
            LogErr(LOG, "no event by name: %s", eventName);
            return;
        }

        events_[idx].RemoveListener(listener);
    }

    //
    // if pData == nullptr, we have no data for this event, so it's ok
    //
    void TriggerEvent(const EventID id, Core::Engine* pEngine, const EventData* pData)
    {
        for (index i = 0; i < ids_.size(); ++i)
        {
            if (ids_[i] == id)
            {
                events_[i].Notify(pEngine, pData);
                return;
            }
        }

        LogErr(LOG, "no event by id: %d", (int)id);
        return;
    }

    //
    // if pData == nullptr, we have no data for this event, so it's ok
    //
    void TriggerEvent(const char* eventName, Core::Engine* pEngine, const EventData* pData)
    {
        assert(eventName && eventName[0] != '\0');

        const index idx = eventNames_.find(eventName);
        if (idx == -1)
        {
            LogErr(LOG, "no event by name: %s", eventName);
            return;
        }

        events_[idx].Notify(pEngine, pData);
    }

};

} // namespace
