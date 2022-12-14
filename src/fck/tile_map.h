#ifndef TILEMAP_MJXAZIMCABMI_H
#define TILEMAP_MJXAZIMCABMI_H

#include "tmx.h"
#include "vector_2d.h"

#include <SFML/Graphics.hpp>

#include <vector>

namespace fck
{

class ResourceCache;

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
    static TileMap *createFromTmxLayer(
        const Tmx::Layer &layer,
        const sf::Vector2i &map_size,
        const sf::Vector2i &tile_size,
        const std::string &texture_name,
        int32_t first_gid);

    TileMap();
    TileMap(sf::Texture &texture, const sf::Vector2i &tile_size, const Vector2D<int32_t> &tiles);
    ~TileMap() = default;

    sf::Texture *getTexture() const;
    void setTexture(sf::Texture &texture, const sf::Vector2i &tile_size = sf::Vector2i());

    const sf::Color &getColor() const;
    void setColor(const sf::Color &color);

    const sf::Vector2i &getMapSize() const;
    void setMapSize(const sf::Vector2i &map_size);

    sf::Vector2i getTileSize() const;
    void setTileSize(const sf::Vector2i &tile_size);

    int32_t getTile(const sf::Vector2i &position) const;
    const Vector2D<int32_t> &getTiles() const;

    void setTile(const sf::Vector2i &position, int32_t tile);
    void setTiles(const Vector2D<int32_t> &tiles);

    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;

protected:
    void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;

private:
    void updatePositions();
    void updateTexCoords();

private:
    sf::VertexArray m_vertices;
    sf::Texture *m_texture;

    sf::Vector2i m_tile_size;
    Vector2D<int32_t> m_tiles;
};

} // namespace fck

#endif // TILEMAP_MJXAZIMCABMI_H
