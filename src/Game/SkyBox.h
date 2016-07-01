#pragma once
#include "Renderer/RenderDevice.h"

bool CreateSkyBoxSurface(std::shared_ptr<renderer::IRenderDevice> device, renderer::Surface &surface, const char *textureFileNames[6]);