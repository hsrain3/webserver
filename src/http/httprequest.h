//
// Created by 王澄雨 on 2022/6/20.
//

#ifndef WEBSERVER_HTTPREQUEST_H
#define WEBSERVER_HTTPREQUEST_H

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <mysql/mysql.h>
#include <cstdio>
#include <cassert>
#include<algorithm>
#include<openssl/md5.h>
class HTTPRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,

    };
    HTTPRequest(){init();}
    ~HTTPRequest() = default;
    void init();
    bool parse(Buffer& buff);
    bool isKeepAlive() const;
    std::string getPath() const;
    std::string& getPath();
    std::string getMethod() const;
    std::string getVersion() const;
    std::string getPost(const std::string& key) const;
    std::string getPost(const char* key) const;


private:
    bool parseRequestLine(const std::string& line);
    void parseHeader(const std::string& line);
    void parseBody(const std::string& line);
    static std::string getPasswordMd5(const std::string& pwd);
    void parsePath();
    void parsePost();
    void parseFormUrlEncoded();

    static bool userVerify(const std::string& name, const std::string& pwd, bool isLogin);
    PARSE_STATE state;
    std::string method, path, version, body;
    std::unordered_map<std::string, std::string> header;
    std::unordered_map<std::string, std::string>post;
    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int convertHexToDec(char ch);

};
#endif //WEBSERVER_HTTPREQUEST_H
