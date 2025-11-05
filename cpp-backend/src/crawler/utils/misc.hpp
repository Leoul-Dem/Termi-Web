#include <chrono>
#include "cpr/cpr.h"

long long get_curr_time(){
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();

    return duration_cast<milliseconds>(duration).count();
}

