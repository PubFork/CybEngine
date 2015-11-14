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
    FileReader(const char *filename);
    ~FileReader();

    bool Open(const char *filename);
    void Close();
    bool IsOpen() const;

    void Seek(size_t offset, SeekOrigin origin);
    void Rewind();
    size_t Tell() const;
    int Peek() const;

    char ReadChar();
    const char *GetLine(size_t *length);

private:
    char *fileBuffer;
    char *filePointer;
    size_t fileSize;
};

} // core