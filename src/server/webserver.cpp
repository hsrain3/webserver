#include<webserver.h>
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
        //在timeMS时间内有时间发生 epoll_wait立即返回，否则等到timeMS时间后返回， -1表示一直阻塞知道有事件
        int eventCnt = epoller->wait(timeMS);
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
    extendTime(client); //延长客户端时间
    threadpool->addTask(client); //todo: add function

}

