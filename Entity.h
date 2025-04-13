#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include <unordered_map>

enum EntityType { PLATFORM, PLAYER, ENEMY };

enum AIType { WALKER, GUARD, FLYER, SHOOTER };
//enum AIState { AI_WALKING, AI_IDLE, AI_ATTACKING, AI_HURT, AI_DEATH };

enum AnimationState {
    STATE_IDLE,
    STATE_WALKING,
    STATE_RUNNING_LEFT,
    STATE_RUNNING_RIGHT,
    STATE_JUMPING,
    STATE_FALLING,
    STATE_ATTACKING,
    STATE_HURT,
    STATE_DEATH
};

class Entity
{
private:
    bool m_is_active = true;

    EntityType m_entity_type;
    AIType m_ai_type;
    //AIState m_ai_state;

    AnimationState m_animation_state;
    std::unordered_map<AnimationState, GLuint> m_state_textures;
    std::unordered_map<AnimationState, glm::ivec2> m_state_frames;


    //  TRANSFORMATIONS  //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;

    float     m_speed,
        m_jumping_power;

    bool m_is_jumping;

    // TEXTURES //
    GLuint    m_texture_id;

    //  ANIMATION  //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
        m_height = 1.0f;

    //  COLLISIONS //
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    void update_model_matrix() {
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    }

    

public:
    // STATIC VARIABLES  //
    static constexpr int SECONDS_PER_FRAME = 8; 


    // METHODS  //
    Entity();
    
    

    Entity(GLuint texture_id, float speed, glm::vec3 acceleration,
        float jump_power, float width, float height, EntityType EntityType);

    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AnimationState AIState); // AI constructor
    
    ~Entity();

    // animation methods
    void set_animation_state(AnimationState state);
    void add_animation_texture(AnimationState state, GLuint texture_id, int cols, int rows);

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;

    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    // Overloading our methods to check for only the map
    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map);
    void render(ShaderProgram* program);

    void ai_activate(Entity* player);
    void ai_walk();
    void ai_guard(Entity* player);
    void ai_fly();

    void const check_collision_with_enemies(Entity* enemies, int enemy_count);


    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    
    void move_left() { m_movement.x = -1.0f; }
    void move_right() { m_movement.x = 1.0f; }
    void move_up() { m_movement.y = 1.0f; }
    void move_down() { m_movement.y = -1.0f; }

    void const jump() { m_is_jumping = true; }

    // GETTERS  //
    EntityType const get_entity_type()    const { return m_entity_type; };
    AIType     const get_ai_type()        const { return m_ai_type; };
    //AIState    const get_ai_state()       const { return m_ai_state; };
    float const get_jumping_power() const { return m_jumping_power; }
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    float get_height() const { return m_height; }

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };
    //  SETTERS //
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    //void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; update_model_matrix();
    }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; update_model_matrix(); }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; }
    void const set_width(float new_width) { m_width = new_width; }
    void const set_height(float new_height) { m_height = new_height; }
    void set_scale(float scale) { m_scale = glm::vec3(scale, scale, 1.0f); }
    void set_collided_bottom(bool collided) { m_collided_bottom = collided; }

    
   

};

#endif // ENTITY_H