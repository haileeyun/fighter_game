/**
* Author: Hailee Yun
* Assignment: Lunar Lander
* Date due: 2025-3-14, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               

#define STB_IMAGE_IMPLEMENTATION // to load my own images
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };
enum ScaleDirection { GROWING, SHRINKING };
enum GameState { PLAYING, PLAYER_1_WINS, PLAYER_2_WINS }; // used to keep track of who wins
GameState g_game_state = PLAYING; // default to running



// Our window dimensions
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

// Background color components
constexpr float BG_RED = 1.0f,
BG_BLUE = 0.71f,
BG_GREEN = 0.76f,
BG_OPACITY = 1.0f;

// shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// texture constants
constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

constexpr char ROCKET_SPRITE_FILEPATH[]   = "shaders/rocket.png";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;


// initial position and scale of sprites
constexpr glm::vec3 INIT_SCALE      = glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_SCALE_ROCKET = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_POS_ROCKET   = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr float ROT_INCREMENT = 40.0f; // rotational constant

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

glm::mat4 g_view_matrix,   
g_rocket_matrix,
g_model_matrix,
g_projection_matrix;  

constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

float g_previous_ticks;
float g_radius = 2;
float g_frames = 0.0f;

int g_frame_counter = 0;

glm::vec3 g_rocket_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_rocket_movement = glm::vec3(0.0f, 0.0f, 0.0f);                                                           
float g_rocket_speed = 5.0f;  
 
// texture ids
GLuint g_rocket_texture_id;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}



void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);  // initialize video

    g_display_window = SDL_CreateWindow("lunar_lander",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

      

    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;

        SDL_Quit();
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Initialise our camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load up our shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_rocket_matrix       = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); // boundaries

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_rocket_texture_id   = load_texture(ROCKET_SPRITE_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_rocket_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                    
                    case SDLK_RIGHT:
                        
                        break;

                   
                    
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;


                    
                    default:
                        break;
                }
            default:
                break;
        }
    }
       
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
                                                                    
    // rocket movement (arrow Keys)
    // for now, simple movement, later doing acceleration
    if (key_state[SDL_SCANCODE_UP]) { g_rocket_movement.y = 1.0f; }
    if (key_state[SDL_SCANCODE_DOWN]) { g_rocket_movement.y = -1.0f; }
    if (key_state[SDL_SCANCODE_RIGHT]) { g_rocket_movement.x = 1.0f; }
    if (key_state[SDL_SCANCODE_LEFT]) { g_rocket_movement.x = -1.0f;  }


    
    // This makes sure that the player can't "cheat" their way into moving faster
    if (glm::length(g_rocket_movement) > 1.0f) {
        g_rocket_movement = glm::normalize(g_rocket_movement);
    }

}


bool check_collision(glm::vec3 pos1, glm::vec3 scale1, glm::vec3 pos2, glm::vec3 scale2) {
    float x_distance = fabs(pos1.x - pos2.x) - ((scale1.x + scale2.x) / 2.0f);
    float y_distance = fabs(pos1.y - pos2.y) - ((scale1.y + scale2.y) / 2.0f);
    return (x_distance < 0.0f && y_distance < 0.0f); // returns true if there is a collision
}

float calculate_y_direction(glm::vec3 ball_pos, glm::vec3 other_pos) {
    // if ball is above the "paddle", move upwards
    if (ball_pos.y > other_pos.y) {
        return 1.0f; // move up
    }
    // if ball is below the "paddle", move downwards
    else if (ball_pos.y < other_pos.y) {
        return -1.0f; // move down
    }
    // ff they are at the same height, no change in angle
    return 0.0f;
}


void update()
{
    
    /* DELTA TIME */
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
    float g_time_accumulator = 0.0f;


    // screen boundaries (from projection_matrix)
    const float LEFT_BOUND = -5.0f;
    const float RIGHT_BOUND = 5.0f;
    const float BOTTOM_BOUND = -3.75f;
    const float TOP_BOUND = 3.75f;


    if (g_game_state == PLAYING) {
        // movement
        g_rocket_position += g_rocket_movement * g_rocket_speed * delta_time;



        // bound checking
        if (g_rocket_position.y + INIT_POS_ROCKET.y < BOTTOM_BOUND) g_rocket_position.y = BOTTOM_BOUND;
        if (g_rocket_position.y + INIT_POS_ROCKET.y > TOP_BOUND) g_rocket_position.y = TOP_BOUND;
        if (g_rocket_position.x + INIT_POS_ROCKET.x > RIGHT_BOUND) g_rocket_position.x = RIGHT_BOUND;
        if (g_rocket_position.x + INIT_POS_ROCKET.x < LEFT_BOUND) g_rocket_position.x = LEFT_BOUND;

    }
    

    /* TRANSFORMATIONS */
    // rocket transformations
    g_rocket_matrix = glm::mat4(1.0f);
    g_rocket_matrix = glm::translate(g_rocket_matrix, INIT_POS_ROCKET + g_rocket_position);
    g_rocket_matrix = glm::scale(g_rocket_matrix, INIT_SCALE_ROCKET);

    
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}

// binding and rendering the textures
void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                          0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    //draw_object(g_background_matrix, g_background_texture_id); // draw first so it appears in the back

    if (g_game_state == PLAYING) {
        // Bind texture
        draw_object(g_rocket_matrix, g_rocket_texture_id);

    }
    else {
        glm::mat4 win_message_matrix = glm::mat4(1.0f);
        win_message_matrix = glm::translate(win_message_matrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Center the message
        win_message_matrix = glm::scale(win_message_matrix, glm::vec3(4.0f, 3.0f, 0.0f)); // Scale the message
       
    }
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();  
        update();         
        render();         
    }

    shutdown();  
    return 0;
}