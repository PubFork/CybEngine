#pragma once

class IGameApp
{
public:
    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void Update(double deltaTime) = 0;
    virtual void RenderScene() = 0;
};

void RunApplication(IGameApp &app, const wchar_t *className);

