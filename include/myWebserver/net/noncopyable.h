#pragma once

/*
*To prevent objects of a class from being copy-constructed or assigned to each other.
*
*/
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:  
    noncopyable() = default;
    ~noncopyable() = default;
};
