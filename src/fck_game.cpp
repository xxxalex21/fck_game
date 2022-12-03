#include "fck_game.h"
#include "debug_draw.h"
#include "entity_utils.h"
#include "events.h"
#include "knowledge_base.h"
#include "settings.h"

#include "components/components.h"
#include "damages/damages.h"
#include "entity_scripts/entity_scripts.h"
#include "guis/guis.h"
#include "skills/skills.h"

#include "fck/event_dispatcher.h"
#include "fck/resource_cache.h"
#include "fck/sprite.h"
#include "fck/sprite_animation.h"
#include "fck/task_sequence.h"
#include "fck/tile_map.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <spdlog/spdlog.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <filesystem>

namespace fck
{

FckGame *FckGame::instance()
{
    return static_cast<FckGame *>(BaseGame::instance());
}

FckGame::FckGame()
    : BaseGame{},
      m_render_window_view{sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(1, 1))},
      m_scene_view{sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(1, 1))},
      m_render_system{&m_render_tree},
      m_scene_system{&m_scene_tree, &m_path_finder},
      m_target_follow_system{&m_path_finder},
      m_look_around_system{&m_scene_tree},
      m_render_debug{false}
{
    m_event_handler = std::make_unique<EventHandler>(
        std::vector<int32_t>{
            sf::Event::Resized,
            sf::Event::Closed,
            event_type::KEYBOARD_ACTION,
            event_type::EXIT_GAME,
            event_type::NEW_GAME,
            event_type::RETURN_TO_GAME,
            event_type::RETURN_TO_MAIN_MENU},
        std::bind(&FckGame::event, this, std::placeholders::_1));

    renderWindow().setView(m_render_window_view);

    m_world.addSystem(m_render_system);
    m_world.addSystem(m_player_actions_system);
    m_world.addSystem(m_movement_system);

    m_world.addSystem(m_view_movement_system);
    m_view_movement_system.setView(&m_scene_view);

    m_world.addSystem(m_drawable_animation_system);
    m_world.addSystem(m_collision_system);
    m_world.addSystem(m_scene_system);
    m_world.addSystem(m_script_system);
    m_world.addSystem(m_target_follow_system);
    m_world.addSystem(m_look_around_system);
    m_world.addSystem(m_stats_system);
    m_world.addSystem(m_skills_system);
    m_world.addSystem(m_damage_sysytem);

    // transform
    EntityUtils::entity_moved.connect(this, &FckGame::onEntityMoved);

    // state
    EntityUtils::entity_state_changed.connect(this, &FckGame::onEntityStateChanged);
    EntityUtils::entity_direction_changed.connect(this, &FckGame::onEntityDirectionChanged);

    // taregt
    EntityUtils::entity_target_changed.connect(this, &FckGame::onEntityTargetChanged);

    // stats
    EntityUtils::entity_health_changed.connect(this, &FckGame::onEntityHealthChanged);
    EntityUtils::entity_armor_changed.connect(this, &FckGame::onEntityArmorChanged);
}

void FckGame::init()
{
    auto settings = Settings::global();

    initFirstResources();

    sf::ContextSettings contex_settings;
    contex_settings.antialiasingLevel = 4;
    renderWindow().create(
        sf::VideoMode({settings->window_width, settings->window_height}),
        std::string{"fck_game"},
        settings->fullscreen ? sf::Style::Fullscreen : sf::Style::Default,
        contex_settings);

    TaskSequence *first_loading_tasks = new TaskSequence();
    first_loading_tasks->setTasks(
        {[this, first_loading_tasks]() {
             setState(game_state::FIRST_LOADING);
             //             first_loading_tasks->pause();
             //             Timer::singleShot(
             //                 sf::milliseconds(1000), [first_loading_tasks]() { first_loading_tasks->start(); });
         },
         [this]() { setState(game_state::LOADING); },
         [this, first_loading_tasks]() {
             LoadingGui *loading_gui = static_cast<LoadingGui *>(m_gui_list.back().get());
             loading_gui->setTotal(first_loading_tasks->tasks().size() - 4);
             loading_gui->increaseProgress();
             loadFonts();
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             loadTextures();
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             loadSounds();
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             auto settings = Settings::global();
             KnowledgeBase::loadDrawablesDirectory(settings->sprites_dir_name);
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             auto settings = Settings::global();
             KnowledgeBase::loadSkillsDirectory(settings->skills_dir_name);
         },
         [this]() {
             auto settings = Settings::global();
             KnowledgeBase::loadEntityScriptsDirectory(settings->scripts_dir_name);
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             auto settings = Settings::global();
             KnowledgeBase::loadEntitiesDirectory(settings->entities_dir_name);
         },
         [this]() {
             static_cast<LoadingGui *>(m_gui_list.back().get())->increaseProgress();
             setupInputActions();
         },
         [this, first_loading_tasks]() {
             setState(game_state::MAIN_MENU);
             first_loading_tasks->deleteLater();
         }});
    first_loading_tasks->start();
}

