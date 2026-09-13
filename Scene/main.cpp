#include "Application.hpp"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    Application app("GAM200", 640, 480);
    if (app.Initialize())
    {
        app.MainLoop();
    }
    
    app.CleanUp();
    return 0;
}