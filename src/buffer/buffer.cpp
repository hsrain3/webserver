//
// Created by 王澄雨 on 2022/6/19.
//
#include "buffer.h"

Buffer::Buffer(int initBufferSize) :buffer(initBufferSize), readPos(0), writePos(0) {}
char* Buffer::beginPos() {
    return &*buffer.begin();
}

const char* Buffer::beginPos() const {
    return &*buffer.begin();
}

//开始写的位置
char* Buffer::beginWrite() {
    return beginPos() + writePos;
}

const char* Buffer::beginWrite() const {
    return beginPos() + writePos;
}

//可以读的数据大小

size_t Buffer::readableBytes() const {
    return writePos - readPos;
}

//可以写的数据大小
size_t Buffer::writableBytes() const {
    return buffer.size() - writePos;
}

//前面可以用的空间
size_t Buffer::prependableBytes() const {
    return readPos;
}

//当前读到的位置
const char* Buffer::peek() const {
    return BeginPtr() + readPos;
}

//

void Buffer::retrieve(size_t len)  {
    assert(len <= readableBytes());
    readPos += len;
}

void Buffer::retrieveUntil(const char *end) {
    assert(peek() < end);
    retrieve(end - peek());
}

//reset clear?
void Buffer::retrieveAll() {
    bzero(&buffer[0],buffer.size());
    readPos = 0;
    writePos = 0;
}

std::string Buffer::retrieveAllToStr() {
    //读出所有能读的
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
}


//没空间写 扩容
void Buffer::ensureWriteable(size_t len) {
    if(len < writableBytes()) {
        makeSpace(len);
    }
    assert(writableBytes() > len);
}

//移动写指针
void Buffer::hasWritten(size_t len) {
    return writePos + len;
}

void Buffer::append(const void *data, size_t len) {
    assert(data);
    append(static_cast<const char*> data, len);

}
//临时数组搬到 buffer
void Buffer::append(const char *str, size_t len) {
    assert(str);
    ensureWriteable(len);
    std::copy(str, str + len, beginWrite());
    havWritten(len);
}

//
void Buffer::append(const std::string &str) {
    append(str.data(), str.length());
}

void Buffer::append(const Buffer &buff) {
    append(buff.peek(),buff.readableBytes());
}

size_t Buffer::readFd(int fd, int *saveErrno) {
    char buff[65535]; //temporary array

    iovec iov[2];

    const size_t writeable = writableBytes(); //可以写的长度

    //分散读 一次系统调用读入两块内存
    iov[0].iov_base = beginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
//有符号整型
    const ssize_t len = readv(fd,iov,2);
    if(len < 0) *saveErrno = errno;
    else if(static_cast<size_t>(len) <= writeable) {
        writePos += len;

    }
    else {
        writePos = buffer.size();
        append(buff, len - writeable); //把临时数组的添加到buffer
    }
    return len;


}

size_t Buffer::writeFd(int fd, int *saveErrno) {
    size_t readSize = readableBytes(); //buffer中可以读的长度
    ssize_t len = write(fd,Peek(),readSize);
    if(len < 0) {
        *saveErrno = errno;
       // return len;
    }

    readPos += len;
    return len;

}

void Buffer::makeSpace(size_t len) {
    if(writableBytes() + prependableBytes() < len) {
        buffer.resize(writePos + len + 1);

    }
    //空间够用 把readpos - writepos copy到头
    else {
        size_t readable = readableBytes();
        std::copy(beginPos() + readPos, beginWrite(), beginPos());
        readPos = 0;
        writePos = readPos + readable;
        assert(readable == readableBytes());
    }
}
