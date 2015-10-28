
#pragma once

#include "../predef.hpp"
extern "C" {
#include "tap-windows.h"
}
#include <cerrno>
#include <cstring>
#include <exception>
#include <string>
#include <boost/lexical_cast.hpp>
#include "../Facade.hpp"


#define __PECAR_IOCTL(fd, command, ...)                                     \
    do {                                                                    \
        if (CS_BUNLIKELY(ioctl(fd, command, __VA_ARGS__) < 0))              \
        {                                                                   \
            if (errno == EEXIST)                                            \
            {                                                               \
                CS_ERR("ioctl error: " << strerror(errno))                  \
            }                                                               \
            else                                                            \
            {                                                               \
                throw IoctlError(errno, std::string(__FILE__) + ":"         \
                    + boost::lexical_cast<std::string>(__LINE__) + ":\t"    \
                    + std::string("ioctl(")                                 \
                    + boost::lexical_cast<std::string>(fd) + ", "           \
                    + boost::lexical_cast<std::string>(command)             \
                    + ") failed ("                                          \
                    + boost::lexical_cast<std::string>(errno)               \
                    + "): "                                                 \
                    + strerror(errno));                                     \
            }                                                               \
        }                                                                   \
    } while (false)

namespace pecar
{

class Iface
{
private:
    int fd;
    typedef struct rtentry RouteEntry;
    typedef struct sockaddr_in SockAddrIn;

public:
    static int getIface(const std::string& name)
    {
        int iface = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
        if (iface >= 0)
        {
            ifreq ifr;
            std::memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
            std::memcpy(ifr.ifr_name, name.data(), std::min(name.size(), sizeof(ifr.ifr_name)));
            __PECAR_IOCTL(iface, TUNSETIFF, &ifr);
        }
        return iface;
    }

    static void addRoute(int ifacea, const RouteItem& route)
    {
        CS_SAY("adding route");
        return;
        int iface = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        RouteEntry entry;
        std::memset(&entry, 0, sizeof(entry));
        SockAddrIn* addr;

        char ifname[] = "tun1";
        entry.rt_dev = ifname;

        addr = reinterpret_cast<SockAddrIn*>(&entry.rt_gateway);
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = route.gateway;

        addr = reinterpret_cast<SockAddrIn*>(&entry.rt_dst);
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = route.ip;

        addr = reinterpret_cast<SockAddrIn*>(&entry.rt_genmask);
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = route.mask;

        entry.rt_flags = RTF_UP | RTF_HOST;
        __PECAR_IOCTL(iface, SIOCADDRT, &entry);
        close(iface);
    }

    static void setMtu(int iface, uint16_t mtu)
    {
        __PECAR_IOCTL(iface, SIOCSIFMTU, mtu);
    }

    static bool ip2uint(const char* ipStr, uint32_t& ipUint)
    {
        struct in_addr addr;
        bool res = inet_aton(ipStr, &addr) == 0;
        ipUint = addr.s_addr;
        return res;
    }

    static int setDefGateway(const char * deviceName, const char * defGateway)
    {
        int sockfd;
        struct rtentry rm;
        struct sockaddr_in ic_gateway; // Gateway IP address
        int err;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1)
        {
            CS_ERR("socket is -1");
            return -1;
        }

        memset(&rm, 0, sizeof(rm));

        ic_gateway.sin_family = AF_INET;
        ic_gateway.sin_addr.s_addr = inet_addr(defGateway);
        ic_gateway.sin_port = 0;

        ((struct sockaddr_in*) &rm.rt_dst)->sin_family = AF_INET;
        ((struct sockaddr_in*) &rm.rt_dst)->sin_addr.s_addr = 0;
        ((struct sockaddr_in*) &rm.rt_dst)->sin_port = 0;

        ((struct sockaddr_in*) &rm.rt_genmask)->sin_family = AF_INET;
        ((struct sockaddr_in*) &rm.rt_genmask)->sin_addr.s_addr = 0;
        ((struct sockaddr_in*) &rm.rt_genmask)->sin_port = 0;

        memcpy((void *) &rm.rt_gateway, &ic_gateway, sizeof(ic_gateway));
        rm.rt_flags = RTF_UP | RTF_GATEWAY;
        if ((err = ioctl(sockfd, SIOCADDRT, &rm)) < 0)
        {
            CS_ERR("SIOCADDRT failed , ret->" << err);
            return -1;
        }
        return 0;
    }
};

}
