#include "stdafx.h"
#include "RenderDevice.h"

#include "Base/Debug.h"
#include "Base/FileUtils.h"

namespace renderer
{

static VertexInputElement standardVertexLayout[] =
{
    { "position",  1, VertexFormat_Float3, 0 },
    { "normal",    2, VertexFormat_Float3, 0 },
    { "texCoord0", 3, VertexFormat_Float2, 0 },
};

static const CreatePipelineStateInfo builtintPipelineStatesCreateInfo[BuiltintPipelineState_Count] = 
{
    {
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { standardVertexLayout , _countof(standardVertexLayout) },
        renderer::Raster_DefaultState
    },
    {
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { standardVertexLayout , _countof(standardVertexLayout) },
        renderer::Raster_DefaultState
    },
    {
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { standardVertexLayout , _countof(standardVertexLayout) },
        renderer::Raster_DefaultState
    }
};

//==============================
// Render Device
//==============================

void RenderDevice::Init()
{
    // Initialize builtin pipeline states
    for (uint32_t i = 0; i < BuiltintPipelineState_Count; i++)
    {
        PipelineState *pipelineState = &builtintPipelineStates[i];
        const CreatePipelineStateInfo &createInfo = builtintPipelineStatesCreateInfo[i];
        pipelineState->Create(createInfo);
    }
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}

PipelineState *RenderDevice::BuiltintPipelineState(uint32_t pipelineStateEnum)
{
    assert(pipelineStateEnum < BuiltintPipelineState_Count);
    return &builtintPipelineStates[pipelineStateEnum];
}

} // renderer