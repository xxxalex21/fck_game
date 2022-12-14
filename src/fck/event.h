#ifndef EVENT_IODHTGAOXWRM_H
#define EVENT_IODHTGAOXWRM_H

#include "common.h"

#include <SFML/Window/Event.hpp>

#include <cstdint>
#include <functional>

namespace fck
{

class Event
{
public:
    Event(int32_t type);
    virtual ~Event() = default;

    int32_t type() const;
    bool isAccepted() const;
    void accept();

private:
    int32_t m_type;
    bool m_accepted;
};

class TaskEvent : public Event
{
public:
    TaskEvent(const std::function<void()> &callback);
    ~TaskEvent() = default;

    void call();

private:
    std::function<void()> m_callback;
};

class TasksEvent : public Event
{
public:
    TasksEvent(const std::vector<std::function<bool()>> &callbacks, int32_t index);
    ~TasksEvent() = default;

    bool call();

private:
    std::vector<std::function<bool()>> m_callbacks;
    int32_t m_index;
};

} // namespace fck

#endif // EVENT_IODHTGAOXWRM_H
