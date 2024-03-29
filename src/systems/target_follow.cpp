#include "target_follow.h"
#include "../entity_funcs.h"
#include "../fck/a_star.h"
#include "../fck/utilities.h"

namespace fck::system
{

TargetFollow::TargetFollow() : m_map{nullptr}, m_walls{nullptr}
{
}

void TargetFollow::update(double delta_time)
{
    for (Entity &entity : getEntities())
    {
        component::TargetFollow &target_follow_component = entity.get<component::TargetFollow>();
        component::Transform &transform_component = entity.get<component::Transform>();
        component::Velocity &velocity_component = entity.get<component::Velocity>();
        component::State &state_component = entity.get<component::State>();

        if (!target_follow_component.follow || !m_walls)
        {
            if (!target_follow_component.path.empty())
            {
                target_follow_component.path.clear();
                target_follow_component.state = component::TargetFollow::LOST;
                velocity_component.velocity = {0.0f, 0.0f};
                entity_funcs::setState(entity, entity_state::IDLE);
                entity_funcs::setDrawableState(
                    entity, entity_state::stateToString(entity_state::IDLE));
                entity_funcs::stopSound(entity, entity_state::stateToString(entity_state::MOVE));
            }
            continue;
        }

        float dist_to_target = vector2::distance(
            transform_component.transform.getPosition(), target_follow_component.target);

        if (target_follow_component.state == component::TargetFollow::RICHED
            && dist_to_target < target_follow_component.min_distance * 1.5f)
            continue;

        target_follow_component.state = component::TargetFollow::LOST;
        velocity_component.velocity = {0.0f, 0.0f};

        sf::Vector2i target_coord = transformPosition(target_follow_component.target);

        // Need update path
        if ((target_follow_component.path.empty()
             || target_follow_component.path.front() != target_coord)
            && dist_to_target > target_follow_component.min_distance)
        {
            // Check cell reachable
            int32_t cell_weight = m_walls->getData(target_coord);
            if (cell_weight != 0)
            {
                static std::vector<sf::Vector2i> neighbor_coords
                    = {{-1, 0}, {0, -1}, {0, 1}, {0, 1}};
                for (const sf::Vector2i &neighbor_coord : neighbor_coords)
                {
                    const int32_t &neighbor_cell_weight
                        = m_walls->getData(target_coord + neighbor_coord);
                    if (neighbor_cell_weight == 0)
                    {
                        target_coord = target_coord + neighbor_coord;
                        cell_weight = neighbor_cell_weight;
                        break;
                    }
                }
            }

            if (cell_weight == 0)
            {
                PathFinder path_finder{*m_walls};
                target_follow_component.path = path_finder.findPath(
                    transformPosition(transform_component.transform.getPosition()), target_coord);

                if (!target_follow_component.path.empty())
                    target_follow_component.path.erase(target_follow_component.path.end() - 1);
            }
        }

        if (target_follow_component.path.empty())
        {
            entity_funcs::setState(entity, entity_state::IDLE);
            continue;
        }

        if (dist_to_target > target_follow_component.min_distance)
        {
            sf::Vector2f path_point = {-1, -1};
            while (!target_follow_component.path.empty())
            {
                sf::Vector2f next_path_point = sf::Vector2f{
                    sf::Vector2i{
                        target_follow_component.path.back().x * m_wall_size.x,
                        target_follow_component.path.back().y * m_wall_size.y}
                    + m_wall_size / 2};

                float dist_to_next_point = vector2::distance(
                    transform_component.transform.getPosition(), next_path_point);

                if (dist_to_next_point < (m_wall_size.x / 4))
                {
                    target_follow_component.path.erase(target_follow_component.path.end() - 1);
                }
                else
                {
                    path_point = next_path_point;
                    break;
                }
            }

            if (path_point != sf::Vector2f{-1, -1})
            {
                float angle
                    = vector2::angleTo(transform_component.transform.getPosition(), path_point);

                velocity_component.velocity
                    = {-velocity_component.max_velocity.x * std::cos(angle),
                       -velocity_component.max_velocity.y * std::sin(angle)};

                if ((velocity_component.velocity.x != 0 || velocity_component.velocity.y != 0)
                    && state_component.state != entity_state::MOVE)
                {
                    entity_funcs::setState(entity, entity_state::MOVE);
                    entity_funcs::setDrawableState(
                        entity, entity_state::stateToString(entity_state::MOVE));
                    entity_funcs::playSound(
                        entity, entity_state::stateToString(entity_state::MOVE));
                }
                else if (
                    !vector2::isValid(velocity_component.velocity)
                    && state_component.state != entity_state::IDLE)
                {
                    entity_funcs::setState(entity, entity_state::IDLE);
                    entity_funcs::setDrawableState(
                        entity, entity_state::stateToString(entity_state::IDLE));
                    entity_funcs::stopSound(
                        entity, entity_state::stateToString(entity_state::MOVE));
                }

                if (velocity_component.velocity.x > 0)
                    entity_funcs::setDirection(entity, entity_state::RIGHT);
                else if (velocity_component.velocity.x < 0)
                    entity_funcs::setDirection(entity, entity_state::LEFT);
            }
        }
        else
        {
            target_follow_component.path.clear();
            target_follow_component.state = component::TargetFollow::RICHED;
            entity_funcs::setState(entity, entity_state::IDLE);
            entity_funcs::setDrawableState(entity, entity_state::stateToString(entity_state::IDLE));
            entity_funcs::stopSound(entity, entity_state::stateToString(entity_state::MOVE));
        }
    }
}

void TargetFollow::onMapChanged(map::Map *map)
{
    m_map = map;
    m_walls = nullptr;
    m_wall_size = {};
}

void TargetFollow::onChunkChanged(const sf::Vector2i &chunk_coords)
{
    if (!m_map)
        return;

    const map::Chunk *chunk = m_map->getChunks().getData(chunk_coords);
    m_walls = &chunk->getWalls();
    m_wall_size = chunk->getWallSize();
}

sf::Vector2i TargetFollow::transformPosition(const sf::Vector2f &position)
{
    if (m_wall_size.x == 0 || m_wall_size.y == 0)
        return {};

    return {int32_t(position.x) / m_wall_size.x, int32_t(position.y) / m_wall_size.y};
}

} // namespace fck::system
