/**
* Author: Hailee Yun
* Assignment: Lunar Lander
* Date due: 2025-3-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
};

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH = 700,
WINDOW_HEIGHT = 550;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "shaders/george_0.png";
const char PLATFORM_FILEPATH[] = "shaders/cloud.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_win = false;  // Track if the player wins
bool g_lose = false; // Track if the player loses

float fuel = 300.0f;



// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}


// move later lol
GLuint g_font_texture_id = load_texture("shaders/font.png");



void initialise()
{
    std::cout << "initalizeee" << std::endl;
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("lunar_lander",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    

    // ––––– PLATFORMS ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];


    // note: might not need this anymore
    

    g_state.platforms[PLATFORM_COUNT - 1].m_texture_id = platform_texture_id;
    g_state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-2.5f, -2.35f, 0.0f));
    g_state.platforms[PLATFORM_COUNT - 1].set_width(0.5f);
    g_state.platforms[PLATFORM_COUNT - 1].set_height(0.5f);
    g_state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);
    g_state.platforms[PLATFORM_COUNT - 1].activate();

    for (int i = 0; i < PLATFORM_COUNT - 2; i++)
    {
        g_state.platforms[i].m_texture_id = platform_texture_id;
        g_state.platforms[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        g_state.platforms[i].set_width(0.5);
        g_state.platforms[i].set_height(0.5f);
        g_state.platforms[i].update(0.0f, NULL, 0);
        g_state.platforms[i].activate();
    }

    g_state.platforms[PLATFORM_COUNT - 2].m_texture_id = platform_texture_id;
    g_state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(2.5f, -2.5f, 0.0f));
    g_state.platforms[PLATFORM_COUNT - 2].set_width(0.5f);
    g_state.platforms[PLATFORM_COUNT - 2].set_height(0.5f);
    g_state.platforms[PLATFORM_COUNT - 2].update(0.0f, NULL, 0);
    g_state.platforms[PLATFORM_COUNT - 2].activate();

    // ––––– PLAYER ––––– //
    // Existing
    g_state.player = new Entity();
    g_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->m_speed = 1.0f;
    g_state.player->set_acceleration(glm::vec3(0.0f, -1.0f, 0.0f)); // setting the gravity
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_state.player->m_walking[g_state.player->LEFT] = new int[4] { 1, 5, 9, 13 };
    g_state.player->m_walking[g_state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    g_state.player->m_walking[g_state.player->UP] = new int[4] { 2, 6, 10, 14 };
    g_state.player->m_walking[g_state.player->DOWN] = new int[4] { 0, 4, 8, 12 };

    g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];  // start player looking left
    g_state.player->m_animation_frames = 4;
    g_state.player->m_animation_index = 0;
    g_state.player->m_animation_time = 0.0f;
    g_state.player->m_animation_cols = 4;
    g_state.player->m_animation_rows = 4;
    g_state.player->set_height(0.9f);
    g_state.player->set_width(0.9f);

    // Jumping
    g_state.player->m_jumping_power = 3.0f;

    GLuint g_font_texture_id = load_texture("shaders/font.png");

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    //g_state.player->set_movement(glm::vec3(0.0f)); // they could still be moving
    g_state.player->set_acceleration(glm::vec3(0.0f, -0.5f, 0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            case SDLK_r:
                
                g_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f)); // Reset player position
                g_state.player->set_movement(glm::vec3(0.0f));
                g_state.player->set_velocity(glm::vec3(0.0f));
                g_state.player->set_acceleration(glm::vec3(0.0f, -0.1f, 0.0f)); // Reset gravity
                g_win = false;
                g_lose = false;
                g_game_is_running = true; // Restart the game
                fuel = 300.0f;
                
                break;

            case SDLK_SPACE:
                // Jump
                //if (g_state.player->m_collided_bottom) g_state.player->m_is_jumping = true;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }
    if (g_win || g_lose || fuel <= 0.0f) return;

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        //g_state.player->m_movement.x = -1.0f;
        g_state.player->set_acceleration(glm::vec3(-1.0f, g_state.player->get_acceleration().y, 0.0f)); // Accelerate left
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];
        g_state.player->move_left();
        fuel -= 1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        //g_state.player->m_movement.x = 1.0f;
        g_state.player->set_acceleration(glm::vec3(1.0f, g_state.player->get_acceleration().y, 0.0f)); // Accelerate right
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];
        g_state.player->move_right();
        fuel -= 1.0f;
    }
    else if (key_state[SDL_SCANCODE_UP])
    {
        g_state.player->set_acceleration(glm::vec3(g_state.player->get_acceleration().x, 1.0f, 0.0f)); // Accelerate upward
        g_state.player->face_up();
        fuel -= 1.0f;
    }

    



    if (glm::length(g_state.player->m_movement) > 1.0f)
    {
        g_state.player->m_movement = glm::normalize(g_state.player->m_movement);
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

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, g_state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    


    // ––––– WIN/LOSE CONDITIONS ––––– //
    // Check if the player lands on platform 3 (win condition)
    
    //g_state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(2.5f, -2.5f, 0.0f));

    // IM SORRY I HAD TO HARDCODE THE WINNING CONDITION BECAUSE I COULDN'T GET CHECK_COLLISION TO WORK :(
    if (g_state.player->m_collided_bottom) {
        if (g_state.player->get_position().x <= 3.0f && g_state.player->get_position().x >= 2.0f) {
            g_win = true;
            g_state.player->set_movement(glm::vec3(0.0f));
            g_state.player->set_velocity(glm::vec3(0.0f)); // Reset velocity
            return;
        }
    }
    
    /*for (int i = 0; i < PLATFORM_COUNT; i++) {
        if (g_state.player->check_collision(&g_state.platforms[i])) {
            std::cout << "Collision detected with platform " << i << std::endl;
            if (i == 2) { // Winning condition
                g_win = true;
            }
        }
    }
    */
        
    

    // Check if the player falls off the screen (lose condition)
    if (g_state.player->get_position().y < -5.0f || // Fell too low
        g_state.player->get_position().x < -5.0f || // Fell too left
        g_state.player->get_position().x > 5.0f ||  // Fell too right
        g_state.player->get_position().y > 5.0f)    // Fell too high
    {
        //g_game_is_running = false; // Stop the game
        g_lose = true; // Set lose flag
        g_state.player->set_movement(glm::vec3(0.0f));
        g_state.player->set_velocity(glm::vec3(0.0f)); // Reset velocity
        return;
    }

    if (fuel <= 0.0f) {
        g_lose = true; // Set lose flag
        g_state.player->set_movement(glm::vec3(0.0f));
        g_state.player->set_velocity(glm::vec3(0.0f)); // Reset velocity
        return;
    }

    if (g_win || g_lose) {
        g_state.player->set_movement(glm::vec3(0.0f));
        return;
    }

}


