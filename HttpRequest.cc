#include "HttpRequest.h"
#include "logger.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::init()
{
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::parse(Buffer &buff)
{
   const char CRLF[] = "\r\n";
    if(buff.readableBytes() <= 0) {
        return false;
    }
    while(buff.readableBytes() && state_ != FINISH) {
        const char* lineEnd = std::search(buff.peek(), buff.beginWriteConst(), CRLF, CRLF + 2);
        std::string line(buff.peek(), lineEnd);
        switch(state_)
        {
        case REQUEST_LINE:
            if(!ParseRequestLine_(line)) {
                return false;
            }
            ParsePath_();
            break;    
        case HEADERS:
            ParseHeader_(line);
            if(buff.readableBytes() <= 2) {
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);
            break;
        default:
            break;
        }
        if(lineEnd == buff.beginWrite()) { break; }
        buff.retrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequest::isKeepAlive() const
{
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::ParseRequestLine_(const std::string &line)
{
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch submatch;
    if(std::regex_match(line, submatch, pattern))
    {
        method_ = submatch[1];
        path_ = submatch[2];
        version_ = submatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader_(const std::string &line)
{
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch submatch;
    if(std::regex_match(line, submatch, pattern))
    {
        header_[submatch[1]] = submatch[2];
    }
    else
    {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const std::string &line)
{
    body_ = line;
    ParsePost_();
    state_ = FINISH;
}

void HttpRequest::ParsePath_()
{
    if(path_ == "/") {
        path_ = "/index.html"; 
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::ParsePost_()
{
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_();
        if(DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                // if(UserVerify(post_["username"], post_["password"], isLogin)) {
                if(1) {
                    path_ = "/welcome.html";
                } 
                else {
                    path_ = "/error.html";
                }
            }
        }
    }     
}

void HttpRequest::ParseFromUrlencoded_()
{
    if(body_.size() == 0) { return; }

    std::string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

int HttpRequest::ConverHex(char ch)
{
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}
