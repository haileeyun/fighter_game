#include "Scene.h"
#include "Platform.h"


class LevelA : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;

    // ————— DESTRUCTOR ————— //
    ~LevelA();

    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
private:

    Entity* m_platform; 
    GLuint m_font_texture_id;


};