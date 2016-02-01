#include "stdafx.h"
#include "FileUtils.h"
#include "Debug.h"

FileReader::FileReader(const char *filename, bool throwOnFailure) :
    fileBuffer(nullptr),
    currentPosition(nullptr),
    fileSize(0)
{
    bool result = Open(filename);
    THROW_FATAL_COND(throwOnFailure && !result, std::string("Failed to open file ") + filename);
}

FileReader::~FileReader()
{
    Close();
}

bool FileReader::Open(const char *filename)
{
    assert(filename);
    
    Close();
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        return false;

    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios_base::beg);

    fileBuffer = new char[fileSize + 1];
    currentPosition = fileBuffer;
    file.read(fileBuffer, fileSize);
    fileBuffer[fileSize] = '\0';

    return true;
}

void FileReader::Close()
{
    if (IsOpen())
    {
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

    switch (origin)
    {
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
    const char* line = currentPosition;

    char c = 0;
    size_t bytesRead = 0;
    do
    {
        bytesRead = ReadChar(c);
    } while (c != '\n' && c != '\0' && bytesRead == sizeof(c));

    if (length)
        *length = currentPosition - line;

    return line;
}

FileWriter::FileWriter(const char *filename, bool truncate, bool throwOnFailure)
{
    bool result = Open(filename, truncate);
    THROW_FATAL_COND(throwOnFailure && !result, std::string("Failed to open file ") + filename);
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

    file.open(filename, mode | std::ios::binary);
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
    file.write((const char*)buffer, numBytes);
    return file.tellp() - origSize;
}

size_t WriteDataToFile(const char *filename, const void *buffer, size_t numBytes)
{
    FileWriter file(filename, true);
    return file.Write(buffer, numBytes);
}

std::string GetBasePath(const char *filename)
{
    size_t lastPathSeperatorPos = 0;
    const char *ptr = filename;
    while (*ptr != '\0')
    {
        if (*ptr == '/' || *ptr == '\\')
            lastPathSeperatorPos = ptr - filename;
        ptr++;
    }

    if (!lastPathSeperatorPos)
        return "";

    return std::string(filename, lastPathSeperatorPos + 1);
}
