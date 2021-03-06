
#pragma once

#include "predef.hpp"
#include <vector>
#include <iostream>

namespace pecar
{

enum Status {
    US_RESOLVING      = 101,
    US_CONNECTING     = 102,
    US_CONNECT_ERR    = 103,
    US_CONNECTED      = 104,

    IO_RUNOUT         = 201,
};

enum AuthRes {
    AUTH_CODE_OK,            // 成功
    AUTH_CODE_EXPIRED,       // 会员过期
    AUTH_CODE_TRAFFIC_EXHAUST, // 阶段流量耗尽
};

class IoctlError:
    public std::exception
{
    int _code;
    std::string message;
public:
    IoctlError(int __code, const std::string& _message) _GLIBCXX_USE_NOEXCEPT:
        exception(), _code(__code), message(_message)
    {}

    int code() const throw()
    {
        return _code;
    }

    const char* what() const _GLIBCXX_USE_NOEXCEPT
    {
        return message.c_str();
    }

    virtual ~IoctlError() _GLIBCXX_USE_NOEXCEPT {}
};

struct RouteItem
{
public:
    uint32_t ip, mask, gateway;

    RouteItem(uint32_t _ip, uint32_t _mask, uint32_t _gateway):
        ip(_ip), mask(_mask), gateway(_gateway)
    {}
};
typedef std::vector<RouteItem> RouteList;

struct Server
{
public:
    std::string host;
    uint16_t port;

    Server(const std::string& _host, uint16_t _port):
        host(_host), port(_port)
    {}
};
typedef std::vector<Server> ServerList;

class Facade
{
public:
    static RouteList getRouteList();

    static Server getServer();
    static Server getServer(const ServerList& excludes);

    static void setStatus(Status status);

    static void setAuthRes(AuthRes res);
};

}
