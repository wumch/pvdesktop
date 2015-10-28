
#pragma once

#include "predef.hpp"
#include <string>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include "Config.hpp"
#include "Crypto.hpp"
#include "Buffer.hpp"
#include "Iface.hpp"


namespace asio = boost::asio;
using asio::ip::tcp;

namespace pecar
{

class Channel:
    private boost::noncopyable,
    public boost::enable_shared_from_this<Channel>
{
private:
    const Config* config;

    asio::io_service ioService;

    tcp::socket us;
    StreamDescriptor ds;

    Buffer dr, dw, ur, uw;

    asio::deadline_timer dwPendingTimer, uwPendingTimer;
    int dwPending, uwPending;

    Crypto crypto;

public:
    Channel():
        config(Config::instance()),
        ioService(1),
        us(ioService),
        ds(ioService),
        dr(config->drBufferSize),
        dw(config->dwBufferSize),
        ur(config->urBufferSize),
        uw(config->uwBufferSize),
        dwPendingTimer(ioService, boost::posix_time::microseconds(config->dwPendingInterval)),
        uwPendingTimer(ioService, boost::posix_time::microseconds(config->uwPendingInterval)),
        dwPending(0), uwPending(0)
    {}

    void start()
    {
        if (prepareDs() && prepareUs())
        {
            work();
            boost::system::error_code err;
            ioService.run(err);
            if (err)
            {
                CS_ERR("error: " << err.message());
                Facade::setStatus(IO_RUNOUT);
            }
        }
    }

    ~Channel()
    {
        shutdown();
    }

private:
    bool prepareDs();

    bool prepareUs();
    bool connectUs();
    bool authUs(const std::string& username, const std::string& password);

    void work();

    void handleDsRead(const boost::system::error_code& err, int bytesRead);
    void handleDsRead(int bytesRead);
    void handleDsWritten(const boost::system::error_code& err, int bytesWritten);

    void handleUsRead(const boost::system::error_code& err, int bytesRead, int bytesLeft);
    void handleUsRead(int bytesRead, int bytesLeft);
    void handleUsWritten(const boost::system::error_code& err, int bytesWritten);


    int dsWritePack(const char* begin, int bytesRemain);

    void continueReadUs(const char* offset, int bytesRemain);

    uint16_t readNetUint16(const char* data) const;

    void shutdown();
};

}