void FckGame::update(const sf::Time &elapsed)
{
    m_world.refresh();

    if (m_state == game_state::LEVEL)
    {
        double delta_time = double(elapsed.asMilliseconds()) / 1000;

        m_visible_entities.clear();

        m_player_actions_system.update(delta_time);
        m_target_follow_system.update(delta_time);

        m_look_around_system.update(delta_time);

        m_script_system.update(delta_time);

        m_skills_system.update(delta_time);
        m_damage_sysytem.update(delta_time);
        m_stats_system.update(delta_time);

        m_collision_system.update(delta_time);
        m_movement_system.update(delta_time);

        m_view_movement_system.update(delta_time);
        m_drawable_animation_system.update(elapsed);

        // Update visible entities
        sf::Vector2f view_pos = m_scene_view.getCenter();
        sf::Vector2f view_size = m_scene_view.getSize();
        sf::FloatRect viewport_rect(
            sf::Vector2f(view_pos.x - view_size.x / 2, view_pos.y - view_size.y / 2),
            sf::Vector2f(view_size.x, view_size.y));

        m_render_tree.querry(viewport_rect, [this](int32_t node_id) {
            Entity entity = m_render_tree.userData(node_id);
            m_visible_entities.push_back(entity);
            return true;
        });

        m_visible_entities.sort([](const Entity &a, const Entity &b) {
            DrawableComponent &dca = a.component<DrawableComponent>();
            DrawableComponent &dcb = b.component<DrawableComponent>();
            return dca.z_order < dcb.z_order;
        });
    }
}

