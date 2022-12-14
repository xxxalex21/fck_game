#ifndef INPUTACTIONS_GLASEISJUVEL_H
#define INPUTACTIONS_GLASEISJUVEL_H

#include "input_action.h"
#include "sigslot.h"

#include <SFML/Window/Event.hpp>

#include <functional>
#include <memory>
#include <unordered_map>

namespace fck
{

template<typename T>
class InputActionsMap
{
public:
    InputActionsMap();
    ~InputActionsMap() = default;

    InputAction &operator[](const T &id);
    void setActions(const std::unordered_map<T, InputAction> &input_actions);
    void removeAction(const T &id);
    void clearActions();

    bool isActive(const T &id) const;

    void event(const sf::Event &e);

public:
    Signal<T> action_activated;
    Signal<T> action_diactivated;

private:
    std::unordered_map<T, InputAction> m_input_actions;
};

template<typename T>
InputActionsMap<T>::InputActionsMap()
{
}

template<typename T>
InputAction &InputActionsMap<T>::operator[](const T &id)
{
    return m_input_actions[id];
}

template<typename T>
void InputActionsMap<T>::setActions(const std::unordered_map<T, InputAction> &input_actions)
{
    m_input_actions = input_actions;
}

template<typename T>
void InputActionsMap<T>::removeAction(const T &id)
{
    m_input_actions.erase(id);
}

template<typename T>
void InputActionsMap<T>::clearActions()
{
    m_input_actions.clear();
}

template<typename T>
bool InputActionsMap<T>::isActive(const T &id) const
{
    auto input_actions_found = m_input_actions.find(id);
    if (input_actions_found == m_input_actions.end())
        return false;
    return input_actions_found->second.activated;
}

template<typename T>
void InputActionsMap<T>::event(const sf::Event &e)
{
    if (e.type != sf::Event::KeyPressed && e.type != sf::Event::KeyReleased)
        return;

    uint64_t mask = 0;
    mask ^= e.key.code << 1;

    for (auto &it : m_input_actions)
    {
        if (it.second.mask == mask)
        {
            if (e.type == sf::Event::KeyPressed)
            {
                if (it.second.type == InputAction::PRESS_ONCE && !it.second.caused
                    && !it.second.activated)
                {
                    it.second.activated = true;
                    it.second.caused = true;
                    action_activated.emit(it.first);
                    continue;
                }

                if (it.second.type == InputAction::PRESS_ONCE && it.second.caused
                    && it.second.activated)
                {
                    it.second.activated = false;
                    continue;
                }

                if (it.second.type == InputAction::HOLD && !it.second.caused)
                {
                    it.second.activated = true;
                    it.second.caused = true;
                    action_activated.emit(it.first);
                    continue;
                }
            }

            if (e.type == sf::Event::KeyReleased)
            {
                if (it.second.type == InputAction::PRESS_ONCE)
                {
                    it.second.caused = false;
                    it.second.activated = false;
                }

                if (it.second.type == InputAction::HOLD)
                {
                    it.second.caused = false;
                    it.second.activated = false;
                    action_diactivated.emit(it.first);
                }

                if (it.second.type == InputAction::RELEASE_ONCE)
                {
                    it.second.activated = true;
                    it.second.activated = false;
                    action_activated.emit(it.first);
                }
            }
        }
    }
}

} // namespace fck

#endif // INPUTACTIONS_GLASEISJUVEL_H
