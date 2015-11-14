#include "stdafx.h"
#include "FileUtils.h"

#include "Log.h"

namespace core
{

FileReader::FileReader(const char *filename) :
    fileBuffer(nullptr),
    filePointer(nullptr),
    fileSize(0)
{
    Open(filename);
}

FileReader::~FileReader()
{
    Close();
}

bool FileReader::Open(const char *filename)
{
    assert(filename);
    Close();

    std::ifstream file(filename);
    if (!file)
        return false;

    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios_base::beg);

    fileBuffer = new char[fileSize];
    filePointer = fileBuffer;
    file.read(fileBuffer, fileSize);

    return true;
}

void FileReader::Close()
{
    if (IsOpen()) {
        delete[] fileBuffer;
        fileBuffer = nullptr;
        filePointer = nullptr;
        fileSize = 0;
    }
}

bool FileReader::IsOpen() const
{
    return fileBuffer != nullptr;
}

void FileReader::Seek(size_t offset, SeekOrigin origin)
{
    assert(IsOpen());

    switch (origin) {
    case Seek_Beg: filePointer = fileBuffer + offset; break;
    case Seek_Cur: filePointer += offset; break;
    case Seek_End: filePointer = fileBuffer + fileSize - offset; break;
    default: assert(0);
    }
}

void FileReader::Rewind()
{
    Seek(0, Seek_Beg);
}

size_t FileReader::Tell() const
{
    assert(IsOpen());
    return filePointer - fileBuffer;
}

int FileReader::Peek() const
{
    int ret = -1;

    if (filePointer + 1 < fileBuffer + fileSize)
        ret = *(filePointer + 1);

    // HACK: This should't be needed, but seems we get the wrong filesize from os or something
    if (ret < 0)
        ret = -1;

    return ret;
}

char FileReader::ReadChar()
{
    return *filePointer++;
}

const char *FileReader::GetLine(size_t *length)
{
    const char *line = filePointer;

    char c = 0;
    do {
        c = ReadChar();
    } while (c != '\n' && c != '\0');

    if (length)
        *length = filePointer - line;

    return line;
}

} // core