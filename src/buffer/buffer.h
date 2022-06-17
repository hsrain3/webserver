#ifndef BUFFER_H
#define BUFFER_H
#include <unistd.h> 
#include <vector>
#include<atomic>
class Buffer{
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;
    size_t writableBytes() const;
    size_t ReadableBytes() const;

private:
    char* BeginPos();  //buffer start pos
    std::vector<char>buffer;
    std::atomic<size_t> readPos;
    std::atomic<size_t> writePos; 

    
    

};
#endif