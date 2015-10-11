
#pragma once

#include "predef.hpp"
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>


extern int main(int, char*[]);

namespace pecar
{

class Config
{
    friend int ::main(int, char*[]);
private:
    static Config _instance;

    Config():
        desc("allowed config options")
    {}

    void load(boost::filesystem::path file);

    static Config* mutableInstance()
    {
        return &_instance;
    }

    void initDesc();

public:
    static const Config* instance()
    {
        return &_instance;
    }

    void init(int argc, char* argv[]);

public:
    std::string programName;
    boost::filesystem::path pidFile;

    bool usTcpNodelay;

    std::string ifaceName;
    uint16_t ifaceMtu;

    uint8_t passwordLen;

    typedef struct timeval Timeval;
    Timeval dsSendTimeout, usSendTimeout;

    int uwPendingInterval, dwPendingInterval;

    std::size_t drBufferSize, dwBufferSize,
        urBufferSize, uwBufferSize;
    std::size_t userPassTotalLen;

    bool usLinger;
    int usLingerTimeout;

    boost::program_options::variables_map options;
    boost::program_options::options_description desc;

private:    // handy utility
    Timeval toTimeval(std::time_t val) const
    {
        Timeval tval = {val, 0};
        return tval;
    }

};

}
