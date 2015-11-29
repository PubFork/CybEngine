#include "stdafx.h"
#include "FileUtils.h"

#include "Log.h"

namespace core
{

FileReader::FileReader(const char *filename) :
    fileBuffer(nullptr),
    currentPosition(nullptr),
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
    {
        DEBUG_LOG_TEXT_COND(!file.is_open(), "Failed to open file %s for reading.", filename);
        return false;
    }

    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios_base::beg);

    fileBuffer = new char[fileSize];
    currentPosition = fileBuffer;
    file.read(fileBuffer, fileSize);

    return true;
}

void FileReader::Close()
{
    if (IsOpen()) {
        delete[] fileBuffer;
        fileBuffer = nullptr;
        currentPosition = nullptr;
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
    case Seek_Beg: currentPosition = fileBuffer + offset; break;
    case Seek_Cur: currentPosition += offset; break;
    case Seek_End: currentPosition = fileBuffer + fileSize - offset; break;
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
    return currentPosition - fileBuffer;
}

int FileReader::Peek() const
{
    int ret = -1;

    if (currentPosition + 1 < fileBuffer + fileSize)
        ret = *(currentPosition + 1);

    // HACK: This should't be needed, but seems we get the wrong filesize from os or something
    if (ret < 0)
        ret = -1;

    return ret;
}

size_t FileReader::Read(void *buffer, size_t numBytes)
{
    if (currentPosition + numBytes > fileBuffer + fileSize)
        numBytes = fileBuffer + fileSize - currentPosition;

    memcpy(buffer, currentPosition, numBytes);
    currentPosition += numBytes;
    return numBytes;
}

size_t FileReader::ReadChar(char &value)
{
    return Read(&value, sizeof(value));
}

size_t FileReader::ReadUInt16(uint16_t &value)
{
    return Read(&value, sizeof(value));
}

size_t FileReader::ReadSInt16(int16_t &value)
{
    return Read(&value, sizeof(value));
}

size_t FileReader::ReadUInt32(uint32_t &value)
{
    return Read(&value, sizeof(value));
}

size_t FileReader::ReadSInt32(int32_t &value)
{
    return Read(&value, sizeof(value));
}

size_t FileReader::ReadFloat(float &value)
{
    return Read(&value, sizeof(value));
}

const char *FileReader::GetLine(size_t *length)
{
    const char *line = currentPosition;

    char c = 0;
    do {
        ReadChar(c);
    } while (c != '\n' && c != '\0');

    if (length)
        *length = currentPosition - line;

    return line;
}

FileWriter::FileWriter(const char *filename, bool truncate)
{
    Open(filename, truncate);
}

FileWriter::~FileWriter()
{
    Close();
}

bool FileWriter::Open(const char *filename, bool truncate)
{
    std::ios_base::openmode mode = std::ios_base::out;
    if (truncate)
        mode |= std::ios_base::trunc;

    file.open(filename, mode);
    DEBUG_LOG_TEXT_COND(!file.is_open(), "Failed to open file %s for writing.", filename);
    return file.is_open();
}

void FileWriter::Close()
{
    file.close();
}

bool FileWriter::IsOpen()
{
    return file.is_open();
}

size_t FileWriter::Write(const void *buffer, size_t numBytes)
{
    std::streamsize origSize = file.tellp();
    file.write((const char *)buffer, numBytes);
    return file.tellp() - origSize;
}

size_t WriteDataToFile(const char *filename, const void *buffer, size_t numBytes)
{
    FileWriter file(filename, true);
    return file.Write(buffer, numBytes);
}

} // core