void FckGame::draw(const sf::Time &elapsed)
{
    // Draw scene
    m_scene_render_texture.clear();
    m_scene_render_texture.setView(m_scene_view);

    if (m_state == game_state::LEVEL || m_state == game_state::LEVEL_MENU)
    {
        DrawableComponent *player_drawable_component = nullptr;
        std::optional<Entity> player_target_entity;
        if (m_player_entity.isValid())
        {
            player_drawable_component = &m_player_entity.component<DrawableComponent>();
            player_target_entity = m_player_entity.component<TargetComponent>().target;
        }

        for (Entity &entity : m_visible_entities)
        {
            if (entity.hasComponent<DrawableComponent>())
            {
                DrawableComponent &drawable_component = entity.component<DrawableComponent>();
                TransformComponent &transform_component = entity.component<TransformComponent>();
                bool transparented = false;

                if (player_drawable_component
                    && player_drawable_component->global_content_bounds.width > 0
                    && player_drawable_component->global_content_bounds.height > 0
                    && drawable_component.global_content_bounds.width > 0
                    && drawable_component.global_content_bounds.height > 0
                    && player_drawable_component->global_content_bounds
                           .findIntersection(drawable_component.global_content_bounds)
                           .has_value()
                    && drawable_component.z_order > player_drawable_component->z_order)
                    transparented = true;

                //                sf::Sprite shadow(*ResourceCache::resource<sf::Texture>("kyoshi_shadow"));
                //                shadow.setOrigin(sf::Vector2f(
                //                    shadow.getTexture()->getSize().x / 2, shadow.getTexture()->getSize().y / 2));
                //                shadow.setPosition(transform_component.transform.getPosition());
                //                shadow.setScale({2.0f, 2.0f});
                //                m_scene_render_texture.draw(shadow);

                if (transparented)
                    drawable_component.drawable->setColor(sf::Color(255, 255, 255, 150));

                sf::RenderStates render_states;
                render_states.transform *= transform_component.transform.getTransform();
                drawable_component.drawable->draw(m_scene_render_texture, render_states);

                if (transparented)
                    drawable_component.drawable->setColor(sf::Color(255, 255, 255, 255));
            }
        }

        if (m_render_debug)
        {
            sf::Vector2f view_pos = m_scene_view.getCenter();
            sf::Vector2f view_size = m_scene_view.getSize();
            sf::FloatRect viewport_rect(
                sf::Vector2f(view_pos.x - view_size.x / 2, view_pos.y - view_size.y / 2),
                sf::Vector2f(view_size.x, view_size.y));

            m_scene_tree.querry(viewport_rect, [this](int32_t node_id) {
                Entity entity = m_scene_tree.userData(node_id);

                debug_draw::drawDrawableBounds(entity, m_scene_render_texture);
                //                debug_draw::drawSceneTreeAABB(entity, m_scene_render_texture);
                debug_draw::drawDrawableContentBounds(entity, m_scene_render_texture);
                debug_draw::drawSceneBounds(entity, m_scene_render_texture);
                debug_draw::drawVelocity(entity, m_scene_render_texture);
                debug_draw::drawPathFinderCellsBounds(
                    entity, m_path_finder.grid().cellSize(), m_scene_render_texture);
                debug_draw::drawTargetFollowPath(
                    entity, m_path_finder.grid().cellSize(), m_scene_render_texture);
                debug_draw::drawLookAroundBound(entity, m_scene_render_texture);
                debug_draw::drawLookAroundLookBound(entity, m_scene_render_texture);

                debug_draw::drawEntityId(entity, m_scene_render_texture);

                return true;
            });
        }
    }

    m_scene_render_texture.display();

    renderWindow().clear();

    renderWindow().draw(m_scene_render_sprite);

    // Draw gui
    for (auto &gui : m_gui_list)
        gui->draw(renderWindow(), sf::RenderStates{});

    ImGui::SFML::Render(renderWindow());

    renderWindow().display();
}

void FckGame::setState(game_state::State state)
{
    if (m_state == state)
        return;

    game_state::State old_state = m_state;
    m_state = state;

    EventDispatcher::runTask([this, old_state]() {
        m_input_actions.action_activated.disconnect();
        m_player_actions_system.setInputActions(nullptr);

        switch (m_state)
        {
        case game_state::FIRST_LOADING: {
            m_gui_list.clear();
            m_gui_list.push_back(std::make_unique<SplashscreenGui>(m_render_window_view.getSize()));
            break;
        }
        case game_state::LOADING: {
            m_gui_list.clear();
            m_gui_list.push_back(std::make_unique<LoadingGui>(m_render_window_view.getSize()));

            break;
        }
        case game_state::MAIN_MENU: {
            m_gui_list.clear();
            MainMenuGui *main_menu_gui = new MainMenuGui(m_render_window_view.getSize());
            m_gui_list.push_back(std::unique_ptr<MainMenuGui>(main_menu_gui));

            m_input_actions.action_activated.connect(
                main_menu_gui, &MainMenuGui::onActionActivated);

            break;
        }
        case game_state::LEVEL: {
            m_gui_list.pop_back();

            if (old_state != game_state::LEVEL_MENU)
            {
                LevelGui *level_gui = new LevelGui(m_render_window_view.getSize(), m_player_entity);
                m_gui_list.push_back(std::unique_ptr<LevelGui>(level_gui));
            }

            m_input_actions.action_activated.connect(this, &FckGame::onActionActivated);
            m_player_actions_system.setInputActions(&m_input_actions);

            break;
        }
        case game_state::LEVEL_MENU: {
            MainMenuGui *main_menu_gui = new MainMenuGui(m_render_window_view.getSize(), true);
            m_gui_list.push_back(std::unique_ptr<MainMenuGui>(main_menu_gui));

            m_input_actions.action_activated.connect(
                main_menu_gui, &MainMenuGui::onActionActivated);

            break;
        }
        default:
            break;
        }
    });
}

