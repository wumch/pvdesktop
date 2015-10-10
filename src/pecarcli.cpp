
#include "Config.hpp"
#include "Channel.hpp"

int main(int argc, char* argv[])
{
    pecar::Config::mutableInstance()->init(argc, argv);
    pecar::Channel().start();
}
