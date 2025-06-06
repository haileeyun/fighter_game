#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Utility.h"
#include "Entity.h"
#include "Map.h"


struct GameState
{
    // ����� GAME OBJECTS ����� //
    Map* map;
    Entity* player;
    Entity* enemies;

    // ����� AUDIO ����� //
    Mix_Chunk* jump_sfx;
    Mix_Chunk* level_up_sfx;
    Mix_Chunk* punch_sfx;

    // ����� POINTERS TO OTHER SCENES ����� //
    int next_scene_id;

    // local stats
    int player_health;
    int player_super;
};

class Scene {
protected:
    GameState m_game_state;

public:
    // ����� ATTRIBUTES ����� //
    int m_number_of_enemies = 1;
    int* lives;

    // ����� METHODS ����� //
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram* program) = 0;

    // ����� GETTERS ����� //
    GameState const get_state() const { return m_game_state; }
    GameState& get_state() { return m_game_state; }
    int const get_number_of_enemies() const { return m_number_of_enemies; }

    // SETTERS
    void set_lives(int* lives) { this->lives = lives; }
    GLuint background_texture_id;



};