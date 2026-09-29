#include "Engine.hpp"
#include "GameScene.hpp"

int main(int /*argc*/, char* /*argv*/[])
{
    Application app{ "GAM200", 640, 480 };
    GameScene scene;

    if (app.Initialize(scene))
    {
        app.Run();
    }
    app.CleanUp();
    return 0;
    
}