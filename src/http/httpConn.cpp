//
// Created by 王澄雨 on 2022/6/21.
//

#include "httpConn.h"

using namespace std;

const char* HTTPConn::srcDir;
std::atomic<int> HTTPConn::userCount;

bool HTTPConn::isET;



HTTPConn::HTTPConn() {
    fd = -1;
    addr = {0};
    isClose = true;
}

HTTPConn::~HTTPConn() {
    closeConn();
}



void HTTPConn::init(int sockFd, const sockaddr_in &addr_) {
    assert(fd > 0);
    userCount++;
    addr  = addr_;
    fd = sockFd;
    //initialize buffer
    writeBuff.retrieveAll();
    readBuff.retrieveAll();
    isClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd, getIP(), getPort(), (int)userCount);
}


ssize_t HTTPConn::read(int *saveErrno) {
    //一次性读出所有数据 et模式非阻塞，直到
    ssize_t len = -1;
    do {
        len = readBuff.readFd(fd,saveErrno);
        if(len <= 0) {
            break;
        }
    } while(isET);
    return len;
    
}

ssize_t HTTPConn::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd,iov,iovCnt);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov[0].iov_len + iov[1].iov_len == 0) {break;} //传输结束
        //写到了第二块内存
        else if(static_cast<size_t>(len) > iov[0].iov_len) {
            iov[1].iov_base = (uint8_t*)iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if(iov[0].iov_len) {
                writeBuff.retrieveAll();
                iov[0].iov_len = 0;
            }
        }
        //还没有写到第二块内存
        else {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            writeBuff.retrieve(len);
        }
    } while(isET||toWriteBytes() > 10240); //?

    return len;

}

int HTTPConn::toWriteBytes() {
    return iov[0].iov_len + iov[1].iov_len;
}

void HTTPConn::closeConn() {
    response.unmapFile();
    if(isClose == false) {
        isClose = true;
        userCount --;
        close(fd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd, getIP(), getPort(), (int)userCount);
    }
}

int HTTPConn::getFd() const {
    return fd;
}

struct sockaddr_in HTTPConn::getAddr() const {
    return addr;
}

const char* HTTPConn::getIP() const {
    return inet_ntoa(addr.sin_addr);
}

int HTTPConn::getPort() const {
    return addr.sin_port;
}

bool HTTPConn::process() {
    request.init();

    if(readBuff.readableBytes() <= 0) { //没有数据
        return false;
    }
    else if(request.parse(readBuff)) {
        LOG_DEBUG("%s",request.getPath().c_str());
        response.init(srcDir,request.getPath(),request.isKeepAlive(),200);
    } else {
        //解析失败
        response.init(srcDir, request.getPath(), false,400);

    }
    //生成响应信息写入writebuff
    response.makeResponse(writeBuff);
    iov[0].iov_base = const_cast<char*>(writeBuff.peek());
    iov[0].iov_len = writeBuff.readableBytes();
    iovCnt = 1;
    //file
    if(response.fileLen() > 0&&response.getFile()) {
        iov[1].iov_base = response.getFile();
        iov[1].iov_len = response.fileLen();
        iovCnt = 2;
    }

    LOG_DEBUG("filesize:%d, %d  to %d", response.fileLen() , iovCnt, toWriteBytes());
    return true;



}

