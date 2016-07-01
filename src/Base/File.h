#pragma once

enum FileOpenMode
{
    FileOpen_Read               = 0x01,
    FileOpen_WriteAppend        = 0x02,
    FileOpen_WriteTruncate      = 0x03
};

enum FileSeekOrigin
{
    FileSeek_Beg,
    FileSeek_Cur,
    FileSeek_End
};

class IFile
{
public:
    virtual ~IFile() {}

    virtual const char *GetFilePath() const = 0;
    virtual const char *GetFileBaseDir() const = 0;
    virtual const char *GetFileBaseName() const = 0;
    virtual const int GetErrorCode() const = 0;         // get errno compatable error code
    virtual bool IsValid() const = 0;
    virtual bool IsWritable() const = 0;

    virtual size_t Tell() const = 0;
    virtual size_t Seek(size_t offset, FileSeekOrigin origin) = 0;
    virtual size_t GetLength() = 0;

    virtual size_t Read(uint8_t *buffer, size_t numBytes) = 0;
    virtual size_t Write(const uint8_t *buffer, size_t numBytes) = 0;
};

class DelegatedFile : public IFile
{
public:
    DelegatedFile(std::shared_ptr<IFile> pfile) :
        filePointer(pfile) 
    {
    }

    virtual const char *GetFilePath() const { return filePointer->GetFilePath(); }
    virtual const char *GetFileBaseDir() const { return filePointer->GetFileBaseDir(); }
    virtual const char *GetFileBaseName() const { return filePointer->GetFileBaseName(); }
    virtual bool IsValid() const { return filePointer && filePointer->IsValid(); }
    virtual bool IsWritable() const { return filePointer->IsWritable(); }
    virtual const int GetErrorCode() const { return filePointer->GetErrorCode(); }

    virtual size_t Tell() const { return filePointer->Tell(); }
    virtual size_t Seek(size_t offset, FileSeekOrigin origin) { return filePointer->Seek(offset, origin); }
    virtual size_t GetLength() { return filePointer->GetLength(); }

    virtual size_t Read(uint8_t *buffer, size_t numBytes) { return filePointer->Read(buffer, numBytes); }
    virtual size_t Write(const uint8_t *buffer, size_t numBytes) { return filePointer->Write(buffer, numBytes); }

protected:
    DelegatedFile() :
        filePointer(nullptr)
    {
    }

    std::shared_ptr<IFile> filePointer;
};

class UnopenedFile : public IFile
{
public:
    virtual const char *GetFilePath() const { return "<unopened_file>"; }
    virtual const char *GetFileBaseDir() const { return GetFilePath(); }
    virtual const char *GetFileBaseName() const { return GetFilePath(); }
    virtual bool IsValid() const { return false; }
    virtual bool IsWritable() const { return false; }
    virtual const int GetErrorCode() const { return ENOENT; }   // return errno no_such_file_or_directory

    virtual size_t Tell() const { return 0; }
    virtual size_t Seek(size_t /*offset*/, FileSeekOrigin /*origin*/) { return 0; }
    virtual size_t GetLength() { return 0; }

    virtual size_t Read(uint8_t * /*buffer*/, size_t /*numBytes*/) { return 0; }
    virtual size_t Write(const uint8_t * /*buffer*/, size_t /*numBytes*/) { return 0; }
};

class SysFile : public DelegatedFile
{
public:
    SysFile();
    SysFile(const std::string &path, int mode);
    virtual ~SysFile() {}
};