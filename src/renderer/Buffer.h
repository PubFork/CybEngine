#pragma once

namespace renderer
{

enum BufferUsage
{
    Buffer_Invalid,
    Buffer_Vertex,
    Buffer_Index,
    Buffer_Uniform,
    Buffer_Compute
};

class Buffer
{
public:
    Buffer() : bufferId(0)
    {
    }

    ~Buffer()
    {
        if (bufferId)
            glDeleteBuffers(1, &bufferId);
    }

    bool Data(BufferUsage usage, const void *buffer, size_t bufSize)
    {
        switch (usage) {
        case Buffer_Index: use = GL_ELEMENT_ARRAY_BUFFER; break;
        default:           use = GL_ARRAY_BUFFER; break;
        }

        if (!bufferId)
            glCreateBuffers(1, &bufferId);
        size = bufSize;

        glBindBuffer(use, bufferId);
        glBufferData(use, size, buffer, GL_STATIC_DRAW);
        return true;
    }

    GLuint Id() const 
    { 
        return bufferId; 
    }

private:
    uint32_t bufferId;
    uint32_t use;
    size_t size;
};


} // renderer