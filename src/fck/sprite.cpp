#include "sprite.h"

namespace fck
{

Sprite::Sprite() : m_vertices{sf::TriangleStrip}, m_texture{nullptr}
{
    m_vertices.resize(4);
}

Sprite::Sprite(sf::Texture &texture) : m_vertices{sf::TriangleStrip}, m_texture{nullptr}
{
    m_vertices.resize(4);
    setTexture(texture);
}

Sprite::Sprite(sf::Texture &texture, const sf::IntRect &rectangle)
    : m_vertices{sf::TriangleStrip}, m_texture{nullptr}
{
    m_vertices.resize(4);
    setTexture(texture);
    setTextureRect(rectangle);
}

drawable_type::Type Sprite::getType() const
{
    return drawable_type::SPRITE;
}

sf::Texture *Sprite::getTexture() const
{
    return m_texture;
}

void Sprite::setTexture(sf::Texture &texture, bool resetRect)
{
    if (resetRect)
        setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(texture.getSize().x, texture.getSize().y)));
    m_texture = &texture;
}

const sf::IntRect &Sprite::getTextureRect() const
{
    return m_texture_rect;
}

void Sprite::setTextureRect(const sf::IntRect &rectangle)
{
    m_texture_rect = rectangle;
    updateTexCoords();
    updatePositions();
}

const sf::Color &Sprite::getColor() const
{
    return m_vertices[0].color;
}

void Sprite::setColor(const sf::Color &color)
{
    m_vertices[0].color = color;
    m_vertices[1].color = color;
    m_vertices[2].color = color;
    m_vertices[3].color = color;
}

sf::FloatRect Sprite::getLocalBounds() const
{
    return m_vertices.getBounds();
}

sf::FloatRect Sprite::getGlobalBounds() const
{
    return getTransform().transformRect(m_vertices.getBounds());
}

void Sprite::draw(sf::RenderTarget &target, const sf::RenderStates &states) const
{
    if (m_texture)
    {
        sf::RenderStates new_state = states;
        new_state.texture = m_texture;
        new_state.transform *= getTransform();
        target.draw(m_vertices, new_state);
    }
}

void Sprite::updatePositions()
{
    m_vertices[0].position = sf::Vector2f(0, 0);
    m_vertices[1].position = sf::Vector2f(0, m_texture_rect.height);
    m_vertices[2].position = sf::Vector2f(m_texture_rect.width, 0);
    m_vertices[3].position = sf::Vector2f(m_texture_rect.width, m_texture_rect.height);
}

void Sprite::updateTexCoords()
{
    float left = static_cast<float>(m_texture_rect.left);
    float right = left + m_texture_rect.width;
    float top = static_cast<float>(m_texture_rect.top);
    float bottom = top + m_texture_rect.height;

    m_vertices[0].texCoords = sf::Vector2f(left, top);
    m_vertices[1].texCoords = sf::Vector2f(left, bottom);
    m_vertices[2].texCoords = sf::Vector2f(right, top);
    m_vertices[3].texCoords = sf::Vector2f(right, bottom);
}

} // namespace fck
