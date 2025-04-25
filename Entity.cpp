#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Utility.h"
#include <windows.h>

#define LOG(argument) std::cout << argument << '\n'


void Entity::ai_activate(Entity* player)
{
    if (m_animation_state == STATE_HURT || m_animation_state == STATE_DEATH) {
        // Just maintain current state
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        return;
    }
    if (m_animation_state == STATE_ATTACKING || m_animation_state == STATE_SUPER_ATTACK) {
        // If the attack animation is done, return to idle
        if (m_animation_index >= m_animation_frames - 1) {
            set_animation_state(STATE_IDLE);
        }
        return;
    }
    switch (m_ai_type)
    {
    case WALKER:
        ai_walk();
        break;

    case GUARD:
        ai_guard(player);
        break;


    case SHOOTER:
        ai_shoot(player);
        break;

    default:
        break;
    }
}

void Entity::ai_walk()
{
    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Entity::ai_guard(Entity* player) {
    // Don't change behavior if in a special animation state
    if (m_animation_state == STATE_ATTACKING ||
        m_animation_state == STATE_HURT ||
        m_animation_state == STATE_DEATH || 
        m_animation_state == STATE_SUPER_ATTACK) {
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        return;
    }

    float distance = glm::distance(m_position, player->get_position());
    bool player_attacking = (player->get_animation_state() == STATE_ATTACKING);
    bool facing_player = ((m_position.x > player->get_position().x && m_movement.x < 0) ||
        (m_position.x < player->get_position().x && m_movement.x > 0));

    // follow player if within certain range
    // Defensive behaviors
    if (player_attacking && distance < 1.0f) {
        // Dodge roll away if player is attacking nearby
        if (rand() % 100 < 10) { // 
            m_movement = (m_position.x > player->get_position().x) ?
                glm::vec3(1.0f, 0.0f, 0.0f) : // Roll right
                glm::vec3(-1.0f, 0.0f, 0.0f); // Roll left
            //m_is_jumping = true;
            return;
        }
        // Otherwise block (reduce incoming damage)
    }

    // Attack patterns
    if (distance < 2.0f) {
        // Close combat 
        //if (rand() % 100 < 20) { // 20% chance to attack each frame when close
        //    //set_animation_state(STATE_ATTACKING);
        //    m_movement = glm::vec3(0.0f);
        //    return;
        //}
        m_movement = glm::vec3(0.0f);
    }
    else if (distance < 7.0f) {
        // Medium range - approach carefully
        if (facing_player) {
            m_movement = (m_position.x > player->get_position().x) ?
                glm::vec3(-1.0f, 0.0f, 0.0f) : // Move left slower
                glm::vec3(1.0f, 0.0f, 0.0f);   // Move right slower
        }
        else {
            // Face player first before moving
            m_movement.x = (m_position.x > player->get_position().x) ? -1.0f : 1.0f;
        }

        // Occasionally jump to avoid being predictable ok maybe fix this later bc this would send then rocketing LOL
        /*if (rand() % 100 < 20 && m_collided_bottom) {
            m_is_jumping = true;
            m_movement = glm::vec3(0.0f, 2.0f, 0.0f);
        }*/
    }
    else {
        // Far away - move toward player more aggressively
        m_movement = (m_position.x > player->get_position().x) ?
            glm::vec3(-2.0f, 0.0f, 0.0f) :
            glm::vec3(2.0f, 0.0f, 0.0f);

       
    }

    
}




void Entity::ai_shoot(Entity* player) {
    if (m_animation_state == STATE_ATTACKING ||
        m_animation_state == STATE_HURT ||
        m_animation_state == STATE_DEATH || m_animation_state == STATE_SUPER_ATTACK) {
        m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        return;
    }

    float distance = glm::distance(m_position, player->get_position());
    static const float PLATFORM_LEFT_EDGE = 4.0f;  // Left edge of the platform
    static const float PLATFORM_RIGHT_EDGE = 12.0f; // Right edge of the platform
    float edge_threshold = 2.0f; // How close the enemy needs to be to the edge to trigger jumping

    bool near_left_edge = m_position.x <= PLATFORM_LEFT_EDGE + edge_threshold;
    bool near_right_edge = m_position.x >= PLATFORM_RIGHT_EDGE - edge_threshold;

    // If too close to the edge and player is approaching, jump over the player
    if ((near_left_edge || near_right_edge) && distance < 3.0f) {
        // Determine jump direction (away from the edge)
        if (near_left_edge) {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f); // Jump right
        }
        else if (near_right_edge) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f); // Jump left
        }
        //m_is_jumping = true; // Trigger jump
        return;
    }

    // If within shooting range but not too close
    if (distance < 10.0f && distance > 3.0f) {
        m_movement = glm::vec3(0.0f);  // Stop moving
    }
    // If too close, back away
    else if (distance <= 3.0f) {
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);  // Move right
        }
        else {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f); // Move left
        }
    }
    // If too far, approach player
    else {
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f); // Move left
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);  // Move right
        }
    }
}


