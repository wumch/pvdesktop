
#include "predef.hpp"
#include <boost/shared_ptr.hpp>
#include "Config.hpp"
#include "Channel.hpp"

int main(int argc, char* argv[])
{
    pecar::Config::mutableInstance()->init(argc, argv);
    boost::shared_ptr<pecar::Channel> channel(new pecar::Channel);
    channel->start();
}
