// Platform.h
#pragma once
#include "Entity.h"
#include "Map.h"

class Platform : public Entity {
public:
    Platform(GLuint texture_id, glm::vec3 position, float width, float height);
    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map);
};