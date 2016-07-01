#include "Precompiled.h"
#include "Base/File.h"
#include "Base/Debug.h"
#include "Base/Sys.h"

class Win32File : public IFile
{
public:
    Win32File();
    Win32File(const std::string &path, int mode);
    virtual ~Win32File();

    virtual const char *GetFilePath() const;
    virtual const char *GetFileBaseDir() const;
    virtual const char *GetFileBaseName() const;
    virtual const int GetErrorCode() const;
    virtual bool IsValid() const;
    virtual bool IsWritable() const;

    virtual size_t Tell() const;
    virtual size_t Seek(size_t offset, FileSeekOrigin origin);
    virtual size_t GetLength();

    virtual size_t Read(uint8_t *buffer, size_t numBytes);
    virtual size_t Write(const uint8_t *buffer, size_t numBytes);

private:
    std::string fileName;
    std::string baseDir;
    FILE *fileDescriptor;
    int openMode;
    int errorCode;
};

Win32File::Win32File() :
    fileDescriptor(nullptr),
    fileName(""),
    openMode(0),
    errorCode(0)
{
}

Win32File::Win32File(const std::string &path, int mode) :
    fileDescriptor(nullptr),
    fileName(path),
    openMode(mode),
    errorCode(0)
{
    const char *openModeStr = "rb";
    if (mode == FileOpen_WriteAppend)
    {
        openModeStr = "ab";
    }
    else if (mode == FileOpen_WriteTruncate)
    {
        openModeStr = "wb";
    }

    fileDescriptor = fopen(path.c_str(), openModeStr);
    if (!fileDescriptor)
    {
        errorCode = errno;
        return;
    }

    rewind(fileDescriptor);

    // get base dir
    auto lastPathSeparatorPos = fileName.find_last_of('/');
    if (lastPathSeparatorPos == std::string::npos)
    {
        lastPathSeparatorPos = fileName.find_last_of('\\');
    }

    if (lastPathSeparatorPos != std::string::npos)
    {
        baseDir = fileName.substr(0, lastPathSeparatorPos + 1);
    } else
    {
        baseDir = "./";
    }
}

Win32File::~Win32File()
{
    if (fileDescriptor)
    {
        fclose(fileDescriptor);
    }
}

const char *Win32File::GetFilePath() const
{
    return fileName.c_str();
}

const char *Win32File::GetFileBaseDir() const
{
    return baseDir.c_str();
}

const char *Win32File::GetFileBaseName() const
{
    auto lastPathSeparatorPos = fileName.find_last_of('/');
    if (lastPathSeparatorPos == std::string::npos)
    {
        lastPathSeparatorPos = fileName.find_last_of('\\');
    }

    lastPathSeparatorPos = (lastPathSeparatorPos == std::string::npos ? 0 : lastPathSeparatorPos + 1);
    return &fileName.at(lastPathSeparatorPos);
}

const int Win32File::GetErrorCode() const
{
    return errorCode;
}

bool Win32File::IsValid() const
{
    return (fileDescriptor != nullptr);
}

bool Win32File::IsWritable() const
{
    return (openMode == FileOpen_WriteAppend) ||
           (openMode == FileOpen_WriteTruncate);
}

size_t Win32File::Tell() const
{
    return ftell(fileDescriptor);
}

size_t Win32File::Seek(size_t offset, FileSeekOrigin origin)
{
    int newOrigin = 0;

    switch (origin)
    {
    case FileSeek_Beg: newOrigin = SEEK_SET; break;
    case FileSeek_Cur: newOrigin = SEEK_CUR; break;
    case FileSeek_End: newOrigin = SEEK_END; break;
    }

    if (fseek(fileDescriptor, (long int)offset, newOrigin))
    {
        return 0;
    }

    return Tell();
}

size_t Win32File::GetLength()
{
    const size_t pos = Tell();
    if (pos >= 0)
    {
        this->Seek(0, FileSeek_End);
        size_t size = Tell();
        Seek(pos, FileSeek_Beg);
        return size;
    }

    return 0;
}

size_t Win32File::Read(uint8_t *buffer, size_t numBytes)
{
    return fread(buffer, 1, numBytes, fileDescriptor);
}

size_t Win32File::Write(const uint8_t *buffer, size_t numBytes)
{
    return fwrite(buffer, 1, numBytes, fileDescriptor);
}

SysFile::SysFile() :
    DelegatedFile(nullptr)
{
    filePointer = std::make_shared<UnopenedFile>();
}

SysFile::SysFile(const std::string &path, int mode)
{
    filePointer = std::make_shared<Win32File>(path, mode);
    if (!filePointer->IsValid())
    {
        Sys_ErrorPrintf("Failed to open file: %s (%s)\n", path.c_str(), strerror(filePointer->GetErrorCode()));
        filePointer = std::make_shared<UnopenedFile>();
    }
}