/**
* Author: Hailee Yun
* Assignment: Pong Clone
* Date due: 2025-3-01, 11:59pm
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
// if one of the players win, the respective png will be rendered
GameState g_game_state = PLAYING; // default to running



// Our window dimensions
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

// Background color components
constexpr float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

// shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// texture constants
constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

constexpr char BLACK_CAT_SPRITE_FILEPATH[]   = "shaders/hailee.png",
               BUTTERFLY_SPRITE_FILEPATH[] = "shaders/danielle.png",
               BALL_SPRITE_FILEPATH[] = "shaders/doughnut.png",
               PLAYER_1_WINS_SPRITE_FILEPATH[] = "shaders/player_1_wins.png",
               PLAYER_2_WINS_SPRITE_FILEPATH[] = "shaders/player_2_wins.png";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;


// initial position and scale of sprites
constexpr glm::vec3 INIT_SCALE      = glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_SCALE_BLACK_CAT = glm::vec3(1.75f, 2.5f, 0.0f),
                    INIT_SCALE_BUTTERFLY = glm::vec3(1.75f, 2.5f, 0.0f),
                    INIT_SCALE_BALL = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_SCALE_PLAYER_1_WINS = glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_SCALE_PLAYER_2_WINS = glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_POS_BLACK_CAT   = glm::vec3(4.0f, 0.0f, 0.0f),
                    INIT_POS_BUTTERFLY = glm::vec3(-4.0f, 0.0f, 0.0f),
                    INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr float ROT_INCREMENT = 1.0f; // rotational constant

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

glm::mat4 g_view_matrix,   
g_black_cat_matrix,
g_butterfly_matrix,
g_ball_matrix,
g_player_1_wins_matrix,
g_player_2_wins_matrix,
g_model_matrix,
g_projection_matrix;  

constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

float g_previous_ticks;
float g_radius = 2;
float g_frames = 0.0f;

int g_frame_counter = 0;

glm::vec3 g_black_cat_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_black_cat_movement = glm::vec3(0.0f, 0.0f, 0.0f);                                                           
float g_black_cat_speed = 10.0f;  // move 1 unit per second

glm::vec3 g_butterfly_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_butterfly_movement = glm::vec3(0.0f, 0.0f, 0.0f);
float g_butterfly_speed = 5.0f;  // move 1 unit per second

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(1.0f, 0.0f, 0.0f); // when game starts, go right, change later
float g_ball_speed = 3.0f;  // move 1 unit per second

bool g_single_player_mode = false; // Default to two-player mode

// multi-ball functionality
int g_num_balls = 1;
constexpr int MAX_BALLS = 3; // max numebr of balls
glm::vec3 g_ball_positions[MAX_BALLS]; // position vector of the balls
glm::vec3 g_ball_movements[MAX_BALLS]; // movement vector of the balls
bool g_ball_active[MAX_BALLS] = { false }; // track which balls are active


// what is added for their rotations
glm::vec3 g_rotation_black_cat   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_butterfly = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_ball = glm::vec3(0.0f, 0.0f, 0.0f);

// glm::vec3 g_scale_butterfly = glm::vec3(0.0f, 0.0f, 0.0f); // to be added to butterfly scale vector so that it pulses

// texture ids
GLuint g_black_cat_texture_id,
       g_butterfly_texture_id,
       g_ball_texture_id,
       g_player_1_wins_texture_id,
       g_player_2_wins_texture_id;

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

    g_display_window = SDL_CreateWindow("pong_clone",
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

    g_black_cat_matrix       = glm::mat4(1.0f);
    g_butterfly_matrix     = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);
    g_player_1_wins_matrix = glm::mat4(1.0f);
    g_player_2_wins_matrix = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); // boundaries

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_black_cat_texture_id   = load_texture(BLACK_CAT_SPRITE_FILEPATH);
    g_butterfly_texture_id = load_texture(BUTTERFLY_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_player_1_wins_texture_id = load_texture(PLAYER_1_WINS_SPRITE_FILEPATH);
    g_player_2_wins_texture_id = load_texture(PLAYER_2_WINS_SPRITE_FILEPATH);

    // Initialize the first ball
    g_ball_positions[0] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
    g_ball_movements[0] = glm::vec3(1.0f, 0.0f, 0.0f); // Move right initially
    g_ball_active[0] = true; // Mark as active

    // Mark other balls as inactive
    for (int i = 1; i < MAX_BALLS; i++) {
        g_ball_active[i] = false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_black_cat_movement = glm::vec3(0.0f);
    if (!g_single_player_mode) { // if it's single player mode, it will just continue to move in whatever direction
        g_butterfly_movement = glm::vec3(0.0f);

    }

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

                    case SDLK_1: // set num of balls to 1
                        g_num_balls = 1;
                        // Deactivate balls 2 and 3
                        g_ball_active[1] = false;
                        g_ball_active[2] = false;

                        g_ball_positions[1] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                        g_ball_movements[1] = glm::vec3(-1.0f, 0.0f, 0.0f); // Move left
                        g_ball_positions[2] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                        g_ball_movements[2] = glm::vec3(1.0f, 1.0f, 0.0f); // Move diagonally right and up

                        break;

                    case SDLK_2:
                        if (g_num_balls == 1) {
                            // Activate ball 2
                            g_ball_positions[1] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                            g_ball_movements[1] = glm::vec3(-1.0f, 0.0f, 0.0f); // Move left
                            g_ball_active[1] = true; // Mark as active
                        }
                        else if (g_num_balls == 3){
                            // if there were 3 balls, deactivatee ball 3
                            g_ball_active[2] = false;
                            g_ball_positions[2] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                            g_ball_movements[2] = glm::vec3(1.0f, 1.0f, 0.0f); // Move diagonally right and up
                        }
                        g_num_balls = 2;

                        break;

                    case SDLK_3:
                        if (g_num_balls < 3) {
                 
                            if (g_num_balls == 1) {
                                // if there was only 1 ball, activate ball 2
                                g_ball_positions[1] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                                g_ball_movements[1] = glm::vec3(-1.0f, 0.0f, 0.0f); // Move left
                                g_ball_active[1] = true; // Mark as active

                                // Activate ball 3
                                g_ball_positions[2] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                                g_ball_movements[2] = glm::vec3(1.0f, 1.0f, 0.0f); // Move diagonally right and up
                                g_ball_active[2] = true; // Mark as active
                            }
                            else if (g_num_balls == 2) {
                                // Activate ball 3
                                g_ball_positions[2] = glm::vec3(0.0f, 0.0f, 0.0f); // Start at the center
                                g_ball_movements[2] = glm::vec3(1.0f, 1.0f, 0.0f); // Move diagonally right and up
                                g_ball_active[2] = true; // Mark as active
                            }
                            
                        }
                        g_num_balls = 3;
                        break;
                    
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;

                    case SDLK_t: 
                        // single-player mode
                        g_single_player_mode = !g_single_player_mode; // switch mode
                        if (g_single_player_mode) {
                            g_butterfly_movement = glm::vec3(0.0f, 1.0f, 0.0f); // Start moving up
                        }
                        else {
                            g_butterfly_movement = glm::vec3(0.0f);
                        }
                        break;


                    case SDLK_r:
                        // restart the game
                        if (g_game_state != PLAYING) {
                            g_game_state = PLAYING;
                            //g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f); // Reset ball position
                            //g_ball_movement = glm::vec3(1.0f, 0.0f, 0.0f); // Reset ball movement
                            g_black_cat_position = glm::vec3(0.0f, 0.0f, 0.0f);
                            g_butterfly_position = glm::vec3(0.0f, 0.0f, 0.0f);

                            // Reset balls
                            g_num_balls = 1;
                            g_ball_positions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
                            g_ball_movements[0] = glm::vec3(1.0f, 0.0f, 0.0f);
                            g_ball_active[0] = true;

                            g_ball_active[1] = false;
                            g_ball_active[2] = false;
                        }
                        break;
                    
                    default:
                        break;
                }
            default:
                break;
        }
    }
       
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
                                                                    
    // black_cat movement (arrow Keys)
    if (key_state[SDL_SCANCODE_UP]) { g_black_cat_movement.y = 1.0f; }
    if (key_state[SDL_SCANCODE_DOWN]) { g_black_cat_movement.y = -1.0f; }

    // butterfly movement (WASD keys)
    if (!g_single_player_mode) { // only allow player 1 to move if not in single player mode
        if (key_state[SDL_SCANCODE_W]) { g_butterfly_movement.y = 1.0f; }  // Move up
        if (key_state[SDL_SCANCODE_S]) { g_butterfly_movement.y = -1.0f; }  // Move down
    }


    
    // This makes sure that the player can't "cheat" their way into moving faster
    if (glm::length(g_black_cat_movement) > 1.0f) {
        g_black_cat_movement = glm::normalize(g_black_cat_movement);
    }
    if (glm::length(g_butterfly_movement) > 1.0f) {
        g_butterfly_movement = glm::normalize(g_butterfly_movement);
    }
    //if (glm::length(g_ball_movement) > 1.0f) {
     //   g_ball_movement = glm::normalize(g_ball_movement);
    //}
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

    if (g_game_state != PLAYING) {
       
        // Reset black cat position
        g_black_cat_position = glm::vec3(0.0f, 0.0f, 0.0f);

        // Reset butterfly position
        g_butterfly_position = glm::vec3(0.0f, 0.0f, 0.0f);
    }


    // screen boundaries (from projection_matrix)
    const float LEFT_BOUND = -5.0f;
    const float RIGHT_BOUND = 5.0f;
    const float BOTTOM_BOUND = -3.75f;
    const float TOP_BOUND = 3.75f;


    if (g_game_state == PLAYING) {
        // movement
        g_black_cat_position += g_black_cat_movement * g_black_cat_speed * delta_time;
        g_butterfly_position += g_butterfly_movement * g_butterfly_speed * delta_time;
        //g_ball_position += g_ball_movement * g_ball_speed * delta_time;

        for (int i = 0; i < MAX_BALLS; i++) {
            if (g_ball_active[i]) {
                g_ball_positions[i] += g_ball_movements[i] * g_ball_speed * delta_time;
                // Boundary checking for balls
                if (g_ball_positions[i].y + INIT_POS_BALL.y > TOP_BOUND || g_ball_positions[i].y + INIT_POS_BALL.y < BOTTOM_BOUND) {
                    g_ball_movements[i].y *= -1.0f; // Reverse y-direction
                }

                // Collision detection between ball and cat
                if (check_collision(
                    INIT_POS_BALL + g_ball_positions[i], INIT_SCALE_BALL,
                    INIT_POS_BLACK_CAT + g_black_cat_position, INIT_SCALE_BLACK_CAT
                )) {
                    g_ball_movements[i].x *= -1.0f; // Reverse direction
                    g_ball_movements[i].y = calculate_y_direction(
                        INIT_POS_BALL + g_ball_positions[i],
                        INIT_POS_BLACK_CAT + g_black_cat_position
                    ); // Adjust y-direction
                }

                // Collision detection between ball and butterfly
                if (check_collision(
                    INIT_POS_BALL + g_ball_positions[i], INIT_SCALE_BALL,
                    INIT_POS_BUTTERFLY + g_butterfly_position, INIT_SCALE_BUTTERFLY
                )) {
                    g_ball_movements[i].x *= -1.0f; // Reverse direction
                    g_ball_movements[i].y = calculate_y_direction(
                        INIT_POS_BALL + g_ball_positions[i],
                        INIT_POS_BUTTERFLY + g_butterfly_position
                    ); // Adjust y-direction
                }

                // Check for win condition
                if (g_ball_positions[i].x + INIT_POS_BALL.x > RIGHT_BOUND) {
                    g_game_state = PLAYER_1_WINS; // butterfly wins
                }
                else if (g_ball_positions[i].x + INIT_POS_BALL.x < LEFT_BOUND) {
                    g_game_state = PLAYER_2_WINS; // black_cat wins
                }

            }
        }
        
        // if in single-player mode, the butterfly will simply move up and down
        if (g_single_player_mode) {
            if (g_butterfly_position.y + INIT_POS_BUTTERFLY.y > TOP_BOUND) {
                g_butterfly_movement.y = -1.0f; // move down
            }
            else if (g_butterfly_position.y + INIT_POS_BUTTERFLY.y < BOTTOM_BOUND) {
                g_butterfly_movement.y = 1.0f; // move up
            }
        }

     

        // bound checking
        if (g_black_cat_position.y + INIT_POS_BLACK_CAT.y < BOTTOM_BOUND) g_black_cat_position.y = BOTTOM_BOUND;
        if (g_black_cat_position.y + INIT_POS_BLACK_CAT.y > TOP_BOUND) g_black_cat_position.y = TOP_BOUND;

        if (g_butterfly_position.y + INIT_POS_BUTTERFLY.y < BOTTOM_BOUND) g_butterfly_position.y = BOTTOM_BOUND;
        if (g_butterfly_position.y + INIT_POS_BUTTERFLY.y > TOP_BOUND) g_butterfly_position.y = TOP_BOUND;

    }
    

    /* TRANSFORMATIONS */
    // black_cat transformations
    g_black_cat_matrix = glm::mat4(1.0f);
    g_black_cat_matrix = glm::translate(g_black_cat_matrix, INIT_POS_BLACK_CAT + g_black_cat_position);
    g_black_cat_matrix = glm::scale(g_black_cat_matrix, INIT_SCALE_BLACK_CAT);


    // butterfly transformations
    g_butterfly_matrix = glm::mat4(1.0f);
    g_butterfly_matrix = glm::translate(g_butterfly_matrix, INIT_POS_BUTTERFLY + g_butterfly_position);
    g_butterfly_matrix = glm::scale(g_butterfly_matrix, INIT_SCALE_BUTTERFLY);

    
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
    

    if (g_game_state == PLAYING) {
        // Bind texture
        draw_object(g_black_cat_matrix, g_black_cat_texture_id);
        draw_object(g_butterfly_matrix, g_butterfly_texture_id);
        //draw_object(g_ball_matrix, g_ball_texture_id);

        // Render all active balls
        for (int i = 0; i < MAX_BALLS; i++) {
            if (g_ball_active[i]) {
                glm::mat4 ball_matrix = glm::mat4(1.0f);
                ball_matrix = glm::translate(ball_matrix, INIT_POS_BALL + g_ball_positions[i]);
                ball_matrix = glm::scale(ball_matrix, INIT_SCALE_BALL);
                draw_object(ball_matrix, g_ball_texture_id);
            }
        }

    }
    else {
        glm::mat4 win_message_matrix = glm::mat4(1.0f);
        win_message_matrix = glm::translate(win_message_matrix, glm::vec3(0.0f, 0.0f, 0.0f)); // Center the message
        win_message_matrix = glm::scale(win_message_matrix, glm::vec3(4.0f, 2.0f, 0.0f)); // Scale the message
        if (g_game_state == PLAYER_1_WINS) {
            draw_object(win_message_matrix, g_player_1_wins_texture_id);
        }
        else if (g_game_state == PLAYER_2_WINS) {
            draw_object(win_message_matrix, g_player_2_wins_texture_id);
        }
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