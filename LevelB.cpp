#include "LevelB.h"
#include "Utility.h"
#include "Entity.h"


#define LEVEL_WIDTH 15
#define LEVEL_HEIGHT 9
#define LOG(argument) std::cout << argument << '\n'

// CONSTANTS

static glm::vec3 INIT_PLAYER_POSITION = glm::vec3(3.0f, 0.0f, 0.0f);
static glm::vec3 INIT_ENEMY_POSITION = glm::vec3(12.0f, 0.0f, 0.0f);
static float BULLET_SPEED = 20.0f;
static int DAMAGE_TO_ENEMY = 50;
static float ENEMY_SPEED = 2.0f;
static float PLAYER_SPEED = 3.0f;


static bool damage_applied = false;


unsigned int LEVELB_DATA[] =
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

LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete m_bullet;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.level_up_sfx);
    Mix_FreeChunk(m_game_state.punch_sfx);
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;

    // initialize map

    GLuint map_texture_id = Utility::load_texture("assets/tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 5, 1);


    // PLAYER CODE

    GLuint idle_texture = Utility::load_texture("assets/idle.png");
    GLuint run_left_texture = Utility::load_texture("assets/run_left.png");
    GLuint run_right_texture = Utility::load_texture("assets/run_right.png");
    GLuint jump_texture = Utility::load_texture("assets/jump.png");
    GLuint fall_texture = Utility::load_texture("assets/fall.png");
    GLuint attack_texture = Utility::load_texture("assets/attack.png");
    GLuint hurt_texture = Utility::load_texture("assets/hurt.png");


    glm::vec3 acceleration = glm::vec3(0.0f, -9.8f, 0.0f);


    m_game_state.player = new Entity(idle_texture, PLAYER_SPEED, glm::vec3(0.0f, -9.8f, 0.0f), 7.0f, 1.5f, 1.8f, PLAYER);

    m_game_state.player->set_scale(2.0f);

    m_game_state.player->add_animation_texture(STATE_IDLE, idle_texture, 4, 1);
    m_game_state.player->add_animation_texture(STATE_RUNNING_LEFT, run_left_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_RUNNING_RIGHT, run_right_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_JUMPING, jump_texture, 3, 1);
    m_game_state.player->add_animation_texture(STATE_FALLING, fall_texture, 2, 1);
    m_game_state.player->add_animation_texture(STATE_ATTACKING, attack_texture, 8, 1);
    m_game_state.player->add_animation_texture(STATE_HURT, hurt_texture, 1, 1);

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
        m_game_state.enemies[i] = Entity(enemy_idle_texture, ENEMY_SPEED, 0.5f, 1.7f, ENEMY, SHOOTER, STATE_IDLE);
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
    m_game_state.enemies[0].update(0.01f, m_game_state.player, NULL, 0, m_game_state.map); 


    m_game_state.enemies[0].set_position(INIT_ENEMY_POSITION);
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].activate();
    m_game_state.enemies[0].set_scale(2.0f);


    // BULLETS
    
    // Create bullets
    GLuint bullet_texture = Utility::load_texture("assets/dagger.png");
    m_bullet = new Entity(bullet_texture, BULLET_SPEED, 0.5f, 0.5f, PROJECTILE);
    m_bullet->set_position(glm::vec3(0.0f)); // might remove this
    m_bullet->set_scale(glm::vec3(0.5f, 0.5f, 1.0f));
    m_bullet->deactivate();


    /**
     BGM and SFX
     */

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    m_game_state.level_up_sfx = Mix_LoadWAV("assets/level_up.wav");
    m_game_state.punch_sfx = Mix_LoadWAV("assets/punch.wav");

}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    enemy_attack_cooldown -= delta_time;
    if (m_bullet->get_position().x <= -5.0f || m_bullet->get_position().x >= 20.0f || m_bullet->get_position().y <= -20.0f || m_bullet->get_position().y >= 20.0f) {
        m_bullet->deactivate(); // deactivate if the bullet goes out of bounds
        m_bullet->set_position(glm::vec3(0.0f));
    }

    // check if bullet hits the player
    if (m_bullet->is_active()) {
        if (m_game_state.player->check_collision(m_bullet)) {
            // subtract health, deactivate bullet, dont render, reset position?
            m_game_state.player->damage(1); // damage player
            m_bullet->deactivate(); // deactivate
            m_bullet->set_position(glm::vec3(0.0f));
            glm::vec3 knockback_dir = glm::normalize(m_game_state.player->get_position() - m_game_state.enemies[0].get_position());
            knockback_dir.y = 1.0f;
            m_game_state.player->apply_knockback(knockback_dir, 3.0f);

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
                m_game_state.enemies[0].set_position(INIT_ENEMY_POSITION);
            }

        }
    }
    

    for (int i = 0; i < ENEMY_COUNT; i++) {
        // Check if enemy is still active/alive
        //m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

        if (m_game_state.enemies[i].get_health() <= 0) {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

            // If enemy is dead but not in death animation yet
            if (m_game_state.enemies[i].get_animation_state() != STATE_DEATH) {
                m_game_state.enemies[i].set_animation_state(STATE_DEATH);
                m_game_state.enemies[i].set_movement(glm::vec3(0.0f)); // Stop movement
            }
            // If death animation is finished, move to next level
            if (m_game_state.enemies[i].get_animation_index() >= m_game_state.enemies[i].get_animation_frames() - 1) {
                m_game_state.next_scene_id = 3; // Move to levelC
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


            // enemy attacking player
            if (glm::distance(m_game_state.player->get_position(), m_game_state.enemies[i].get_position()) < 9.0f) {
                // If enemy is very close and cooldown is finished
                if (enemy_attack_cooldown <= 0 && m_game_state.enemies[i].get_animation_state() != STATE_ATTACKING && !(m_bullet->is_active())) {
                    m_game_state.enemies[i].set_animation_state(STATE_ATTACKING);
                    enemy_attack_cooldown = 2.0f; // 2 second cooldown
                }

                // send bullet in middle of attack animation
                if (m_game_state.enemies[i].get_animation_state() == STATE_ATTACKING && m_game_state.enemies[i].get_animation_index() == m_game_state.enemies[i].get_animation_frames() / 2) {
                    glm::vec3 bullet_pos = m_game_state.enemies[i].get_position();
                    glm::vec3 direction;

                    // Determine direction based on enemy facing
                    if (m_game_state.player->get_position().x < m_game_state.enemies[i].get_position().x) {
                        bullet_pos.x -= 0.5f;
                        direction = glm::vec3(-1.0f, 0.0f, 0.0f);
                    }
                    else {
                        bullet_pos.x += 0.5f;
                        direction = glm::vec3(1.0f, 0.0f, 0.0f);
                    }
                    // activate the bullet, shoot it
                    m_bullet->activate();
                    m_bullet->shoot(bullet_pos, direction, BULLET_SPEED);
                    
                    
                }
            }

            // Update enemy
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        }
    }
    m_bullet->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

}

void LevelB::render(ShaderProgram* program)
{

    m_game_state.map->render(program);
    m_game_state.player->render(program);

    // render enemies
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(program);
    }
    if (m_bullet->is_active()) {
        m_bullet->render(program);
    }


    Mix_PlayChannel(2, m_game_state.punch_sfx, 0);




}