
#include "Config.hpp"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#define _PECAR_OUT_CONFIG_PROPERTY(property)     << CS_OC_GREEN(#property) << ":\t\t" << CS_OC_RED(property) << std::endl

namespace pecar
{

void Config::init(int argc, char* argv[])
{
    boost::filesystem::path programPath = argv[0];
#if BOOST_VERSION > 104200
    programName = programPath.filename().string();
#else
    programName = programPath.filename();
#endif

    boost::filesystem::path defaultConfig("etc/" + programName + ".conf");
    desc.add_options()
        ("help,h", "show this help and exit.")
        ("config,c", boost::program_options::value<boost::filesystem::path>()->default_value(defaultConfig),
            ("config file, default " + defaultConfig.string() + ".").c_str())
        ("no-config-file", boost::program_options::bool_switch()->default_value(false),
            "force do not load options from config file, default false.");
    initDesc();

    try
    {
        boost::program_options::command_line_parser parser(argc, argv);
        parser.options(desc).allow_unregistered().style(boost::program_options::command_line_style::unix_style);
        boost::program_options::store(parser.run(), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE(e.what() << CS_LINESEP << desc);
    }
    boost::program_options::notify(options);

    if (options.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }
    else
    {
        bool noConfigFile = options["no-config-file"].as<bool>();
        if (!noConfigFile)
        {
            load(options["config"].as<boost::filesystem::path>());
        }
    }
}

void Config::initDesc()
{
    boost::filesystem::path defaultPidFile("/var/run/" + programName + ".pid");

    desc.add_options()
        ("upstream-tcp-nodelay", boost::program_options::bool_switch()->default_value(true),
            "enables tcp-nodelay feature for upstream socket or not, default on.")

        ("pid-file", boost::program_options::value<boost::filesystem::path>()->default_value(defaultPidFile),
            ("pid file, default " + defaultPidFile.string() + ".").c_str())

        ("downstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to downstream (second), 0 means never timeout, default 30.")

        ("upstream-send-timeout", boost::program_options::value<std::time_t>()->default_value(30),
            "timeout for send to uptream (second), 0 means never timeout, default 30.")

        ("downstream-read-buffer-size", boost::program_options::value<std::size_t>()->default_value(64),
            "read buffer size for downstream (KB), default 64.")

        ("downstream-write-buffer-size", boost::program_options::value<std::size_t>()->default_value(64),
            "write buffer size for downstream (KB), default 64.")

        ("upstream-read-buffer-size", boost::program_options::value<std::size_t>()->default_value(64),
            "read buffer size for upstream (KB), default 64.")

        ("upstream-write-buffer-size", boost::program_options::value<std::size_t>()->default_value(64),
            "write buffer size for upstream (KB), default 64.")

        ("downstream-linger", boost::program_options::bool_switch()->default_value(true),
            "socket linger on/off for downstream, default off.")

        ("downstream-linger-timeout", boost::program_options::value<int>()->default_value(3),
            "socket linger timeout for downstream (second), default 3.")

        ("upstream-linger", boost::program_options::bool_switch()->default_value(false),
            "socket linger on/off for upstream, default off.")

        ("upstream-linger-timeout", boost::program_options::value<int>()->default_value(0),
            "socket linger timeout for upstream (second), default 3.")

        ("downstream-write-pending-interval", boost::program_options::value<int>()->default_value(5),
            "checking interval while downstream write pending (millisecond), default 5.")

        ("upstream-write-pending-interval", boost::program_options::value<int>()->default_value(5),
            "checking interval while upstream write pending (millisecond), default 5.")

        ("username-password-total-max-len", boost::program_options::value<std::size_t>()->default_value(254),
            "max length of username and password, default 254.")

        ("downstream-write-pending-interval", boost::program_options::value<int>()->default_value(10),
            "downstream-write-pending-interval (ms), default 10.")

        ("upstream-write-pending-interval", boost::program_options::value<int>()->default_value(10),
            "upstream-write-pending-interval (ms), default 10.")

        ("iface-name", boost::program_options::value<std::string>()->default_value(std::string("tun0")),
            "iface name, default tun0.")
        ("iface-mtu", boost::program_options::value<uint16_t>()->default_value(1400),
            "iface MTU, default 1400.")
    ;
}

void Config::load(boost::filesystem::path file)
{
    try
    {
        boost::program_options::store(boost::program_options::parse_config_file<char>(file.c_str(), desc), options);
    }
    catch (const std::exception& e)
    {
        CS_DIE("faild on read/parse config-file: " << file << "\n" << e.what());
    }
    boost::program_options::notify(options);

    pidFile = options["pid-file"].as<boost::filesystem::path>();

    usTcpNodelay = options["upstream-tcp-nodelay"].as<bool>();

    dsSendTimeout = toTimeval(options["downstream-send-timeout"].as<std::time_t>());
    usSendTimeout = toTimeval(options["upstream-send-timeout"].as<std::time_t>());

    drBufferSize = options["downstream-read-buffer-size"].as<std::size_t>() << 10;
    dwBufferSize = options["downstream-write-buffer-size"].as<std::size_t>() << 10;
    urBufferSize = options["upstream-read-buffer-size"].as<std::size_t>() << 10;
    uwBufferSize = options["upstream-write-buffer-size"].as<std::size_t>() << 10;

    usLinger = options["upstream-linger"].as<bool>();
    usLingerTimeout = options["upstream-linger-timeout"].as<int>();

    dwPendingInterval = options["downstream-write-pending-interval"].as<int>();
    uwPendingInterval = options["upstream-write-pending-interval"].as<int>();

    userPassTotalLen = options["username-password-total-max-len"].as<std::size_t>();

    ifaceName = options["iface-name"].as<std::string>();
    ifaceMtu = options["iface-mtu"].as<uint16_t>();

    CS_SAY(
        "loaded configs in [" << file.string() << "]:" << std::endl
        _PECAR_OUT_CONFIG_PROPERTY(programName)
        _PECAR_OUT_CONFIG_PROPERTY(usTcpNodelay)
        _PECAR_OUT_CONFIG_PROPERTY(drBufferSize)
        _PECAR_OUT_CONFIG_PROPERTY(dwBufferSize)
        _PECAR_OUT_CONFIG_PROPERTY(urBufferSize)
        _PECAR_OUT_CONFIG_PROPERTY(uwBufferSize)
        _PECAR_OUT_CONFIG_PROPERTY(dwPendingInterval)
        _PECAR_OUT_CONFIG_PROPERTY(uwPendingInterval)
        _PECAR_OUT_CONFIG_PROPERTY(usLinger)
        _PECAR_OUT_CONFIG_PROPERTY(usLingerTimeout)
        _PECAR_OUT_CONFIG_PROPERTY(userPassTotalLen)
        _PECAR_OUT_CONFIG_PROPERTY(ifaceName)
        _PECAR_OUT_CONFIG_PROPERTY(ifaceMtu)
    );
}

Config Config::_instance;

}

#undef _PECAR_OUT_CONFIG_PROPERTY
