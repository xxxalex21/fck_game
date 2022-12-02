#ifndef FCK_COMMON_H
#define FCK_COMMON_H

#include "fck/common.h"

#include <cstdint>

namespace fck
{

const int32_t Z_ORDER = 1000000;

namespace gui
{

const int32_t FONT_SIZE = 28;
const int32_t HP_HUD_SIZE = 26;

} // namespace gui

namespace event_type
{

enum Type
{
    EXIT_GAME = 1000,
    NEW_GAME,
    RETURN_TO_GAME,
    RETURN_TO_MAIN_MENU,

    KEYBOARD_ACTION,

};

}

namespace keyboard_action
{

enum Action
{
    TOGGLE_RENDER_DEBUG = 10000,
    TOOGLE_COLLISION_DEBUG = 10001,

    BACK = 12000,

    PLAYER_MOVE_LEFT = 14001,
    PLAYER_MOVE_UP = 14002,
    PLAYER_MOVE_RIGHT = 14003,
    PLAYER_MOVE_DOWN = 14004,

    CHANGE_TARGET = 15000,

    PLAYER_SKILL_1 = 16000,
    PLAYER_SKILL_2 = 16001,
    PLAYER_SKILL_3 = 16002,
    PLAYER_SKILL_4 = 16003
};

}

namespace game_state
{
enum State
{
    NO_STATE,
    FIRST_LOADING,
    LOADING,
    MAIN_MENU,
    LEVEL,
    LEVEL_MENU
};
}

namespace entity_state
{

enum State
{
    NO_STATE = 1,

    IDLE = 2,
    MOVE = 4,

    ACTION = 50,
    UNDER_ACTION = 51,

    SKILL_APPLY = 8,
    DAMAGED = 16,
    DEATH = 32,

    NOT_AVALIBLE = SKILL_APPLY | DAMAGED | DEATH
};

std::string stateToString(State state);
State stateFromString(const std::string &string);

enum Direction
{
    NO_DIRECTION = 0,
    LEFT = -1,
    RIGHT = 1
};

std::string directionToString(Direction direction);
Direction directionFromString(const std::string &string);

} // namespace entity_state

namespace component_type
{

enum Type
{
    NO_TYPE,
    TRANSFORM,
    VELOCITY,
    SCENE,
    PLAYER,
    DRAWABLE,
    DRAWABLE_ANIMATION,
    COLLISION,
    STATE,
    SOUND,
    SCRIPT,
    TARGET,
    TARGET_FOLLOW,
    LOOK_AROUND,
    TYPE,
    STATS,
    ACTIONS,
    SKILLS,
    DAMAGE
};

std::string toString(Type type);
Type fromString(const std::string &string);
} // namespace component_type

namespace entity_type
{

enum Type
{
    NO_TYPE,
    PLAYER,
    ENEMY
};

std::string toString(Type type);
Type fromString(const std::string &string);
} // namespace entity_type

namespace collision_type
{

enum Type
{
    NO_TYPE,
    STATIC,
    DYNAMIC
};

std::string toString(Type type);
Type fromString(const std::string &string);

} // namespace collision_type

} // namespace fck

#endif // FCK_COMMON_H