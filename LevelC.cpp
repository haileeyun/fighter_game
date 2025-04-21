#include "LevelC.h"
#include "Utility.h"
#include "Entity.h"


#define LEVEL_WIDTH 15
#define LEVEL_HEIGHT 9
#define LOG(argument) std::cout << argument << '\n'

// CONSTANTS

static glm::vec3 INIT_PLAYER_POSITION = glm::vec3(3.0f, 0.0f, 0.0f);
static glm::vec3 INIT_ENEMY_POSITION = glm::vec3(12.0f, 0.0f, 0.0f);
static int DAMAGE_TO_ENEMY = 50;
static float ENEMY_SPEED = 2.0f;
static float PLAYER_SPEED = 3.0f;


static bool damage_applied = false;


unsigned int LEVELC_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0,
    0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

LevelC::~LevelC()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.level_up_sfx);
    Mix_FreeChunk(m_game_state.punch_sfx);
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;

    // initialize map

    GLuint map_texture_id = Utility::load_texture("assets/tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 5, 1);


    // PLAYER CODE

    GLuint idle_texture = Utility::load_texture("assets/metal_idle (2).png");
    GLuint run_left_texture = Utility::load_texture("assets/metal_run_left (2).png");
    GLuint run_right_texture = Utility::load_texture("assets/metal_run_right (2).png");
    GLuint jump_texture = Utility::load_texture("assets/metal_jump (2).png");
    GLuint fall_texture = Utility::load_texture("assets/metal_fall (2).png");
    GLuint attack_texture = Utility::load_texture("assets/metal_basic_attack (2).png");
    GLuint hurt_texture = Utility::load_texture("assets/metal_hurt (2).png");


    glm::vec3 acceleration = glm::vec3(0.0f, -9.8f, 0.0f);


    m_game_state.player = new Entity(idle_texture, PLAYER_SPEED, glm::vec3(0.0f, -9.8f, 0.0f), 5.0f, 1.0f, 1.8f, PLAYER);


    m_game_state.player->set_scale(2.0f);

    m_game_state.player->add_animation_texture(STATE_IDLE, idle_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_RUNNING_LEFT, run_left_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_RUNNING_RIGHT, run_right_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_JUMPING, jump_texture, 3, 1);
    m_game_state.player->add_animation_texture(STATE_FALLING, fall_texture, 3, 1);
    m_game_state.player->add_animation_texture(STATE_ATTACKING, attack_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_HURT, hurt_texture, 6, 1);

    // Set initial state
    m_game_state.player->set_animation_state(STATE_IDLE);

    m_game_state.player->set_position(INIT_PLAYER_POSITION);


    // ENEMIES

    GLuint enemy_idle_texture = Utility::load_texture("assets/mushroom_idle.png");
    GLuint enemy_run_left_texture = Utility::load_texture("assets/mushroom_run_left.png");
    GLuint enemy_run_right_texture = Utility::load_texture("assets/mushroom_run_right.png");
    GLuint enemy_attack_texture = Utility::load_texture("assets/mushroom_attack.png");
    GLuint enemy_hurt_texture = Utility::load_texture("assets/mushroom_hit.png");
    GLuint enemy_death_texture = Utility::load_texture("assets/mushroom_die.png");


    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_idle_texture, 1.0f, 0.5f, 1.7f, ENEMY, GUARD, STATE_IDLE);
        m_game_state.enemies[i].set_health(100); // Set initial health
    }

    m_game_state.enemies[0].add_animation_texture(STATE_IDLE, enemy_idle_texture, 7, 1);
    m_game_state.enemies[0].add_animation_texture(STATE_RUNNING_LEFT, enemy_run_left_texture, 8, 1);
    m_game_state.enemies[0].add_animation_texture(STATE_RUNNING_RIGHT, enemy_run_right_texture, 8, 1);
    m_game_state.enemies[0].add_animation_texture(STATE_ATTACKING, enemy_attack_texture, 10, 1);
    m_game_state.enemies[0].add_animation_texture(STATE_HURT, enemy_hurt_texture, 5, 1);
    m_game_state.enemies[0].add_animation_texture(STATE_DEATH, enemy_death_texture, 15, 1);

    // Set initial state
    m_game_state.enemies[0].set_animation_state(STATE_IDLE);
    m_game_state.enemies[0].update(0.01f, m_game_state.player, NULL, 0, m_game_state.map); // idkkk


    m_game_state.enemies[0].set_position(INIT_ENEMY_POSITION);
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].activate();
    m_game_state.enemies[0].set_scale(2.0f);


    /**
     BGM and SFX
     */

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    m_game_state.level_up_sfx = Mix_LoadWAV("assets/level_up.wav");
    m_game_state.punch_sfx = Mix_LoadWAV("assets/punch.wav");

}