void FckGame::event(Event *event)
{
    if (event->type() == sf::Event::Resized)
    {
        SfmlEvent *e = static_cast<SfmlEvent *>(event);

        float scene_view_scale = 0.3;

        m_scene_view.setSize(sf::Vector2f(
            e->get().size.width * scene_view_scale + 0.5,
            e->get().size.height * scene_view_scale + 0.5));

        (void)(m_scene_render_texture.create(
            sf::Vector2u(e->get().size.width, e->get().size.height)));
        m_scene_render_texture.setSmooth(true);
        m_scene_render_sprite.setTexture(m_scene_render_texture.getTexture(), true);

        m_render_window_view.setSize(sf::Vector2f(e->get().size.width, e->get().size.height));
        m_render_window_view.setCenter(
            sf::Vector2f(e->get().size.width / 2, e->get().size.height / 2));

        renderWindow().setView(m_render_window_view);

        // Resize gui
        for (auto &gui : m_gui_list)
            gui->resize(m_render_window_view.getSize());

        return;
    }

    if (event->type() == event_type::EXIT_GAME || event->type() == sf::Event::Closed)
    {
        exitGame();
        event->accept();
        return;
    }

    if (event->type() == event_type::NEW_GAME)
    {
        newGame();
        event->accept();
        return;
    }

    if (event->type() == event_type::RETURN_TO_GAME)
    {
        setState(game_state::LEVEL);
        event->accept();
        return;
    }

    if (event->type() == event_type::RETURN_TO_MAIN_MENU)
    {
        returnToMainMenu();
        event->accept();
        return;
    }
}

void FckGame::initFirstResources()
{
    auto settings = Settings::global();

    // Load splashscreen
    ResourceCache::loadFromFile<sf::Texture>("splash", settings->splash_screen_bg_file_name);
    ResourceCache::loadFromFile<sf::Texture>("ui", settings->ui_file_name);
    ResourceCache::loadFromFile<sf::Font>("mini_pixel-7", settings->font_file_name)
        ->setSmooth(true);
}

void FckGame::loadFonts()
{
    auto settings = Settings::global();

    for (const auto &entry : std::filesystem::directory_iterator(settings->fonts_dir_name))
    {
        std::string file_name;
        if (entry.path().filename().has_stem())
            file_name = entry.path().filename().stem().string();
        else
            file_name = entry.path().filename().string();

        if (file_name.empty())
            continue;

        std::string file_path = entry.path().relative_path().string();

        sf::Font *font = ResourceCache::loadFromFile<sf::Font>(file_name, file_path);
        if (font)
            font->setSmooth(true);
    }

    // Load fonts to ImGui
    ImGui::GetIO().Fonts->Clear();
    GuiBase::main_menu_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
        settings->font_file_name.c_str(),
        gui::FONT_SIZE,
        nullptr,
        ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
    GuiBase::hp_hud_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
        settings->font_file_name.c_str(),
        gui::HP_HUD_SIZE,
        nullptr,
        ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
    (void)(ImGui::SFML::UpdateFontTexture());
}

void FckGame::loadTextures()
{
    auto settings = Settings::global();

    for (const auto &entry : std::filesystem::directory_iterator(settings->textures_dir_name))
    {
        std::string file_name;
        if (entry.path().filename().has_stem())
            file_name = entry.path().filename().stem().string();
        else
            file_name = entry.path().filename().string();

        if (file_name.empty())
            continue;

        std::string file_path = entry.path().relative_path().string();

        ResourceCache::loadFromFile<sf::Texture>(file_name, file_path);
    }
}

void FckGame::loadSounds()
{
    auto settings = Settings::global();

    for (const auto &entry : std::filesystem::directory_iterator(settings->sounds_dir_name))
    {
        std::string file_name;
        if (entry.path().filename().has_stem())
            file_name = entry.path().filename().stem().string();
        else
            file_name = entry.path().filename().string();

        if (file_name.empty())
            continue;

        std::string file_path = entry.path().relative_path().string();

        ResourceCache::loadFromFile<sf::SoundBuffer>(file_name, file_path);
    }
}

