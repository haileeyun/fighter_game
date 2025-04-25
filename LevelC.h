#include "Scene.h"


class LevelC : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;
    float enemy_attack_cooldown = 0.0f;


    // ————— DESTRUCTOR ————— //
    ~LevelC();

    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void set_shader_program(ShaderProgram* program) {
        m_shader_program = program;
    }
private:

    GLuint m_font_texture_id;
    Entity* m_lightning; // Lightning effect for super attacks
    bool colorInverted;
    ShaderProgram* m_shader_program;
    bool m_color_inversion_active;

  



};