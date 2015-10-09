
#pragma once

#include "predef.hpp"
#include <string>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/posix/stream_descriptor_service.hpp>
#include <boost/static_assert.hpp>
#include "Config.hpp"
#include "Crypto.hpp"
#include "Buffer.hpp"


namespace asio = boost::asio;
using asio::ip::tcp;

namespace pecar
{

class Channel
{
private:
    const Config* config;

    asio::io_service ioService;

    tcp::socket us;
    StreamDescriptor ds;

    Buffer bufdr, bufdw, bufur, bufuw;

public:
    Channel():
        ioService(config.)
};

}
