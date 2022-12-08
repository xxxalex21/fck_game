#ifndef FCKGAME_H
#define FCKGAME_H

#include "fck_common.h"
#include "gui_base.h"
#include "level.h"

#include "systems/systems.h"

#include "fck/a_star.h"
#include "fck/b2_dynamic_tree.h"
#include "fck/base_game.h"
#include "fck/input_actions_map.h"
#include "fck/world.h"

#include <SFML/Graphics.hpp>

#include <list>
#include <memory>

namespace fck
{

class FckGame : public BaseGame
{
public:
    static FckGame *instance();

    FckGame();
    ~FckGame() = default;

    void init();

protected:
    void update(const sf::Time &elapsed);
    void draw(const sf::Time &elapsed);

private:
    void setState(game_state::State state);

    void event(Event *event);

    void initFirstResources();
    void loadFonts();
    void loadTextures();
    void loadSounds();

    void exitGame();
    void newGame();
    void returnToMainMenu();

    void setupInputActions();

public: // slots
    void onActionActivated(keyboard_action::Action action);

    // world
    void onWorldEntityEnabled(const Entity &entity);
    void onWorldEntityDisabled(const Entity &entity);
    void onWorldEntityDestroyed(const Entity &entity);

    // transform
    void entityMove(const Entity &entity, const sf::Vector2f &offset);
    void entitySetPosition(const Entity &entity, const sf::Vector2f &position);
    void entitySetParent(const Entity &entity, const Entity &parent);

    // state
    void entitySetState(const Entity &entity, entity_state::State state);
    void entitySetDirection(const Entity &entity, entity_state::Direction direction);

    // taregt
    void entitySetTarget(const Entity &entity, const Entity &target);

    // marker
    void entitySetMarker(const Entity &entity, const Entity &marker);

    // stats
    void entitySetHealth(const Entity &entity, float health);
    void entitySetArmor(const Entity &entity, float armor);

    // destroy
    void entityDestroy(const Entity &entity);

    // collided
    void entityCollided(const Entity &entity, const Entity &other);

    // level
    void onLevelRoomOpened(const sf::Vector2i &room_coord);
    void onLevelRoomEnabled(const sf::Vector2i &room_coord);

private:
    sf::RenderTexture m_scene_render_texture;
    sf::Sprite m_scene_render_sprite;
    sf::View m_render_window_view;

    std::unique_ptr<EventHandler> m_event_handler;

    game_state::State m_state;

    std::list<std::unique_ptr<GuiBase>> m_gui_list;

    InputActionsMap<keyboard_action::Action> m_input_actions;

    sf::View m_scene_view;
    std::list<Entity> m_visible_entities;

    World m_world;

    b2::DynamicTree<Entity> m_render_tree;
    b2::DynamicTree<Entity> m_scene_tree;

    PathFinder m_path_finder;

    std::unique_ptr<Level> m_level;
    Entity m_player_entity;

    RenderSystem m_render_system;
    PlayerActionsSystem m_player_actions_system;
    MovementSystem m_movement_system;
    ViewMovementSystem m_view_movement_system;
    DrawableAnimationSystem m_drawable_animation_system;
    CollisionSystem m_collision_system;
    SceneSystem m_scene_system;
    ScriptSystem m_script_system;
    TargetFollowSystem m_target_follow_system;
    LookAroundSystem m_look_around_system;
    StatsSystem m_stats_system;
    SkillsSystem m_skills_system;
    DamageSystem m_damage_sysytem;

    bool m_render_debug;
};

} // namespace fck

#endif // FCKGAME_H
