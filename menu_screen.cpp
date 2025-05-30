#include "menu_screen.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8




MenuScreen::~MenuScreen()
{
}

void MenuScreen::initialise()
{
    m_game_state.next_scene_id = -1;

    
}

void MenuScreen::update(float delta_time)
{
    
}

void MenuScreen::render(ShaderProgram* program)
{
    //glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    // Render text
    Utility::draw_text(program, Utility::load_texture("assets/white_font_sheet.png"),
        "Welcome", 0.7f, 0.0f, glm::vec3(-2.0f, 0.5f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("assets/white_font_sheet.png"),
        "Use arrows to move.", 0.3f, 0.0f, glm::vec3(-2.5f, -0.5f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("assets/white_font_sheet.png"),
        "X for attack and S for super.", 0.3f, 0.0f, glm::vec3(-4.0f, -1.5f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("assets/white_font_sheet.png"),
        "Press Enter", 0.5f, 0.0f, glm::vec3(-2.5f, -2.5f, 0.0f));

}