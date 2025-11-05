#include "crawler.hpp"
#include "database.hpp"
#include "page_parser.hpp"
#include "url_utils.hpp"
#include "misc.hpp"
#include <vector>
#include <string>

Crawler::Crawler(std::string db_path, std::string seed_url) :
  db(db_path),
  seed_url(seed_url)
  {}

void Crawler::enqueue_links(){

}

bool Crawler::crawl_again(const long long& recorded_time){
  return (get_curr_time() - recorded_time) > PAGE_LIFETIME;
}

void Crawler::crawl_seen_page(Curr_URL curr_url, Site site_){
  // validate if the page is good
      site_.status_code = get_url_status_code(curr_url.url);
      if(!is_status_good(site_.status_code)){
        return;
      }

      // parse the page html
      std::string temp_html;
      // and get all the forward links
      std::vector<std::string> fwd_links;
      parse_page(curr_url.url, temp_html, fwd_links);

      // check if the page hash changed
      if(temp_html != site_.HTML){
        site_.HTML = temp_html;
        site_.last_crawled = get_curr_time();

        std::vector<std::string> temp_fwd_links;
        db.get_forward_links(curr_url.url, temp_fwd_links);

        // check if the forward links have changed
        if(fwd_links != temp_fwd_links){
          db.add_forward_links(curr_url.url, temp_fwd_links);
        }

        // TODO
        // REDIS ENQUEUE temp_fwd_links
      }
}

void Crawler::crawl_unseen_page(Curr_URL curr_url, Site site_){
  site_.status_code = get_url_status_code(curr_url.url);
  if(!is_status_good(site_.status_code)){
    return;
  }

  std::string temp_html;
  std::vector<std::string> fwd_links;
  parse_page(curr_url.url, site_.HTML, fwd_links);

  site_.last_crawled = get_curr_time();
  db.add_url(curr_url.url, site_);
  db.add_forward_links(curr_url.url, fwd_links);
  // TODO
  // REDIS ENQUEUE fwd_links, curr_url.url
}

void Crawler::crawl(){

  Curr_URL curr_url;
  Site site_;

  for(int i = 0; i < CRAWL_LIMIT; i++){
    // TODO
    curr_url = {}; // REDIS DEQUEUE
    site_ = {};

    // check if the page has been crawled
    if(!db.get_site(curr_url.url, site_) &&
        crawl_again(site_.last_crawled)){

      crawl_seen_page(curr_url, site_);

    }else{
      crawl_unseen_page(curr_url, site_);
    }

    db.append_back_link(curr_url.url, curr_url.b_link);
  }
}

Crawler::~Crawler(){}


