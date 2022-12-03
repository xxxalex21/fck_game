#include "kyoshi_script.h"

#include "../components/components.h"

namespace fck
{

KyoshiScript::KyoshiScript() : m_attack_interval{0.5}
{
}

void KyoshiScript::update(const Entity &entity, double delta_time)
{
    TargetFollowComponent &target_follow_component = entity.component<TargetFollowComponent>();
    TargetComponent &target_component = entity.component<TargetComponent>();

    if (target_follow_component.state == TargetFollowComponent::RICHED)
    {
        m_attack_interval -= delta_time;

        if (m_attack_interval < 0)
        {
            SkillsComponent &skills_component = entity.component<SkillsComponent>();
            skills_component.next_skill = 0;
            m_attack_interval = 0.5;
        }
    }
}

} // namespace fck
