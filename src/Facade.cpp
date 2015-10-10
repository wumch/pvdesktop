/*
 * Facade.cpp
 *
 *  Created on: 2015年10月10日
 *      Author: wumch
 */

#include "Facade.hpp"

namespace pecar
{

RouteList Facade::getRouteList()
{
    RouteList routes;
    // todo: implement
    return routes;
}

Server Facade::getServer()
{
    return Server("pecar.wumch.com", 1723);
}

Server Facade::getServer(const ServerList& excludes)
{
    return Server("pecar.wumch.com", 1723);
}

} /* namespace pecar */