constexpr int FONTBANK_SIZE = 16;

void draw_text(ShaderProgram* program, GLuint g_font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    glBindTexture(GL_TEXTURE_2D, g_font_texture_id);
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (std::size_t i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());
    

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    

    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}



void render()
{

    glClear(GL_COLOR_BUFFER_BIT);

    g_state.player->render(&g_program);

    //draw_text(&g_program, load_texture("shaders/font.png"), "welcome", 0.5f, 0.05f,glm::vec3(-3.5f, 2.0f, 0.0f));

    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);


    // ––––– DISPLAY FUEL ––––– //
    std::string fuel_text = "Fuel: " + std::to_string((int)fuel);
    draw_text(&g_program, load_texture("shaders/font.png"), fuel_text, 0.5f, 0.05f, glm::vec3(-4.5f, 3.5f, 0.0f));
    // ––––– DISPLAY WIN/LOSE MESSAGES ––––– //
    if (g_win)
    {
        draw_text(&g_program, load_texture("shaders/font.png"), "You Win!", 0.5f, 0.05f, glm::vec3(-1.5f, 0.0f, 0.0f));
    }
    else if (g_lose)
    {
        draw_text(&g_program, load_texture("shaders/font.png"), "You Lose!", 0.5f, 0.05f, glm::vec3(-1.5f, 0.0f, 0.0f));
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
