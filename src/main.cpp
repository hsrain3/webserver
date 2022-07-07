//
// Created by 王澄雨 on 2022/7/3.
//

#include<unistd.h>

#include "server/webserver.h"

int  main() {
    WebServer server(
            1316,3,60000,false,3306,"root","root","webServer", 12,6,false,1,1024
            );
    server.Start();
}