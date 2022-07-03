//
// Created by 王澄雨 on 2022/7/1.
//

#include "httpresponse.h"

using namespace std;

const unordered_map<string,string> HTTPResponse::SUFFIX_TYPE = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/nsword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "},
};

const unordered_map<int string> HTTPResponse::CODE_STATUS = {
        {200,"OK"},
        {400,"Bad Request"},
        {403,"Forbidden"},
        {404, "Not Found"},
};

const unordered_map<int string> HTTPResponse::CODE_PATH = {
        {400,"/400.html"},
        {403, "/403.html"},
        {404, "/404.html"},
};
HTTPResponse::HTTPResponse() {
    code = -1;
    path = srcDir = "";
    isKeepAlive = false;
    mmFile = nullptr;
    mmFileStat ={0};
}

HTTPResponse::~HTTPResponse() {
    UnmapFile();
}

void HTTPResponse::init(const std::string &srcDir, std::string &path, bool isKeepAlive, int code) {
    assert(srcDir != "");
    if(mmFile) {UnmapFile();} //先解除映射
    this->code = code;
    this->isKeepAlive = isKeepAlive;
    this->path = path;
    this->srcDir = srcDir;
    this->mmFile = mmFile;
    mmFileStat = {0};

}

void HTTPResponse::makeResponse(Buffer &buff) {
    if(stat((srcDir + path).data(),&mmFileStat) < 0||S_ISDIR(mmFileStat.st_mode)) {
        code = 404;
    }
    else if(!(mmFileStat.st_mode&S_IROTH)) { //没有读权限？
        code = 403;
    } else if(code == -1) {
        code = 200;
    }
    errorHtml();
    addStateLine(buff);
    addHeader(buff);
    addContent(buff);

}

char* HTTPResponse::getFile() {
    return mmFile;
}

size_t HTTPResponse::FileLen() const {
    return mmFileStat.st_size;
}

void HTTPResponse::errorHtml() {
    if(CODE_PATH.count(code) == 1) {
        path = CODE_PATH.find(code)->second;
        stat((srcDir + path).data(),&mmFileStat);
    }
}

//添加响应状态行
void HTTPResponse::addStateLine(Buffer &buff) {
    string status;
    if(CODE_STATUS.count(code) == 1) {
        status = CODE_STATUS.find(code)->second;
    }
    else {
        code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.append("HTTP/1.1" + to_string(code) + " " + status + "\r\n");
}

void HTTPResponse::addHeader(Buffer &buff) {
    buff.append("Connection: ");
    if(isKeepAlive) {
        buff.append("keep-alive\r\n");
        buff.append("keep-alive: max=6, timeout=120\r\n");

    } else {
        buff.append("close\r\n");
    }
    buff.append("Content-type: " + getFileType() + "\r\n");
}

//添加响应体
void HTTPResponse::addContent(Buffer &buff) {
    int srcFd = open((srcDir + path).data(),O_RDONLY);
    if(srcFd < 0) {
        errorContent(buff,"File not found!");
        return;
    }
    //将文件映射到内存
    LOG_DEBUG("file path %s",(srcDir + path).data());
    int* mmRet = (int*)mmap(0,mmFileStat.st_size, PROT_READ, MAP_PRIVATE,srcFd,0);
    if(*mmRet == -1) {
        errorContent(buff,"File not Found!");
        return;
    }
    mmFile = (char*)mmRet;
    close(srcFd);
    buff.append("Content-length: " + to_string(mmFileStat.st_size)+"\r\n\r\n");
}

void HTTPResponse::unmapFile() {
    if(mmFile) {
        munmap(mmFile,mmFileStat.st_size);
        mmFile = nullptr;
    }
}

string HTTPResponse::getFileType() {
    //判断文件类型
    string::size_type idx = path.find_last_of('.');
    if(idx == string::npos) {
        return "text/plain";
    }

    string suffix = path.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;

    }
    return "text/plain";
}

void HTTPResponse::errorContent(Buffer &buff, string message) {
    string body,status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}






