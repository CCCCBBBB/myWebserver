#pragma once
#include "Buffer.h"


#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>   


class HttpRequest
{
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
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };   
    HttpRequest() { init(); }
    ~HttpRequest() = default;

    void init();
    bool parse(Buffer& buff);

    std::string path() const { return path_; }
    std::string& path()  { return path_; }
    std::string method() const { return method_; }
    std::string version() const { return version_; }

    std::string GetPost(const std::string& key) const 
    {
        if(post_.count(key) == 1) {
            return post_.find(key)->second;
        }
        return "";
    }

    std::string GetPost(const char* key) const 
    {
        if(post_.count(key) == 1) {
            return post_.find(key)->second;
        }
        return "";
    }
    bool isKeepAlive() const;
private:

    bool ParseRequestLine_(const std::string& line);
    void ParseHeader_(const std::string& line);
    void ParseBody_(const std::string& line);

    void ParsePath_();
    void ParsePost_();
    void ParseFromUrlencoded_();
    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static int ConverHex(char ch);
    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};