#include "lose_scene.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char FONTSHEET_FILEPATH[] = "assets/font1.png";



LoseScene::~LoseScene()
{
}

void LoseScene::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/tiles.png");

}

void LoseScene::update(float delta_time)
{

}

void LoseScene::render(ShaderProgram* program)
{
    // Set a solid background color
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render text
    Utility::draw_text(program, Utility::load_texture("assets/font_sheet2.png"),
        "You lost", 0.5f, 0.0f, glm::vec3(-1.5f, 0.5f, 0.0f));
    

}