void FckGame::exitGame()
{
    exit();
}

void FckGame::newGame()
{
    TaskSequence *loading_new_game_tasks = new TaskSequence();
    loading_new_game_tasks->setTasks(
        {[this]() { setState(game_state::LOADING); },
         [this, loading_new_game_tasks]() {
             m_path_finder.grid().setCellSize({32, 32});
             m_path_finder.grid().resize({1000, 1000});

             m_player_entity = KnowledgeBase::createPlayer(
                 "kyoshi", &m_world); //m_entity_db.createPlayer(&m_world);
             m_player_entity.component<TransformComponent>().transform.setPosition(
                 {200.0f, 200.0f});

             ScriptComponent &player_entity_script_component
                 = m_player_entity.addComponent<ScriptComponent>();
             player_entity_script_component.entity_script.reset(new PlayerScript());

             onEntityMoved(m_player_entity, sf::Vector2f{});

             m_player_entity.enable();

             /////////////////////////////////
             Entity k1 = KnowledgeBase::createEntity(
                 "kyoshi_2", &m_world); //m_entity_db.createEntity("kyoshi_2", &m_world);
             k1.component<TransformComponent>().transform.setPosition({100.0f, 100.0f});
             //             k1.component<VelocityComponent>().velocity = {20.0f, 20.0f};

             //             k1.component<SceneComponent>().wall = true;

             TargetComponent &k1_target_component = k1.component<TargetComponent>();
             //k1_target_component.target = m_player_entity;

             TargetFollowComponent &k1_target_foolow_component
                 = k1.component<TargetFollowComponent>();
             k1_target_foolow_component.follow = true;
             k1_target_foolow_component.min_distance = 32.0f;
             k1_target_foolow_component.max_distance = 200.0f;

             onEntityMoved(k1, sf::Vector2f{});

             ScriptComponent &k1_script_component = k1.addComponent<ScriptComponent>();
             k1_script_component.entity_script.reset(new KyoshiScript());

             k1.enable();

             Entity k2 = KnowledgeBase::createEntity(
                 "kyoshi_2", &m_world); //m_entity_db.createEntity("kyoshi_2", &m_world);
             k2.component<TransformComponent>().transform.setPosition({100.0f, 150.0f});
             //             k1.component<VelocityComponent>().velocity = {20.0f, 20.0f};

             //             k2.component<SceneComponent>().wall = true;

             TargetComponent &k2_target_component = k2.component<TargetComponent>();
             k2_target_component.target = m_player_entity;

             TargetFollowComponent &k2_target_foolow_component
                 = k2.component<TargetFollowComponent>();
             k2_target_foolow_component.follow = true;

             onEntityMoved(k2, sf::Vector2f{});

             k2.enable();

             /////////////////////////////////
             //             barrel.component<TransformComponent>().setPosition({100.0f, 50.0f});

             //             for (const auto &group : tmx.objectGroups())
             //             {
             //                 for (const Tmx::Object &object : group.objects)
             //                 {
             //                     if (object.type == Tmx::Object::RECT)
             //                     {
             //                         Entity entity =m_world.createEntity();
             //                         SceneComponent &scene_component = entity.addComponent<SceneComponent>();
             //                         CollisionComponent &collision_component
             //                             = entity.addComponent<CollisionComponent>();
             //                         scene_component.bounds = sf::FloatRect(object.rect);
             //                         collision_component.aabb = sf::FloatRect(object.rect);
             //                         entity.enable();
             //                     }
             //                 }
             //             }

             Tmx tmx2;
             tmx2.loadFromFile("resources/test1.tmx");

             auto tile_maps_2 = TileMap::createFromTmx(&tmx2);

             int32_t z = 1;
             for (TileMap *tile_map : tile_maps_2)
             {

                 if (tile_map)
                 {
                     Entity entity = m_world.createEntity();

                     TransformComponent &transform_component
                         = entity.addComponent<TransformComponent>();

                     DrawableComponent &drawable_component
                         = entity.addComponent<DrawableComponent>();
                     drawable_component.drawable = std::unique_ptr<TileMap>(tile_map);
                     //                     drawable_component.bounds =

                     drawable_component.global_bounds = sf::FloatRect(
                         transform_component.transform.getPosition(),
                         tile_map->localBounds().getSize());
                     drawable_component.z_order = z++;

                     entity.enable();

                     //                     break;
                 }
             }

             for (const auto &group : tmx2.objectGroups())
             {
                 for (const Tmx::Object &object : group.objects)
                 {
                     if (object.type == Tmx::Object::RECT)
                     {
                         Entity entity = m_world.createEntity();

                         TransformComponent &transform_component
                             = entity.addComponent<TransformComponent>();
                         transform_component.transform.setPosition(
                             sf::Vector2f(object.rect.getPosition()));

                         SceneComponent &scene_component = entity.addComponent<SceneComponent>();
                         scene_component.local_bounds = sf::FloatRect(object.rect);
                         scene_component.local_bounds.left = 0;
                         scene_component.local_bounds.top = 0;
                         scene_component.wall = true;

                         CollisionComponent &collision_component
                             = entity.addComponent<CollisionComponent>();

                         collision_component.type = collision_type::STATIC;

                         onEntityMoved(entity, sf::Vector2f{});

                         entity.enable();
                     }
                 }
             }

             setState(game_state::LEVEL);

             loading_new_game_tasks->deleteLater();
         }});
    loading_new_game_tasks->start();
}