void Entity::shoot(glm::vec3 position, glm::vec3 direction, float speed)
{
    //m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
    m_position = position;
    m_movement = direction;
}
void Entity::add_animation_texture(AnimationState state, GLuint texture_id, int cols, int rows) {
    m_state_textures[state] = texture_id;
    m_state_frames[state] = glm::ivec2(cols, rows);

    if (m_state_textures.size() == 1) {
        set_animation_state(state);
    }
}

void Entity::set_animation_state(AnimationState state) {
    if (m_animation_state == state) return;

    m_animation_state = state;

    if (m_state_textures.find(state) == m_state_textures.end()) {
        return; 
    }

    m_texture_id = m_state_textures[state];
    glm::ivec2 frames = m_state_frames[state];
    m_animation_cols = frames.x;
    m_animation_rows = frames.y;
    m_animation_frames = frames.x * frames.y;
    m_animation_index = 0;  
    m_animation_time = 0.0f;  

    if (m_animation_indices) delete[] m_animation_indices;
    m_animation_indices = new int[m_animation_frames];

    for (int i = 0; i < m_animation_frames; i++) {
        m_animation_indices[i] = i;
    }
}


// Default constructor
Entity::Entity() : 
    m_position(0.0f), m_movement(0.0f),
    m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f),
    m_animation_cols(1), m_animation_frames(1), // Default to 1 frame
    m_animation_index(0), m_animation_rows(1),
    m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f),
    m_acceleration(0.0f),
    m_width(1.0f), m_height(1.0f),
    m_animation_state(STATE_IDLE) // Initialize player state
{
    // Initialize animation indices for single frame
    m_animation_indices = new int[1] {0};
}

// Main constructor
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration,
    float jump_power, float width, float height, EntityType entityType) :
    Entity() // Delegate to default constructor
{
    m_texture_id = texture_id;
    m_speed = speed;
    m_acceleration = acceleration;
    m_jumping_power = jump_power;
    m_width = width;
    m_height = height;
    m_entity_type = entityType;

    // Initialize with single frame animation
    m_animation_indices = new int[1] {0};
    m_animation_cols = 1;
    m_animation_rows = 1;
    m_animation_frames = 1;
}

// Simpler constructor implementation
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType entityType) :
    Entity() // Delegate to default constructor
{
    m_texture_id = texture_id;
    m_speed = speed;
    m_width = width;
    m_height = height;
    m_entity_type = entityType;

    // Initialize with single frame animation
    m_animation_indices = new int[1] {0};
    m_animation_cols = 1;
    m_animation_rows = 1;
    m_animation_frames = 1;
}



Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AnimationState AIState) : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType), m_ai_type(AIType), m_animation_state(AIState)
{
    
}


Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
            }
            else if (m_velocity.y < 0)
            {
                m_position.y += y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_bottom = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x > 0)
            {
                m_position.x -= x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_right = true;

            }
            else if (m_velocity.x < 0)
            {
                m_position.x += x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_left = true;
            }
        }
    }
}

void const Entity::check_collision_y(Map* map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;

    }
}

