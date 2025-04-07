#include "win_scene.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char FONTSHEET_FILEPATH[] = "assets/font_sheet2.png";



WinScene::~WinScene()
{
}

void WinScene::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");

}

void WinScene::update(float delta_time)
{

}

void WinScene::render(ShaderProgram* program)
{
    // Set a solid background color
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render text
    Utility::draw_text(program, Utility::load_texture("assets/font_sheet2.png"),
        "You win!", 0.5f, 0.0f, glm::vec3(-1.5f, 0.5f, 0.0f));


}