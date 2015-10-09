
#pragma once

#include "predef.hpp"
#if _PECAR_IS_POSIX
#   include <boost/asio/posix/stream_descriptor.hpp>
#   include "posix/Iface.hpp"
#elif _PECAR_IS_WINDOWS
#   ifdef BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE
#       include <boost/asio/windows/stream_handle.hpp>
#       include "windows/Iface.hpp"
#   else
#       error asio::windows::stream_handle is not useable
#   endif
#endif


namespace asio = boost::asio;

namespace pecar
{

#if _PECAR_IS_POSIX
typedef asio::posix::stream_descriptor StreamDescriptor;
#elif _PECAR_IS_WINDOWS_PC
typedef asio::windows::stream_handle StreamDescriptor;
#endif

}
