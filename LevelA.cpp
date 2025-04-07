#include "LevelA.h"
#include "Utility.h"
#include "Entity.h"


#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8
#define LOG(argument) std::cout << argument << '\n'


constexpr char SPRITESHEET_FILEPATH[] = "assets/george_0.png",
ENEMY_FILEPATH[] = "assets/black_cat.png";



unsigned int LEVELA_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelA::~LevelA()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.level_up_sfx);  
    Mix_FreeChunk(m_game_state.punch_sfx); 


}

void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);

    // extra platforms (not using Map class)
    GLuint floating_grass_texture_id = Utility::load_texture("assets/floating_grass_tile.png");
    m_platforms.push_back(new Platform(floating_grass_texture_id, glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, 0.5f));
    

    // PLAYER CODE

    GLuint idle_texture = Utility::load_texture("assets/idle.png");
    GLuint run_left_texture = Utility::load_texture("assets/run_left.png");
    GLuint run_right_texture = Utility::load_texture("assets/run_right.png");
    GLuint jump_texture = Utility::load_texture("assets/jump.png");
    GLuint fall_texture = Utility::load_texture("assets/fall.png");

    glm::vec3 acceleration = glm::vec3(0.0f, -9.8f, 0.0f);


    m_game_state.player = new Entity(
        idle_texture,
        5.0f,
        glm::vec3(0.0f, -9.8f, 0.0f),
        5.0f,
        1.5f,
        1.8f,
        PLAYER);
    
    m_game_state.player->set_scale(2.0f);

    // Set up state textures
    m_game_state.player->add_state_texture(PLAYER_IDLE, idle_texture, 4, 1);
    m_game_state.player->add_state_texture(RUNNING_LEFT, run_left_texture, 8, 1);
    m_game_state.player->add_state_texture(RUNNING_RIGHT, run_right_texture, 8, 1);
    m_game_state.player->add_state_texture(JUMPING, jump_texture, 3, 1);
    m_game_state.player->add_state_texture(FALLING, fall_texture, 2, 1);

    // Set initial state
    m_game_state.player->set_player_state(PLAYER_IDLE);


    m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));


    // ENEMIES

    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    if (enemy_texture_id == 0) {
        __debugbreak();
    }

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.7f, 0.7f, ENEMY, GUARD, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(7.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].activate();


    /**
     BGM and SFX
     */
    

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
    m_game_state.level_up_sfx = Mix_LoadWAV("assets/level_up.wav");
    m_game_state.punch_sfx = Mix_LoadWAV("assets/punch.wav"); 

}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (m_game_state.player->check_collision(&m_game_state.enemies[i])) {
            //m_game_state.next_scene_id = 0;  // temp -> gonna change to taking a life later
            //return;  // Exit immediately after collision
            *lives -= 1;
            Mix_PlayChannel(1, m_game_state.punch_sfx, 0);  // Play punch sound
            if (*lives == 0) {
                m_game_state.next_scene_id = 4; // render lose scene
                *lives = 3;
                return;
            }
            m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));
            m_game_state.enemies[i].set_position(glm::vec3(7.0f, 0.0f, 0.0f));


        }
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    }

    for (Platform* platform : m_platforms) {
        m_game_state.player->check_collision(platform);
    }


    if (m_game_state.player->get_position().y < -10.0f) {
        m_game_state.next_scene_id = 2;
        Mix_PlayChannel(-1, m_game_state.level_up_sfx, 0);  // Play level-up sound
    }
}

void LevelA::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);

    // render enemies
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(program);
    }

    // Render platforms
    for (Platform* platform : m_platforms) {
        platform->render(program);
    }

    if (lives != nullptr) {
        std::string lives_text = "Lives: " + std::to_string(*lives);
        Utility::draw_text(program, Utility::load_texture("assets/font_sheet2.png"),
            lives_text, 0.5f, 0.0f, glm::vec3(3.0f, -0.5f, 0.0f));
    }


}