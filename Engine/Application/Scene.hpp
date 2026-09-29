#pragma once

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void InitModel() {}                         
    virtual void Update(float /*dt*/) {}                 
    virtual void Draw() {}                               
    virtual void Resize(int /*width*/, int /*height*/) {}
    virtual void CleanUp() {}                           

    bool IsRunning() const { return m_running; }

protected:
    bool m_running = true;                               
};