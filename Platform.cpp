// Platform.cpp
#include "Platform.h"

Platform::Platform(GLuint texture_id, glm::vec3 position, float width, float height)
    : Entity(texture_id, 0.0f, width, height, PLATFORM)
{
    set_position(position);
}

void Platform::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map) {
    // Platforms are static, so no update needed
    // But we call parent to maintain any base behavior
    Entity::update(delta_time, player, collidable_entities, collidable_entity_count, map);
}