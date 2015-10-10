
#pragma once

#include "predef.hpp"
extern "C" {    // for inet_aton
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <string>

namespace pecar
{

class Utils
{
public:
    static bool isIp(const std::string& host)
    {
        struct in_addr addr;
        return inet_aton(host.c_str(), &addr) != 0;
    }
};

}
