
#pragma once

#include "../predef.hpp"
extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <netinet/in.h>
}
#include <cstring>
#include <string>

namespace pecar
{

class Iface
{
private:
    int fd;
    typedef struct rtentry RouteEntry;
    typedef struct sockaddr_in SockAddrIn;

public:
    int getIface(const std::string& name) const
    {
        int iface = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
        if (iface >= 0)
        {
            ifreq ifr;
            std::memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
            std::memcpy(ifr.ifr_name, name.data(), std::min(name.size(), sizeof(ifr.ifr_name)));
            if (ioctl(iface, TUNSETIFF, &ifr) < 0)
            {
                CS_DIE("Cannot get TUN interface");
            }
        }
        CS_SAY("done");

        return iface;
    }

    static void addRoute(int iface, uint32_t ip, uint32_t mask, uint32_t gatewayIp)
    {
        RouteEntry entry;
        SockAddrIn gateway; // Gateway IP address

        std::memset(&entry, 0, sizeof(entry));

        gateway.sin_family = AF_INET;
        gateway.sin_addr.s_addr = gatewayIp;
        gateway.sin_port = 0;

        static_cast<SockAddrIn*>(&entry.rt_dst)->sin_family = AF_INET;
        static_cast<SockAddrIn*>(&entry.rt_dst)->sin_addr.s_addr = ip;
        static_cast<SockAddrIn*>(&entry.rt_dst)->sin_port = 0;

        static_cast<SockAddrIn*>(&entry.rt_genmask)->sin_family = AF_INET;
        static_cast<SockAddrIn*>(&entry.rt_genmask)->sin_addr.s_addr = mask;
        static_cast<SockAddrIn*>(&entry.rt_genmask)->sin_port = 0;

        std::memcpy((void *) &entry.rt_gateway, &gateway, sizeof(gateway));
        entry.rt_flags = RTF_UP | RTF_GATEWAY;
        int err = ioctl(iface, SIOCADDRT, &entry);
        if (err < 0)
        {
            CS_ERR("SIOCADDRT failed, ret->" << err);
        }
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

}

}
