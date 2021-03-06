
#include "Channel.hpp"
#include <iostream>
#include <cstring>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include "Utils.hpp"
#include "Facade.hpp"


#define __PECAR_KICK_IF(cond) if (CS_UNLIKELY(cond)) { CS_SAY("[" << (uint64_t)this << "] will return"); shutdown(); return; }

#define __PECAR_KICK_IF_ERR(err) if (CS_UNLIKELY(err)) { CS_SAY("[" << (uint64_t)this << "]: [" << CS_OC_RED(err.message()) << "]"); shutdown(); return; }

#define __PECAR_FALSE_IF(err) if (CS_UNLIKELY(err)) { CS_SAY("[" << (uint64_t)this << "] will return false."); shutdown(); return false; }

#define __PECAR_BUFFER(buf)     asio::buffer(buf.data, buf.capacity)

std::ostream& operator<<(std::ostream& out, const pecar::Server& server)
{
    return out << server.host << ':' << server.port;
}

namespace pecar
{

enum {
    ip_pack_len_end = 4,
    ip_pack_min_len = 20,
    ip_pack_max_len = 65535
};
BOOST_STATIC_ASSERT(ip_pack_min_len >= ip_pack_len_end);

bool Channel::prepareDs()
{
    int iface;
    try
    {
        iface = Iface::getIface(config->ifaceName);

        RouteList routes = Facade::getRouteList();
        for (RouteList::iterator it = routes.begin(); it != routes.end(); ++it)
        {
            Iface::addRoute(iface, *it);
        }
//        Iface::setMtu(iface, config->ifaceMtu);
    }
    catch (const IoctlError& e)
    {
        CS_ERR("failed on get interface: " << e.what());
        return false;
    }

    ds.assign(iface);
    return true;
}

bool Channel::prepareUs()
{
    return connectUs() && authUs("wumch", "test");
}

bool Channel::authUs(const std::string& username, const std::string& password)
{
    std::size_t keyIvLen = crypto.genKeyIv(uw.data);
    ur.data[0] = username.size();
    ur.data[1] = password.size();
    std::memcpy(ur.data + 2, username.data(), username.size());
    std::memcpy(ur.data + 2 + username.size(), password.data(), password.size());
    std::size_t userPassLen = username.size() + password.size();
    if (userPassLen > config->userPassTotalLen)
    {
        CS_ERR("length of username + password over " << config->userPassTotalLen);
        return false;
    }
    crypto.encrypt(ur.data, 2 + userPassLen, uw.data + keyIvLen);

    {
        boost::system::error_code err;
        asio::write(us, __PECAR_BUFFER(uw), asio::transfer_exactly(keyIvLen + 2 + userPassLen), err);
        if (err)
        {
            CS_SAY("error on greet to upstream: " << err.message());
            return false;
        }
    }

    {
        boost::system::error_code err;
        asio::read(us, __PECAR_BUFFER(ur), asio::transfer_exactly(1), err);
        if (err)
        {
            CS_SAY("error on greet to upstream: " << err.message());
            return false;
        }
    }

    crypto.decrypt(ur.data, 1, uw.data);
    Facade::setAuthRes(AuthRes(uw.data[0]));

    switch (uw.data[0])
    {
    case AUTH_CODE_OK:
        CS_SAY("AUTH_CODE_OK");
        return true;
        break;
    case AUTH_CODE_EXPIRED:
        CS_ERR("AUTH_CODE_EXPIRED");
        return false;
        break;
    case AUTH_CODE_TRAFFIC_EXHAUST:
        CS_ERR("AUTH_CODE_TRAFFIC_EXHAUST");
        return false;
        break;
    default:
        CS_ERR("auth: unknown error");
        return false;
    }
}

bool Channel::connectUs()
{
    Server server = Facade::getServer();

    Facade::setStatus(US_CONNECTING);
    uint32_t ipUint;
    bool connected = false;
    if (Iface::ip2uint(server.host.c_str(), ipUint))
    {
        tcp::endpoint endpoint(asio::ip::address_v4(ipUint), server.port);
        boost::system::error_code err;
        us.connect(endpoint, err);
        if (!err)
        {
            connected = true;
        }
    }
    else
    {
        tcp::resolver resolver(ioService);
        tcp::resolver::query query(server.host, boost::lexical_cast<std::string>(server.port));
        for (tcp::resolver::iterator it = resolver.resolve(query); it != tcp::resolver::iterator(); ++it)
        {
            boost::system::error_code err;
            us.connect(*it, err);
            if (!err)
            {
                connected = true;
                break;
            }
        }
    }
    if (!connected)
    {
        Facade::setStatus(US_CONNECT_ERR);
        CS_ERR("can not connect to " << server);
    }
    else
    {
        Facade::setStatus(US_CONNECTED);
    }

    return connected;
}

void Channel::work()
{
    ds.async_read_some(__PECAR_BUFFER(dr),
        boost::bind(&Channel::handleDsRead, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
    us.async_read_some(__PECAR_BUFFER(ur),
        boost::bind(&Channel::handleUsRead, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred, 0));
}

void Channel::handleDsRead(const boost::system::error_code& err, int bytesRead)
{
    __PECAR_KICK_IF_ERR(err);
    handleDsRead(bytesRead);
}

void Channel::handleDsRead(int bytesRead)
{
    if (uwPending != 0)
    {
        return uwPendingTimer.async_wait(boost::bind(&Channel::handleDsRead, shared_from_this(), bytesRead));
    }
    CS_DUMP(bytesRead);
    crypto.encrypt(dr.data, bytesRead, uw.data);
    ++uwPending;
    asio::async_write(us, __PECAR_BUFFER(uw), asio::transfer_exactly(bytesRead),
        boost::bind(&Channel::handleUsWritten, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
    ds.async_read_some(__PECAR_BUFFER(dr),
        boost::bind(&Channel::handleDsRead, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Channel::handleUsWritten(const boost::system::error_code& err, int bytesWritten)
{
    CS_DUMP(bytesWritten);
    __PECAR_KICK_IF_ERR(err);
    --uwPending;
}

void Channel::handleUsRead(const boost::system::error_code& err, int bytesRead, int bytesLeft)
{
    __PECAR_KICK_IF_ERR(err);
    handleUsRead(bytesRead, bytesLeft);
}

void Channel::handleUsRead(int bytesRead, int bytesLeft)
{
    if (dwPending != 0)
    {
        return dwPendingTimer.async_wait(boost::bind(&Channel::handleUsRead, shared_from_this(), bytesRead, bytesLeft));
    }
    CS_DUMP(bytesRead);
    CS_DUMP(bytesLeft);
    crypto.decrypt(ur.data, bytesRead, dw.data + bytesLeft);
    const int totalBytes = bytesLeft + bytesRead;
    if (CS_BLIKELY(totalBytes >= ip_pack_len_end))
    {
        const int firstPackLen = readNetUint16(dw.data + 2);
        __PECAR_KICK_IF(firstPackLen < ip_pack_min_len);

        if (firstPackLen == totalBytes)
        {
            ++dwPending;
            asio::async_write(ds, __PECAR_BUFFER(dw), asio::transfer_exactly(firstPackLen),
                boost::bind(&Channel::handleDsWritten, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
            us.async_read_some(__PECAR_BUFFER(ur), boost::bind(&Channel::handleUsRead, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred, 0));
        }
        else if (firstPackLen < totalBytes)
        {
            int bytesRemain = totalBytes;
            const char* packBegin = dw.data;
            int bytesWritten;
            while (bytesRemain >= ip_pack_min_len && (bytesWritten = dsWritePack(packBegin, bytesRemain)) > 0)
            {
                bytesRemain -= bytesWritten;
                packBegin += bytesWritten;
            }
            if (bytesRemain == 0)
            {
                us.async_read_some(__PECAR_BUFFER(ur), boost::bind(&Channel::handleUsRead, shared_from_this(),
                    asio::placeholders::error, asio::placeholders::bytes_transferred, bytesRemain));
            }
            else
            {
                continueReadUs(packBegin, bytesRemain);
            }
        }
        else
        {
            continueReadUs(dw.data, totalBytes);
        }
    }
    else
    {
        continueReadUs(dw.data, totalBytes);
    }
}

void Channel::handleDsWritten(const boost::system::error_code& err, int bytesWritten)
{
    CS_DUMP(bytesWritten);
    __PECAR_KICK_IF_ERR(err);
    --dwPending;
}

void Channel::continueReadUs(const char* offset, int bytesRemain)
{
    if (offset != dw.data && bytesRemain > 0)
    {
        const int space = offset - dw.data;
        if (bytesRemain < space)
        {
            std::memcpy(dw.data, offset, bytesRemain);
        }
        else
        {
            if ((space << 3) < bytesRemain && space < 256)
            {
                std::memcpy(ur.data, offset, bytesRemain);
                std::memcpy(dw.data, ur.data, bytesRemain);
            }
            else
            {
                const char* src = offset;
                char* dest = dw.data;
                for (int remains = bytesRemain;
                    remains > 0;
                    remains -= space, src += space, dest += space)
                {
                    std::memcpy(dest, src, std::min(space, remains));
                }
            }
        }
    }
    us.async_read_some(asio::buffer(ur.data, ur.capacity - bytesRemain),
        boost::bind(&Channel::handleUsRead, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred, bytesRemain));
}

int Channel::dsWritePack(const char* begin, int bytesRemain)
{
    int packLen = readNetUint16(begin + 2);
    if (bytesRemain < packLen)
    {
        return -1;
    }
    else
    {
        ++dwPending;
        asio::async_write(ds, asio::buffer(begin, packLen), asio::transfer_exactly(packLen),
            boost::bind(&Channel::handleDsWritten, shared_from_this(),
                asio::placeholders::error, asio::placeholders::bytes_transferred));
        return packLen;
    }
}

uint16_t Channel::readNetUint16(const char* data) const
{
    return (static_cast<uint8_t>(*data) << 8) + static_cast<uint8_t>(*(data + 1));
}

void Channel::shutdown()
{
    CS_SAY("shutdown");
    boost::system::error_code err;
    if (us.is_open())
    {
        us.close(err);  // TODO: ensure close(ifd) not called here.
    }
    if (us.native() >= 0)
    {
        close(us.native());
    }
    if (us.is_open())
    {
        us.shutdown(tcp::socket::shutdown_both, err);
        us.close(err);
    }
}

}

#undef __PECAR_KICK_IF
#undef __PECAR_KICK_IF_ERR
#undef __PECAR_FALSE_IF
#undef __PECAR_BUFFER
