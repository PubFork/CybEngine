#include "stdafx.h"
#include "Model_obj.h"

#include "core/FileUtils.h"
#include "core/Log.h"

namespace engine
{

inline std::string ParseString(const char *buf, size_t *length)
{
    while (isspace(*buf))
        buf++;

    size_t l = strcspn(buf, " \t\r\n");

    if (length)
        *length = l;

    return std::string(buf, l);
}

ObjModel *OBJ_Load(const char *filename)
{
    core::FileReader file(filename);
    ObjModel *model = new ObjModel;

    while (file.Peek() != -1) {
        size_t lineLength = 0;
        const char *linebuf = file.GetLine(&lineLength);
        
        // skip leading whitespaces
        while (isspace(*linebuf)) 
            linebuf++;  

        // skip comments and empty lines
        if ( linebuf[0] == '#' || linebuf[0] == '\n' || lineLength <= 1)
            continue;   

        if (linebuf[0] == 'g' && isspace(linebuf[1])) {
            std::string name = ParseString(linebuf + 1, nullptr);
            DEBUG_LOG_TEXT("Group name %s", name.c_str());
        }
    }

    return model;
}

void OBJ_Free(ObjModel *model)
{
    delete model;
}

} // engine