void const Entity::check_collision_x(Map* map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

void const Entity::check_collision_with_enemies(Entity* enemies, int enemy_count) {
    for (int i = 0; i < enemy_count; i++) {
        if (check_collision(&enemies[i])) {
            // handle collision from player's perspective
            if (m_entity_type == PLAYER) {
                m_collided_left = (m_velocity.x < 0);
                m_collided_right = (m_velocity.x > 0);
                return; // return after first collision
            }
        }
    }
}


bool Entity::check_attack_collision(Entity* other) const {
    // attack range can be slightly larger than regular collision
    float attack_range = 1.5f; 

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width * attack_range + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    if (!m_is_active) return;


    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    if (m_entity_type == PLAYER) {
        // handle player animation state based on movement
        if (m_animation_state == STATE_SUPER_ATTACK) {
            m_is_attacking = true;

            // Don't change state until animation is done
            if (m_animation_index >= m_animation_frames - 1) {
                set_animation_state(STATE_IDLE);
                m_is_attacking = false;

            }
        }
        else if (m_animation_state == STATE_ATTACKING) {
            m_is_attacking = true;
            // reached end of attack animation
            if (m_animation_index >= m_animation_frames - 1) {
                set_animation_state(STATE_IDLE);
                m_is_attacking = false;
            }
        }
        else if (m_animation_state == STATE_HURT) {
            // Don't change state until hurt animation is done
            if (m_animation_index >= m_animation_frames - 1) {
                set_animation_state(STATE_IDLE);
            }
        }
        else if (m_velocity.y > 0.1f) {
            set_animation_state(STATE_JUMPING);
        }
        else if (m_velocity.y < -0.1f) {
            set_animation_state(STATE_FALLING);
        }
        else if (fabs(m_velocity.x) > 0.1f) {
            if (m_velocity.x > 0) {
                set_animation_state(STATE_RUNNING_RIGHT);
            }
            else {
                set_animation_state(STATE_RUNNING_LEFT);
            }
        }
        else {
            set_animation_state(STATE_IDLE);
        }
    }
    else if (m_entity_type == ENEMY) {

        // Only call AI if not in a special state
        if (m_animation_state != STATE_ATTACKING &&
            m_animation_state != STATE_HURT &&
            m_animation_state != STATE_DEATH &&
            m_animation_state != STATE_SUPER_ATTACK) {
            ai_activate(player);

            // Set movement animations only if not in a special state
            if (m_movement.x < -0.1f) {
                set_animation_state(STATE_RUNNING_LEFT);
            }
            else if (m_movement.x > 0.1f) {
                set_animation_state(STATE_RUNNING_RIGHT);
            }
            else if (m_velocity.y > 0.1f) {
                set_animation_state(STATE_JUMPING);
            }
            else if (m_velocity.y < -0.1f) {
                set_animation_state(STATE_FALLING);
            }
            else {
                set_animation_state(STATE_IDLE);
            }
        }
        // Check if special animations have completed
        else if ((m_animation_state == STATE_ATTACKING || m_animation_state == STATE_HURT || m_animation_state == STATE_SUPER_ATTACK) &&
            m_animation_index >= m_animation_frames - 1) {
            set_animation_state(STATE_IDLE);
        }
        if (m_animation_state == STATE_IDLE) {
            set_scale(2.0f);  // Reset enemy to normal scale immediately after super finishes
            //glm::vec3 pos = get_position();
            //set_position(glm::vec3(pos.x, -2.9f, pos.z));
        }
    }

    // Animation frame update
    if (m_animation_indices != NULL && m_animation_frames > 0)
    {
        m_animation_time += delta_time;
        float frames_per_second = 1.0f / SECONDS_PER_FRAME;

        if (m_animation_time >= frames_per_second)
        {
            m_animation_time = 0.0f;
            m_animation_index++;

            if (m_animation_index >= m_animation_frames)
            {
                // Only reset for looping animations
                if (m_animation_state != STATE_DEATH && m_entity_type != EFFECT) {
                    m_animation_index = 0;
                }
                else {
                    m_animation_index = m_animation_frames - 1; // Stay on last frame for death
                }
            }
        }
    }


    
    m_velocity.x = m_movement.x * m_speed;
    m_velocity += m_acceleration * delta_time;
    

    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    m_position.y += m_velocity.y * delta_time;
    m_position.x += m_velocity.x * delta_time;

    if (m_entity_type == PLAYER) {
        // Player checks collisions with enemies
        check_collision_with_enemies(collidable_entities, collidable_entity_count);
    }
    
    else if (m_entity_type != EFFECT) {
        // Enemies check collisions normally
        check_collision_y(collidable_entities, collidable_entity_count);
        check_collision_x(collidable_entities, collidable_entity_count);
    }

    if (m_entity_type != EFFECT) {
        check_collision_y(map);
        check_collision_x(map);
    }
    
    


    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);  // Apply scaling
}


void Entity::render(ShaderProgram* program)

{
    
    program->set_model_matrix(m_model_matrix);
    /*if (m_entity_type == EFFECT) {
        OutputDebugString(("Lightning frame: " +
            std::to_string(m_animation_index)  + "\n").c_str());
    }*/

    if (m_animation_indices != NULL && m_animation_frames > 0) {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
    }
    else {

        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }

    //__debugbreak();
}


// super stuff

// In Entity.cpp
void Entity::set_hits_needed_for_super(int hits) {
    m_hits_needed_for_super = hits;
    m_hits_landed = 0;
    m_super_ready = false;
}

void Entity::set_super_damage(float damage) {
    m_super_damage = damage;
}

void Entity::increment_hits_landed() {
    m_hits_landed++;
    if (m_hits_landed >= m_hits_needed_for_super) {
        m_super_ready = true;
        // Optional: Set a visual indicator
        // set_animation_state(STATE_SUPER_ATTACK_READY);
    }
}



bool Entity::is_super_ready() const {
    return m_super_ready;
}

void Entity::use_super_attack() {
    set_animation_state(STATE_SUPER_ATTACK);
    m_super_ready = false;
    m_hits_landed = 0;
    
}
