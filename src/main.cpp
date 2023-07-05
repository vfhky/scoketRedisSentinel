#include "sentinel.h"
#include "utils.h"
#include "demonize.h"



using namespace socketRedisSentinel;



int main(int argc, char * argv[])
{
    int opt = 0;
    LOG_LEVEL sysLogLv = Debug;
    LOG_TYPE sysLogType = LOG_TYPE_STDOUT;

    while ((opt = getopt(argc, argv, "hl:t:")) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-h] [-l] [-t]" << std::endl;
                std::cout << "  -h: display help message" << std::endl;
                std::cout << "  -l: set the system log level , should be in 1-7." << std::endl;
                std::cout << "  -t: set the log type , 1-just print on screen 2-write to file." << std::endl;
                exit(0);
            case 'l':
                sysLogLv = (LOG_LEVEL)Utils::stringToU32(optarg);
                break;
            case 't':
                sysLogType = (LOG_TYPE)Utils::stringToU32(optarg);
                break;
            default:
                std::cout << "not support , use " << argv[0] << " -h to get help info." << std::endl;
                exit(1);
        }
    }

    try {
        Demonize::initSrv(sysLogLv, sysLogType);
    } catch (const std::out_of_range& e) {
        LOG(Error, "out of range exception", sysLogLv, sysLogType, e.what());
        exit(1);
    } catch (const std::invalid_argument& e) {
        LOG(Error, "invalid argument exception", sysLogLv, sysLogType, e.what());
        exit(1);
    } catch (const std::exception& e) {
        LOG(Error, "exception", sysLogLv, sysLogType, e.what());
        exit(1);
    } catch ( ... ) {
        LOG(Error, "uknown exception", sysLogLv, sysLogType);
        exit(1);
    }

    return 0;
}


