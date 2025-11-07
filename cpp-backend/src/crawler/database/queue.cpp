#include "queue.hpp"
#include <iostream>

CrawlerQueue::CrawlerQueue(const std::string& host) {
    try {
        redis = new sw::redis::Redis(host);
        std::cout << "Connected to KeyDB/Redis at " << host << std::endl;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Failed to connect to KeyDB/Redis: " << e.what() << std::endl;
        throw;
    }
}

CrawlerQueue::~CrawlerQueue() {
    delete redis;
    std::cout << "KeyDB/Redis connection closed." << std::endl;
}

std::string CrawlerQueue::serialize_item(const QueueItem& item) {
    return item.url + delim + item.back_link;
}

QueueItem CrawlerQueue::deserialize_item(const std::string& serialized) {
    size_t pos = serialized.find(delim);
    if (pos == std::string::npos) {
        return {serialized, ""};
    }
    return {
        serialized.substr(0, pos),
        serialized.substr(pos + delim.length())
    };
}

bool CrawlerQueue::enqueue(const QueueItem& item) {
    try {
        std::string serialized = serialize_item(item);
        redis->rpush(queue_key, serialized);
        return true;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Enqueue failed: " << e.what() << std::endl;
        return false;
    }
}

bool CrawlerQueue::enqueue_batch(const std::vector<QueueItem>& items) {
    if (items.empty()) return true;
    
    try {
        std::vector<std::string> serialized_items;
        serialized_items.reserve(items.size());
        
        for (const auto& item : items) {
            serialized_items.push_back(serialize_item(item));
        }
        
        redis->rpush(queue_key, serialized_items.begin(), serialized_items.end());
        return true;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Batch enqueue failed: " << e.what() << std::endl;
        return false;
    }
}

bool CrawlerQueue::dequeue(QueueItem& item) {
    try {
        auto result = redis->blpop(queue_key, std::chrono::seconds(1));
        if (result) {
            item = deserialize_item(result->second);
            return true;
        }
        return false;
    } catch (const sw::redis::Error& e) {
        std::cerr << "Dequeue failed: " << e.what() << std::endl;
        return false;
    }
}

size_t CrawlerQueue::size() {
    try {
        return redis->llen(queue_key);
    } catch (const sw::redis::Error& e) {
        std::cerr << "Size check failed: " << e.what() << std::endl;
        return 0;
    }
}

bool CrawlerQueue::is_empty() {
    return size() == 0;
}

void CrawlerQueue::clear() {
    try {
        redis->del(queue_key);
    } catch (const sw::redis::Error& e) {
        std::cerr << "Clear failed: " << e.what() << std::endl;
    }
}