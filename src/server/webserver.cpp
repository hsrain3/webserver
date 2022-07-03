#include"webserver.h"
#include<iostream>
void WebServer::Start() {
    int timeMS = -1; //set epollwait block time 
    if(!isClose) {
        // todo: log module
        std::cout<<"=========Server Start ========"<<std::endl;


    }
    while(!isClose){
        if(timeoutMS > 0) {
            //获取最先超时的连接超时的时刻
            timeMS = timer->GetNextTick();
        }
        //在timeMS时间内有时间发生 epoll_wait立即返回，否则等到timeMS时间后返回（处理超时连接，否则一直阻塞）， timems = -1表示一直阻塞直到有事件
        int eventCnt = epoller->wait(timeMS); //减少epoll_wait调用次数
        //遍历事件
        for(int i = 0;i < eventCnt; i++) {
            int fd = epoller->getEventFd(i);
            uint32_t events = epoller->getEvents(i);
            //new connection come in 
            if(fd == listenFd) {
                dealListen(); 
            }

            //error case, every bit represents an event
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users.count(fd) > 0);
                CloseConn(&users[fd]);
            }
            // data arrives
            else if(events & EPOLLIN) {
                // ensure connection exists
                assert(users.count(fd) > 0);
                dealRead(&users[fd]);
            }
            // send data
            else if(events& EPOLLOUT) {
                assert(users.count(fd) > 0);
                dealWrite(&users[fd]);
            } else {
                //todo: log err
            }

        }

    }



}

void WebServer::dealListen() {
   sockaddr_in addr;
   socklen_t len = sizeof(addr);
   //监听描述符et模式？
   do {
    int fd = accept(listenFd,(struct sockaddr *)&addr, &len);
    if(fd<=0) return;//
    else if(HTTPConn::userCount >= MAX_FD) {
        sendError(fd, "Server busy!");
        //todo: log client is full
        return;
    }
    addClient(fd, addr);

   }while (listenEvent & EPOLLET); //et模式一直accept直到无fd


}

void WebServer::dealRead(HTTPConn* client) {
    assert(client);
    extendTime(client); //延长客户端超时时间
    threadpool->addTask(std::bind(&WebServer::onRead,this,client)); //bind member function
}

void WebServer::dealWrite(HTTPConn *client) {
    assert(client);
    extendTime(client);
    threadpool->addTask(std::bind(&WebServer::onWrite, this, client));

}

//延长客户端超时时间
void WebServer::extendTime(HTTPConn *client){
    assert(client);
    if(timeoutMS > 0) {
        timer->adjust(client->getFd(),timeoutMS);
    }
}

void WebServer::onRead(HTTPConn *client) {
    assert(client);
    unt ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno); //读取客户端数据
    if(ret <=  0 && readErrno != EAGAIN) {
        closeConn(client);
        return;
    }

    onProcess(client);
}

void WebServer::onProcess(HTTPConn *client) {
    if(client->process()) {  //有数据request 已经读完注册epollout事件，没有数据注册epollin
        epoller->modFd(client->getFd(),connEvent|EPOLLOUT);
    } else {
        epoller->modFd(client->getFd(),connEvent|EPOLLIN);
    }
}

void WebServer::onWrite(HTTPConn *client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    //已经写完，判断是否保持连接
    if(client->toWriteBytes() == 0){
        if(client->isKeepAlive()) {
            onProcess(client);
            return;
        }
    } else if(ret < 0) {
        //没有空间写？
        if(writeErrno == EAGAIN) {
            //继续监听写事件
            epoller->modFd(client->getFd(),connEvent|EPOLLOUT);
            return;
        }
    }
    closeConn(client);

}
//create listern fd

bool WebServer::initSocket() {
    int ret;
    sockaddr_in addr;
    if(port > 65535 || port < 1024) {
        LOG_ERROR("Port:%d error", port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //转换为网络字节序
    addr.sin_port = htonl(port);
    struct linger optLinger = { 0 };
    if(openLinger) {
        //优雅关闭
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    listenFd = socket(AF_INET, SOCK_STREAM,0);
    if(listenFd < 0) {
        LOG_ERROR("Create socket error!", port);
        return false;
    }
    ret = setsockopt(listenFd,SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if( ret < 0) {
        close(listenFd);
        LOG_ERROR("Init linger error", port);
        return false;
    }
    int optVal = 1;
    //端口复用
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optVal,sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsocketopt error !");
        close(listenFd);
        return false;
    }
    ret = listen(listenFd,6);//?
    if(ret < 0) {
         LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller->addFd(listenFd,listenEvent| EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd);
        return false;
    }

    SetFdNonblock(listenFd);
    LOG_INFO("Server port:%d", port_);
    return true;


}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    // int flag = fcntl(fd, F_GETFD, 0);
    // flag = flag  | O_NONBLOCK;
    // // flag  |= O_NONBLOCK;
    // fcntl(fd, F_SETFL, flag);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}