void LevelC::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);


    // Check if player has fallen off the platform
    if (m_game_state.player->get_position().y < -15.0f) {
        // Player has fallen, lose a life
        *lives -= 1;

        // Reset player position and health
        m_game_state.player->set_position(INIT_PLAYER_POSITION);
        m_game_state.player->set_health(100);  // Reset health to full

        // If no lives left, go to lose scene
        if (*lives <= 0) {
            m_game_state.next_scene_id = 4;
            *lives = 3;  // Reset lives for next game
            return;
        }
    }

    // cooldown so the enemy doesn't spam attack me    
    enemy_attack_cooldown -= delta_time;


    for (int i = 0; i < ENEMY_COUNT; i++) {
        // Check if enemy is still active/alive
        if (m_game_state.enemies[i].get_health() <= 0) {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

            // If enemy is dead but not in death animation yet
            if (m_game_state.enemies[i].get_animation_state() != STATE_DEATH) {
                m_game_state.enemies[i].set_animation_state(STATE_DEATH);
                m_game_state.enemies[i].set_movement(glm::vec3(0.0f)); // Stop movement
            }
            // If death animation is finished, move to next level
            if (m_game_state.enemies[i].get_animation_index() >= m_game_state.enemies[i].get_animation_frames() - 1) {
                m_game_state.next_scene_id = 5; // win
                Mix_PlayChannel(-1, m_game_state.level_up_sfx, 0);
                return;
            }
        }
        else {
            // Player attacking enemy
            if (m_game_state.player->get_animation_state() == STATE_ATTACKING) {
                if (m_game_state.player->get_animation_index() == m_game_state.player->get_animation_frames() / 2 && !damage_applied) {
                    // Apply damage once
                    if (m_game_state.player->check_attack_collision(&m_game_state.enemies[i])) {
                        m_game_state.enemies[i].damage(20);
                        m_game_state.enemies[i].set_animation_state(STATE_HURT);

                        // Calculate knockback direction
                        glm::vec3 knockback_dir = glm::normalize(m_game_state.enemies[i].get_position() - m_game_state.player->get_position());
                        knockback_dir.y = 1.0f; // Add some upward force
                        m_game_state.enemies[i].apply_knockback(knockback_dir, 3.0f);

                        Mix_PlayChannel(1, m_game_state.punch_sfx, 0);
                        damage_applied = true;
                    }
                }

                // Reset flag when attack animation is done
                if (m_game_state.player->get_animation_index() >= m_game_state.player->get_animation_frames() - 1) {
                    damage_applied = false;
                }
            }

            // Enemy attacking player
            if (glm::distance(m_game_state.player->get_position(), m_game_state.enemies[i].get_position()) < 2.0f) {
                // If enemy is very close and cooldown is finished
                if (enemy_attack_cooldown <= 0 && m_game_state.enemies[i].get_animation_state() != STATE_ATTACKING) {
                    m_game_state.enemies[i].set_animation_state(STATE_ATTACKING);
                    enemy_attack_cooldown = 1.0f; // 1 second cooldown
                }

                // Apply damage at middle of attack animation
                if (m_game_state.enemies[i].get_animation_state() == STATE_ATTACKING && m_game_state.enemies[i].get_animation_index() == m_game_state.enemies[i].get_animation_frames() / 2) {
                    m_game_state.player->damage(1);
                    glm::vec3 knockback_dir = glm::normalize(m_game_state.player->get_position() - m_game_state.enemies[i].get_position());
                    knockback_dir.y = 1.0f;
                    m_game_state.player->apply_knockback(knockback_dir, 3.0f);

                    // If player health reaches 0, lose a life
                    if (m_game_state.player->get_health() <= 0) {
                        *lives -= 1;
                        m_game_state.player->set_health(100); // reset health
                        Mix_PlayChannel(1, m_game_state.punch_sfx, 0);

                        if (*lives == 0) {
                            m_game_state.next_scene_id = 4; // Render lose scene
                            *lives = 3;
                            return;
                        }

                        // Reset positions after losing health
                        m_game_state.player->set_position(INIT_PLAYER_POSITION);
                        m_game_state.enemies[i].set_position(INIT_ENEMY_POSITION);
                    }
                }
            }

            // Update enemy
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        }
    }

}

void LevelC::render(ShaderProgram* program)
{

    m_game_state.map->render(program);
    m_game_state.player->render(program);

    // render enemies
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(program);
    }



}