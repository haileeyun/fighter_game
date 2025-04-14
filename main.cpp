/**
* Author: Hailee Yun
* Assignment: Final Project
* Date due: 2025-04-22, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "menu_screen.h" // start screen
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "lose_scene.h"
#include "win_scene.h"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH = 960,
WINDOW_HEIGHT = 720;

constexpr float BG_RED = 0.678f,
BG_BLUE = 0.902f,
BG_GREEN = 0.847f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene* g_current_scene;
LevelA* g_levelA;
LevelB* g_levelB;
LevelC* g_levelC;
MenuScreen* g_menu_screen;
LoseScene* g_lose_scene;
WinScene* g_win_scene;


glm::vec3 g_camera_position = glm::vec3(0.0f);
float g_camera_zoom = 1.0f;

Effects* g_effects;
Scene* g_levels[6]; 

SDL_Window* g_display_window;


ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

int lives;

Mix_Music* g_bgm = nullptr;

GLuint g_background_texture;
glm::mat4 g_background_matrix;




AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene* scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();



// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene* scene)
{
    if (g_current_scene != nullptr) {
        // Pass the lives pointer to the new scene
        //scene->set_lives(g_current_scene->lives);
    }
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("rise_of_the_ai",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    //glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    g_background_texture = Utility::load_texture("assets/sky.png");
    g_background_matrix = glm::mat4(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_menu_screen = new MenuScreen();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    g_lose_scene = new LoseScene();
    g_win_scene = new WinScene();


    g_levels[0] = g_menu_screen; 
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    g_levels[4] = g_lose_scene;
    g_levels[5] = g_win_scene;


    // start at menu_screen
    switch_to_scene(g_levels[0]);
    lives = 3;
    g_levels[1]->set_lives(&lives);
    g_levels[2]->set_lives(&lives);
    g_levels[3]->set_lives(&lives);

    g_effects = new Effects(g_projection_matrix, g_view_matrix);
    //g_effects->start(SHRINK, 2.0f);


    // AUDIO
    Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);
    Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

    // Load music once
    g_bgm = Mix_LoadMUS("assets/drums.wav");
    if (!g_bgm) {
        printf("Failed to load music: %s\n", Mix_GetError());
    }

    // Start playing (looped)
    Mix_PlayMusic(g_bgm, -1); // -1 = infinite loop
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // 50% volume
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));

    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_UP:
                // Jump
                if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
                    if (g_current_scene->get_state().player->get_collided_bottom())
                    {
                        g_current_scene->get_state().player->jump();
                        Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                    }
                }

                break;

            case SDLK_x:
                if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
                    //g_current_scene->get_state().player->set_animation_state(STATE_ATTACKING);
                    if (g_current_scene->get_state().player->get_animation_state() != STATE_ATTACKING) {
                        g_current_scene->get_state().player->set_animation_state(STATE_ATTACKING);
                        g_current_scene->get_state().player->start_attack();
                    }
                }
                break;

            case SDLK_RETURN:
                if (g_current_scene == g_menu_screen) {
                    switch_to_scene(g_levelA);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }
    if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
        const Uint8* key_state = SDL_GetKeyboardState(NULL);

        if (key_state[SDL_SCANCODE_LEFT])        g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_RIGHT])  g_current_scene->get_state().player->move_right();

        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }

}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    // Reset view matrix for all scenes
    g_view_matrix = glm::mat4(1.0f);

    if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
        
        glm::vec3 player_position = g_current_scene->get_state().player->get_position();
        bool has_active_enemies = false;
        glm::vec3 enemy_position(0.0f);
        // check for enemy
        if (g_current_scene->get_state().enemies[0].get_health() > 0) {
            enemy_position = g_current_scene->get_state().enemies[0].get_position();
            has_active_enemies = true;
        }
        if (has_active_enemies) {
            // Calculate midpoint between player and enemy
            glm::vec3 midpoint = (player_position + enemy_position) * 0.5f;

            // Calculate distance between fighters
            float distance = glm::distance(player_position, enemy_position);

            // Base zoom level - smaller value = more zoomed out
            float base_zoom = 0.5f;

            // Calculate dynamic zoom factor based on distance
            // The max function ensures we don't zoom in too close
            // The min function ensures we don't zoom out too far
            float zoom_factor = glm::clamp(8.0f / (distance + 4.0f), 0.3f, 1.0f); // need to check with cruz if i can use clamp



            // Apply zoom and center on midpoint
            //g_view_matrix = glm::scale(g_view_matrix, glm::vec3(zoom_factor, zoom_factor, 1.0f));
            //g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-midpoint.x, 3.75f, 0.0f));


            // Then modify the camera code to smoothly transition:
            glm::vec3 target_position = glm::vec3(-midpoint.x, 3.75f, 0.0f);
            float target_zoom = glm::clamp(8.0f / (distance + 4.0f), 0.3f, 1.0f);

            // Smooth lerp to target values
            g_camera_position = g_camera_position + (target_position - g_camera_position) * 0.1f;
            g_camera_zoom = g_camera_zoom + (target_zoom - g_camera_zoom) * 0.05f;

            // Apply smoothed values
            g_view_matrix = glm::scale(g_view_matrix, glm::vec3(g_camera_zoom, g_camera_zoom, 1.0f));
            g_view_matrix = glm::translate(g_view_matrix, g_camera_position);

        }
        else {
            // Default camera behavior (follow player)
            if (player_position.x > LEVEL1_LEFT_EDGE) {
                g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-player_position.x, 3.75, 0));
            }
            else {
                g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
            }
        }


        // Only adjust view for game levels
        /*if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
        }
        else {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
        }*/

        // Scene transitions
        if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().y < -10.0f) switch_to_scene(g_levelB);
        if (g_current_scene == g_levelB && g_current_scene->get_state().player->get_position().y < -10.0f) switch_to_scene(g_levelC);
    }

}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_shader_program.set_model_matrix(g_background_matrix);
    glm::mat4 original_view_matrix = g_view_matrix;
    g_shader_program.set_view_matrix(glm::mat4(1.0f));
    float vertices[] = { -5.0, -3.75, 5.0, -3.75, 5.0, 3.75, -5.0, -3.75, 5.0, 3.75, -5.0, 3.75 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, g_background_texture);

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Restore the view matrix for the rest of the scene
    g_shader_program.set_view_matrix(original_view_matrix);

    // world rendering
    g_current_scene->render(&g_shader_program);

    // text rendering (stuff with fixed positions)
    glm::mat4 ui_view_matrix = glm::mat4(1.0f); 
    g_shader_program.set_view_matrix(ui_view_matrix);

    GLuint font_texture_id = Utility::load_texture("assets/white_font_sheet.png");

    if (g_current_scene != g_menu_screen && g_current_scene != g_lose_scene && g_current_scene != g_win_scene) {
        std::string health_text = "Health: " + std::to_string(g_current_scene->get_state().player->get_health());
        Utility::draw_text(&g_shader_program, font_texture_id, health_text, 0.3f, 0.0f, glm::vec3(-4.5f, -3.25f, 0.0f));  // Fixed position in bottom left

        // Enemy health in bottom right corner
        std::string enemy_health_text = "Enemy: " + std::to_string(g_current_scene->get_state().enemies[0].get_health());
        Utility::draw_text(&g_shader_program, font_texture_id, enemy_health_text, 0.3f, 0.0f, glm::vec3(2.0f, -3.25f, 0.0f));  // Fixed position in bottom right
    }

    //g_shader_program.set_view_matrix(g_view_matrix);

    g_effects->render();

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_menu_screen;
    delete g_lose_scene;
    delete g_win_scene;
    delete g_effects;

    Mix_HaltMusic();
    if (g_bgm) {
        Mix_FreeMusic(g_bgm);
        g_bgm = nullptr;
    }
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();

        if (g_current_scene->get_state().next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);

        render();
    }

    shutdown();
    return 0;
}