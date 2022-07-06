//
// Created by 王澄雨 on 2022/6/23.
//
#include "httprequest.h"

using namespace std;

const unordered_set<string> HTTPRequest::DEFAULT_HTML {
     "/index", "/register", "/login",
};

const unordered_map<string,int> HTTPRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, {"/login.html", 1},
};

void HTTPRequest::init() {
    method = path = version = body = "";
    state = REQUEST_LINE;
    header.clear();
    post.clear();

}

bool HTTPRequest::isKeepAlive() const {
    if(header.count("Connection") == 1) {
        return header.find("Connection")->second == "keep-alive" && version == "1.1";

    }
    return false;
}

bool HTTPRequest::parse(Buffer &buff) {
    const char* CRLF = "\r\n";
    if(buff.readableBytes() <= 0) {
        return false;
    }
    while(buff.readableBytes()&&state != FINISH) {
        //读一行
        const char* lineEnd = search(buff.peek(),buff.beginWriteConst(),CRLF,CRLF + 2);
        string line(buff.peek(),lineEnd);
        switch(state) {
            case REQUEST_LINE:
                //解析请求首行
                if(!parseRequestLine(line)) {
                    return false; //?
                }
                state = HEADERS;
                //解析路径
                parsePath();
                break;
            case HEADERS:
                parseHeader(line);
                if(buff.readableBytes() <= 2) { //?
                    state = FINISH;
                }
                break;
            case BODY:
                parseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.beginWrite()) {break;}
        buff.retrieveUntil(lineEnd+2);

    }
    LOG_DEBUG("[%s], [%s], [%s]", method.c_str(), path.c_str(), version.c_str());
    return true;
}

void HTTPRequest::parsePath() {
    //根目录默认定向到index.html
    if(path == "/") {
        path = "/index.html";
    }else {
        //todo: other default page
        for(auto& item: DEFAULT_HTML) {
            if(item == path) {
                path += ".html";
                break;
            }
        }
    }

}

bool HTTPRequest::parseRequestLine(const std::string &line) {
    // GET / HTTP/1.1
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, pattern)) {
        method = subMatch[1];
        path = subMatch[2];
        version = subMatch[3];

        return true;
    }
     LOG_ERROR("RequestLine Error");
    return false;

}

void HTTPRequest::parseHeader(const std::string &line) {
    regex pattern("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line,subMatch,pattern)) {
        header[subMatch[1]] = subMatch[2];
    }else {
        state = BODY;
    }
}

void HTTPRequest::parseBody(const std::string &line) {
    body = line;
    parsePost();
    state = FINISH;
     LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());

}

int HTTPRequest::convertHexToDec(char ch) {
    if(ch >= 'A'&&ch <= 'F') return ch - 'A' + 10;
    if(ch >= 'a'&&ch <= 'f') return ch - 'a' + 10;
    return ch;
}

void::HTTPRequest::parsePost() {
    if(method == "POST" && header["Content-type"] == "application/x-ww-form-urlencoded") {
        parseFormUrlEncoded();
        if(DEFAULT_HTML_TAG.count(path)) {
            int tag = DEFAULT_HTML_TAG.find(path)->second;
              LOG_DEBUG("Tag:%d", tag);
            if(tag == 0|| tag == 1) {
                bool isLogin = (tag == 1);
                if(userVerify(post["username"], post["password"], isLogin)) {
                    path = "/welcome.html";
                } else {
                    path = "/error.html";
                }
            }
        }
    }
}

void HTTPRequest::parseFormUrlEncoded() {
    if(body.size() == 0) {return;}
    string key, value;
    int num = 0;
    int n = body.size();
    int i = 0,j = 0;
    for(;i < n;i ++) {
        //url decode
        char ch = body[i];
        switch (ch) {
            case '=':
                key = body.substr(j,i - j);
                j = i + 1;
                break;
            case '+':
                body[i] = ' ';
                break;
            case '%':
                //16进制编码
                num = convertHexToDec(body[i+1])*16 + convertHexToDec(body[i+2]);
                body[i+2] = num%10 + '0';
                body[i+1] = num/10 + '0';
                i += 2;
                break;
            case '&':
                value = body.substr(j,i - j);
                j = i + 1;
                post[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;

        }
    }
    assert(j <= i);
    //only one pair
    if(post.count(key) == 0&&j < i) {
        value = body.substr(j,i-j);
        post[key] = value;
    }

}
//todo
bool HTTPRequest::userVerify(const string &name, const std::string &pwd, bool isLogin) {
    if(name == ""||pwd == "") {return false;}
    LOG_INFO("Verify name:%s pwd:%s",name.c_str(),pwd.c_str());
    MYSQL* sql;
    SQLConnRAII(&sql, SQLConnPool::getInstance());
    assert(sql);
    bool flag = false;
    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    if(!isLogin) {flag = true;}
    //查询用户及密码
    snprintf(order,256,"Select username, password from user where username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s",order);

    if(mysql_query(sql,order)) {
        mysql_free_result(res);
        return false;
    }

    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0],row[1]);
        string password(row[1]);
        //登录
        if(isLogin) {
            if(pwd == password) {flag = true;}
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } else {
            flag = false;
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);
    //注册行为
    if(!isLogin && flag == true) {
        LOG_DEBUG("register!");
        bzero(order, 256);
        snprintf(order, 256, "INDERT INTO user(username, password) VALUES('%s', '%s)", name.c_str(),pwd.c_str());
        LOG_DEBUG("%s", order);
        if(mysql_query(sql,order)) {
            LOG_DEBUG("insert error");
            flag = false;
        }
        flag = true;
    }
    SQLConnPool::getInstance()->freeConn(sql);
    LOG_DEBUG("User verify success!");
    return flag;
//
}

std::string HTTPRequest::getPath() const {
    return path;
}

string& HTTPRequest::getPath() {
    return path;
}
string HTTPRequest::getMethod() const {
    return method;
}

string HTTPRequest::getVersion() const {
    return version;
}

string HTTPRequest::getPost(const char *key) const {
    assert(key != nullptr);
    if(post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}
string HTTPRequest::getPost(const string& key) const {
    assert(key != "");
    if(post.count(key) == 1) {
        return post.find(key)->second;
    }
    return "";
}