void FckGame::returnToMainMenu()
{
    TaskSequence *return_to_main_menu_tasks = new TaskSequence();
    return_to_main_menu_tasks->setTasks(
        {[this]() { setState(game_state::LOADING); },
         [this]() { m_world.destroyAllEntities(); },
         [this]() { m_path_finder.grid().clear(); },
         [this]() { m_visible_entities.clear(); },
         [this, return_to_main_menu_tasks]() {
             setState(game_state::MAIN_MENU);
             return_to_main_menu_tasks->deleteLater();
         }});
    return_to_main_menu_tasks->start();
}

void FckGame::setupInputActions()
{
    m_input_actions[keyboard_action::BACK]
        = InputAction(sf::Keyboard::Escape, InputAction::RELEASE_ONCE);

    m_input_actions[keyboard_action::TOGGLE_RENDER_DEBUG]
        = InputAction(sf::Keyboard::F1, InputAction::RELEASE_ONCE);

    m_input_actions[keyboard_action::PLAYER_MOVE_LEFT]
        = InputAction(sf::Keyboard::A, InputAction::HOLD);
    m_input_actions[keyboard_action::PLAYER_MOVE_UP]
        = InputAction(sf::Keyboard::W, InputAction::HOLD);
    m_input_actions[keyboard_action::PLAYER_MOVE_RIGHT]
        = InputAction(sf::Keyboard::D, InputAction::HOLD);
    m_input_actions[keyboard_action::PLAYER_MOVE_DOWN]
        = InputAction(sf::Keyboard::S, InputAction::HOLD);

    m_input_actions[keyboard_action::CHANGE_TARGET]
        = InputAction(sf::Keyboard::Tab, InputAction::PRESS_ONCE);

    m_input_actions[keyboard_action::PLAYER_SKILL_1]
        = InputAction(sf::Keyboard::J, InputAction::PRESS_ONCE);
    m_input_actions[keyboard_action::PLAYER_SKILL_2]
        = InputAction(sf::Keyboard::K, InputAction::PRESS_ONCE);
    m_input_actions[keyboard_action::PLAYER_SKILL_3]
        = InputAction(sf::Keyboard::L, InputAction::PRESS_ONCE);
}

void FckGame::onActionActivated(keyboard_action::Action action)
{
    if (action == keyboard_action::TOGGLE_RENDER_DEBUG)
        m_render_debug = !m_render_debug;

    if (action == keyboard_action::BACK)
        setState(game_state::LEVEL_MENU);
}

