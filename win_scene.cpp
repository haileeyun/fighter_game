#include "win_scene.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char FONTSHEET_FILEPATH[] = "assets/white_font_sheet.png";



WinScene::~WinScene()
{
}

void WinScene::initialise()
{
    m_game_state.next_scene_id = -1;


}

void WinScene::update(float delta_time)
{

}

void WinScene::render(ShaderProgram* program)
{
   

    // Render text
    Utility::draw_text(program, Utility::load_texture("assets/white_font_sheet.png"),
        "You win!", 0.5f, 0.0f, glm::vec3(-1.5f, 0.5f, 0.0f));


}