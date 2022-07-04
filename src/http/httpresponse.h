//
// Created by 王澄雨 on 2022/6/20.
//

#ifndef WEBSERVER_HTTPRESPONSE_H
#define WEBSERVER_HTTPRESPONSE_H

#include "../log/log.h"
#include "../buffer/buffer.h"
#include <string>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/stat.h>
#include<unistd.h>
#include <fcntl.h> 


class HTTPResponse{
public:
    HTTPResponse();
    ~HTTPResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void makeResponse(Buffer& buff);
    void unmapFile();

    char* getFile();
    size_t fileLen() const;
    void errorContent(Buffer& buff, std::string message);
    int getCode() const {return code;}
private:
    void addStateLine(Buffer& buff);
    void addHeader(Buffer& buff);
    void addContent(Buffer& buff);

    void errorHtml();
    std::string getFileType();

    int code;  //http status code
    bool isKeepAlive; //是否保持连接

    std::string path;

    std::string srcDir; //资源目录

    char* mmFile; //文件内存映射指针
    struct stat mmFileStat; //文件状态信息

    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;//后缀-类型
    static const std::unordered_map<int, std::string> CODE_STATUS; //状态码-描述

    static const std::unordered_map<int, std::string> CODE_PATH;  //状态码-路径






};
#endif //WEBSERVER_HTTPRESPONSE_H
