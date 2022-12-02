#include "entity.h"
#include "common.h"
#include "utilities.h"
#include "world.h"

namespace fck
{

Entity::Entity() : m_world{nullptr}
{
}

Entity::Entity(Id id, World *scene) : m_id{id}, m_world{scene}
{
}

Entity::Entity(const Entity &other)
{
    m_id = other.m_id;
    m_world = other.m_world;
}

Entity &Entity::operator=(const Entity &other)
{
    m_id = other.m_id;
    m_world = other.m_world;
    return *this;
}

const Id &Entity::id() const
{
    return m_id;
}

World *Entity::world() const
{
    fck_assert(m_world, "scene reference in entity is null");
    return m_world;
}

bool Entity::isValid() const
{
    return m_world ? m_world->isValid(*this) : false;
}

bool Entity::isEnabled() const
{
    return world()->isEnabled(*this);
}

void Entity::enable()
{
    world()->enableEntity(*this);
}

void Entity::disable()
{
    world()->disableEntity(*this);
}

void Entity::destroy()
{
    world()->destroyEntity(*this);
}

void Entity::removeAllComponents()
{
    world()->m_entity_attributes.component_storage.removeAllComponents(*this);
}

ComponentsFilter Entity::componentFilter() const
{
    return world()->m_entity_attributes.component_storage.componentsFilter(*this);
}

bool Entity::operator==(const Entity &entity) const
{
    return m_id == entity.m_id && entity.m_world == m_world;
}

bool Entity::operator!=(const Entity &entity) const
{
    return !operator==(entity);
}

void Entity::addComponent(ComponentBase *component, TypeId component_type_id)
{
    world()->m_entity_attributes.component_storage.addComponent(
        *this, component, component_type_id);
}

void Entity::removeComponent(TypeId component_type_id)
{
    world()->m_entity_attributes.component_storage.removeComponent(*this, component_type_id);
}

ComponentBase *Entity::component(TypeId component_type_id) const
{
    return world()->m_entity_attributes.component_storage.component(*this, component_type_id);
}

bool Entity::hasComponent(TypeId component_type_id) const
{
    return world()->m_entity_attributes.component_storage.hasComponent(*this, component_type_id);
}

} // namespace fck