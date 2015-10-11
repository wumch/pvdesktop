/*
 * Facade.cpp
 *
 *  Created on: 2015年10月10日
 *      Author: wumch
 */

#include "Facade.hpp"
#include <arpa/inet.h>

namespace pecar
{

RouteList Facade::getRouteList()
{
    RouteList routes;
    routes.push_back(RouteItem(inet_addr("112.90.51.173"), inet_addr("255.255.255.255"), inet_addr("10.0.0.3")));
    routes.push_back(RouteItem(inet_addr("222.161.220.33"), inet_addr("255.255.255.255"), inet_addr("10.0.0.3")));
    // todo: implement
    return routes;
}

Server Facade::getServer()
{
    return Server("192.168.1.9", 1723);
    return Server("pecar.wumch.com", 1723);
}

Server Facade::getServer(const ServerList& excludes)
{
    return Server("pecar.wumch.com", 1723);
}

void Facade::setStatus(Status status)
{
    CS_DUMP(status);
}

void Facade::setAuthRes(AuthRes res)
{
    CS_DUMP(res);
}

} /* namespace pecar */