void FckGame::onEntityMoved(const Entity &entity, const sf::Vector2f &offset)
{
    TransformComponent &transform_component = entity.component<TransformComponent>();

    if (!transform_component.children.empty())
    {
        for (auto it = transform_component.children.begin();
             it != transform_component.children.end();)
        {
            if (it->isValid())
            {
                TransformComponent &child_transform_component = it->component<TransformComponent>();
                EntityUtils::setEntityPosition(*it, transform_component.transform.getPosition());
                ++it;
            }
            else
            {
                it = transform_component.children.erase(it);
            }
        }
    }

    if (entity.hasComponent<SceneComponent>())
        m_scene_system.moveEntity(entity, offset);

    if (entity.hasComponent<DrawableComponent>())
        m_render_system.moveEntity(entity, offset);

    if (entity.hasComponent<ScriptComponent>())
    {
        ScriptComponent &script_component = entity.component<ScriptComponent>();
        if (script_component.entity_script)
            script_component.entity_script->onEntityMoved(entity, offset);
    }

    if (entity.hasComponent<LookAroundComponent>())
        m_look_around_system.updateBounds(entity);
}

void FckGame::onEntityStateChanged(const Entity &entity, entity_state::State old_state)
{
    StateComponent &state_component = entity.component<StateComponent>();
    std::string state_string = entity_state::stateToString(state_component.state);

    if (state_component.state != old_state && entity.hasComponent<DrawableAnimationComponent>())
    {
        DrawableAnimationComponent &drawable_animation_component
            = entity.component<DrawableAnimationComponent>();

        if (drawable_animation_component.drawable_animation)
        {
            drawable_animation_component.drawable_animation->setCurrentState(state_string);
            drawable_animation_component.drawable_animation->start();
        }
    }

    if (state_component.state != old_state && entity.hasComponent<SoundComponent>())
    {
        SoundComponent &sound_component = entity.component<SoundComponent>();

        if (state_component.state == entity_state::MOVE)
        {
            sound_component.sound.play();
        }
        else
        {
            sound_component.sound.stop();
        }
    }
}

void FckGame::onEntityDirectionChanged(const Entity &entity, entity_state::Direction old_direction)
{
    StateComponent &state_component = entity.component<StateComponent>();

    if (entity.hasComponent<TransformComponent>())
    {
        TransformComponent &transform_component = entity.component<TransformComponent>();
        transform_component.transform.setScale({float(state_component.direction) * 1.5f, 1.5f});
    }

    if (entity.hasComponent<LookAroundComponent>())
        m_look_around_system.updateBounds(entity);
}

void FckGame::onEntityTargetChanged(const Entity &entity, const Entity &target)
{
    if (entity == m_player_entity && m_state == game_state::LEVEL)
    {
        LevelGui *level_gui = static_cast<LevelGui *>(m_gui_list.back().get());
        level_gui->updateTargetStats();
    }
}

void FckGame::onEntityHealthChanged(const Entity &entity, float old_health)
{
    if (m_state == game_state::LEVEL)
    {
        if (m_player_entity.isValid())
        {
            if (entity == m_player_entity)
            {
                LevelGui *level_gui = static_cast<LevelGui *>(m_gui_list.back().get());
                level_gui->updatePlayerStats();
            }
            else if (entity == m_player_entity.component<TargetComponent>().target)
            {
                LevelGui *level_gui = static_cast<LevelGui *>(m_gui_list.back().get());
                level_gui->updateTargetStats();
            }
        }
    }
}

void FckGame::onEntityArmorChanged(const Entity &entity, float old_armor)
{
    if (m_state == game_state::LEVEL)
    {
        if (m_player_entity.isValid())
        {
            if (entity == m_player_entity)
            {
                LevelGui *level_gui = static_cast<LevelGui *>(m_gui_list.back().get());
                level_gui->updatePlayerStats();
            }
            else if (entity == m_player_entity.component<TargetComponent>().target)
            {
                LevelGui *level_gui = static_cast<LevelGui *>(m_gui_list.back().get());
                level_gui->updateTargetStats();
            }
        }
    }
}

} // namespace fck
