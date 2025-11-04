#pragma once

#include "./database/database.hpp"
#include <string>

using namespace std;

class Crawler{
private:
  URLDatabase db;
  const int CRAWL_LIMIT = 100000;
  const long long PAGE_LIFETIME = 1209600000;
  const std::string seed_url;

  struct Curr_URL{
    std::string url;
    std::string b_link;
  };

  bool crawl_again(const long long& recorded_time);
  void crawl_seen_page(Curr_URL curr_url, Site site_);
  void crawl_unseen_page(Curr_URL curr_url, Site site_);
  void enqueue_links();


public:

  Crawler(std::string db_path, std::string seed_url);

  void crawl();

  ~Crawler();


};