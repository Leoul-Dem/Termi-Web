#include "crawler.hpp"
#include <thread>
#include <vector>


using namespace std;
int main(){
    Crawler crawler("", "");

    std::vector<std::thread> workers;

    for(int i = 0; i < 8; i++){
        workers.emplace_back(&Crawler::crawl, &crawler);
        workers[i].detach();
    }

    return 0;
}