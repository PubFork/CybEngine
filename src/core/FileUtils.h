#pragma once

namespace core
{

enum SeekOrigin
{
    Seek_Beg,
    Seek_Cur,
    Seek_End
};

class FileReader
{
public:
    FileReader(const char *filename, bool throwOnFailure = false);
    ~FileReader();

    bool Open(const char *filename);
    void Close();
    bool IsOpen() const;

    void Seek(size_t offset, SeekOrigin origin);
    void Rewind();
    size_t Tell() const;
    int Peek() const;

    size_t Read(void *buffer, size_t numBytes);
    size_t ReadChar(char &c);
    size_t ReadUInt16(uint16_t &value);
    size_t ReadSInt16(int16_t &value);
    size_t ReadUInt32(uint32_t &value);
    size_t ReadSInt32(int32_t &value);
    size_t ReadFloat(float &value);

    const char *RawBuffer() const { return fileBuffer; }
    size_t Size() const { return fileSize; }

    const char *GetLine(size_t *length);    // deprecated

private:
    char *fileBuffer;
    char *currentPosition;
    size_t fileSize;
};

class FileWriter
{
public:
    FileWriter(const char* filename, bool truncate, bool throwOnFailure = false);
    ~FileWriter();

    bool Open(const char* filename, bool truncate);
    void Close();
    bool IsOpen();

    size_t Write(const void* buffer, size_t numBytes);

private:
    std::ofstream file;
};

size_t WriteDataToFile(const char* filename, const void* buffer, size_t numBytes);
std::string GetBasePath(const char* filename);

} // core