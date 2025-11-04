#ifndef URLDATABASE_H
#define URLDATABASE_H

#include "rocksdb/db.h"
#include <string>
#include <chrono>
#include <vector>
#include <iostream>

struct Site{
    std::string HTML;
    int status_code;
    long long last_crawled;
};

class URLDatabase {
private:
    rocksdb::DB* db;
    std::string db_path;

    // Handles for each "table" or Column Family
    rocksdb::ColumnFamilyHandle* site_cf;
    rocksdb::ColumnFamilyHandle* fwd_links_cf;
    rocksdb::ColumnFamilyHandle* back_links_cf;

    const std::string site_delim = "::SECTION::";
    const std::string link_delim = "::LINK::";

    std::string serialize_links(const std::vector<std::string>& links);
    std::vector<std::string> deserialize_links(const std::string& serialized);

    std::string serialize_site(const Site& site);
    Site deserialize_site(const std::string& site);

public:
    URLDatabase(const std::string& path);

    ~URLDatabase();

    // All the individual setters
    bool add_url(const std::string& url, const Site& site);
    bool append_forward_link(const std::string& url, const std::string& link);
    bool append_back_link(const std::string& url, const std::string& link);

    // All the large setters
    bool add_forward_links(const std::string& url, const std::vector<std::string>& links);
    bool add_back_links(const std::string& url, const std::vector<std::string>& links);

    // All the getters
    int get_site(const std::string& url, Site& site);
    int get_forward_links(const std::string& url, std::vector<std::string>& links);
    int get_back_links(const std::string& url, std::vector<std::string>& links);
};
#endif