#pragma once

#include <string>
#include <vector>
#include <sw/redis++/redis++.h>

struct QueueItem {
    std::string url;
    std::string back_link;
};

class CrawlerQueue {
private:
    sw::redis::Redis* redis;
    const std::string queue_key = "crawler:queue";
    const std::string delim = "::DELIM::";

    std::string serialize_item(const QueueItem& item);
    QueueItem deserialize_item(const std::string& serialized);

public:
    CrawlerQueue(const std::string& host = "tcp://127.0.0.1:6379");
    ~CrawlerQueue();

    // Queue operations
    bool enqueue(const QueueItem& item);
    bool enqueue_batch(const std::vector<QueueItem>& items);
    bool dequeue(QueueItem& item);
    
    // Utility operations
    size_t size();
    bool is_empty();
    void clear();
};