#ifndef BUFFER_H
#define BUFFER_H
#include <unistd.h> 
#include <vector>
#include<atomic>
#include<string>
#include <cstring>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/errno.h>
class Buffer{
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;
    size_t writableBytes() const;
    size_t readableBytes() const;

    const char* peek() const;  //开始读的位置

    size_t readFd(int fd, int* saveErrno);  //返回保存的errno
    size_t writeFd(int fd, int* saveErrno);
    size_t prependableBytes() const;
    void retrieve(size_t len) ;
    void retrieveUntil(const char* end);
    void retrieveAll();
    std::string retrieveAllToStr();
    void ensureWriteable(size_t len);
    void hasWritten(size_t len);

    void append(const Buffer& buff);
    void append(const void* data, size_t len);
    void append(const char* data, size_t len);
    void append(const std::string& str);
    char* beginWrite();
    const char* beginWrite() const;

private:
    char* beginPos();  //buffer start pos
    const char* beginPos() const; //?

    void makeSpace(size_t len);
    std::vector<char>buffer;
    std::atomic<size_t> readPos;
    std::atomic<size_t> writePos; 

    
    

};
#endif