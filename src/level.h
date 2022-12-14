#ifndef LEVEL_ZKKSOSIXHXVE_H
#define LEVEL_ZKKSOSIXHXVE_H

#include "fck_common.h"

#include "fck/b2_dynamic_tree.h"
#include "fck/entity.h"
#include "fck/grid.h"
#include "fck/sigslot.h"
#include "fck/tmx.h"
#include "fck/utilities.h"
#include "fck/vector_2d.h"
#include "fck/world.h"

#include "spdlog/spdlog.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace fck
{

class Room
{
public:
    Room();
    ~Room() = default;

    int32_t getNeighbors() const;
    void setNeighbors(int32_t neighbors);

    room_type::Type getType() const;
    void setType(room_type::Type type);

    const std::unordered_map<room_side::Side, sf::Vector2f> &getEntryPoints() const;
    void setEntryPoints(const std::unordered_map<room_side::Side, sf::Vector2f> &entry_points);

    const Grid<int32_t> &getWalls() const;
    void setWalls(const Grid<int32_t> &walls);

    const Grid<tile_material_type::Type> &getTileMaterials() const;
    void setTileMaterials(const Grid<tile_material_type::Type> &tile_materials);

    bool isOpen() const;
    void setOpen(bool open);

private:
    int32_t m_neighbors;
    room_type::Type m_type;
    std::unordered_map<room_side::Side, sf::Vector2f> m_entry_points;
    Grid<int32_t> m_walls;
    Grid<tile_material_type::Type> m_tile_materials;
    bool m_open;
};

class Level
{
public:
    Level(World *world, b2::DynamicTree<Entity> *scene_tree);
    ~Level();

    bool loadFromFile(const std::string &file_name);

    void createRoomsMap(const Vector2D<BoolProxy> &rooms_map);
    void createRandomRoomsMap(int32_t room_count = 30);
    void createRoomsContent();

    const Vector2D<Room *> &getRoomsMap() const;
    const sf::Vector2i &getFirstRoomCoord() const;
    const sf::Vector2i &getCurrentRoomCoord() const;
    sf::Vector2f getRoomPixelsSize() const;

    void enableRoom(const sf::Vector2i &coord, const sf::Vector2f &target_position);

private:
    void createRoom(int32_t index, const Tmx::Group &rooms_group);
    void createRoomTilemapLayers(int32_t index, const std::vector<Tmx::Layer> &layers);
    void createRoomJumpBlocks(int32_t index, const Tmx::ObjectGroup &exit_blocks_object_group);
    void createRoomCollisions(int32_t index, const Tmx::ObjectGroup &collisions_object_group);
    void createRoomEntities(int32_t index, const Tmx::ObjectGroup &entities_object_group);

    std::pair<Entity, const Tmx::Tileset *> createTileMapFromLayer(const Tmx::Layer &layer);
    const Tmx::Tileset *getTilesetByGid(int32_t gid);

public:
    Signal<const sf::Vector2i &> room_opened;
    Signal<const sf::Vector2i &> room_enabled;

private:
    World *m_world;
    b2::DynamicTree<Entity> *m_scene_tree;
    std::unique_ptr<Tmx> m_level_tmx;

    struct RoomsCache
    {
        void clear();
        void enableRoom(const sf::Vector2i &room_coord);
        void disableRoom(const sf::Vector2i &room_coord);

        Vector2D<Room *> map;
        Vector2D<std::vector<Entity> *> entities;
        sf::Vector2i m_first_room_coord;
    } m_rooms_cache;

    sf::Vector2i m_current_room_coord;

    Entity m_player_entity;
};

} // namespace fck

#endif // LEVEL_ZKKSOSIXHXVE_H
