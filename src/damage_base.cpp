#include "damage_base.h"

namespace fck
{

DamageBase::DamageBase(const Entity &entity, double interval, const Entity &source)
    : m_entity{entity}, m_source{source}, m_interval{interval}, m_elapsed{0}
{
}

const Entity &DamageBase::entity() const
{
    return m_entity;
}

const Entity &DamageBase::source() const
{
    return m_source;
}

double DamageBase::interval() const
{
    return m_interval;
}

double DamageBase::elapsed() const
{
    return m_elapsed;
}

bool DamageBase::isReady() const
{
    return m_elapsed == m_interval;
}

void DamageBase::finish()
{
    m_elapsed = m_interval;
}

void DamageBase::_update(double delta_time)
{
    if (m_elapsed < m_interval)
        m_elapsed += delta_time;

    if (m_elapsed > m_interval)
        m_elapsed = m_interval;

    update(delta_time);
}

} // namespace fck
