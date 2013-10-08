#include <ctime>

#include "xlog.h"

using namespace std;

xlog_impl xlog;

xlog_impl& xlog_impl::operator<<(std::ostream& ( *pf )(std::ostream&))
{
    (get() << pf).flush(); // flush aggressively in case of crash
#ifndef NDEBUG
    cout << pf;
#endif
    return *this;
}

struct xlog_helper
{
    fstream f;
    xlog_helper() 
        : f("log.txt", fstream::out) // open the log file
    {
        timestamp(); // timestamp the log
    }
    void timestamp() {
        time_t rawtime;
        time(&rawtime);
        char buffer[256];
        ctime_s(buffer, 256, &rawtime);
        f << buffer;
#ifndef NDEBUG
        cout << buffer;
#endif
    }
};

fstream& xlog_impl::get()
{
    static xlog_helper h; // open and timestamp logfile on first use
    return h.f;
}
