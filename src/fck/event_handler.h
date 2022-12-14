#ifndef EVENTHANDLER_JRNTUTUBPKZE_H
#define EVENTHANDLER_JRNTUTUBPKZE_H

#include "event.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace fck
{

class EventHandler
{
public:
    EventHandler(
        const std::vector<int32_t> &event_types,
        const std::function<void(Event *)> &callback = {},
        const std::string &name = std::string{});
    virtual ~EventHandler();

    const std::vector<int32_t> &getEventTypes() const;
    std::string getName() const;

    bool isEnable() const;
    void setEnable(bool enable);

    void deleteLater();

    virtual void event(Event *event);

private:
    std::vector<int32_t> m_event_types;
    std::function<void(Event *)> m_callback;
    std::string m_name;
    bool m_enable;
};

} // namespace fck

#endif // EVENTHANDLER_JRNTUTUBPKZE_H
