#include "database.hpp"
#include <iostream>
#include <sstream>


// Private Methods

URLDatabase::URLDatabase(const std::string& path){
  rocksdb::Options options;
  options.create_if_missing = true;
  options.create_missing_column_families = true;
  // Define the column families we want to use
  std::vector<std::string> cf_names = {
    ROCKSDB_DEFAULT_COLUMN_FAMILY_NAME, // "default"
    "site",
    "fwd_links",
    "back_links"
  };

  std::vector<rocksdb::ColumnFamilyDescriptor> column_families;
  for (const auto& name : cf_names) {
    column_families.push_back(
        rocksdb::ColumnFamilyDescriptor(name, rocksdb::ColumnFamilyOptions())
    );
  }

  std::vector<rocksdb::ColumnFamilyHandle*> handles;
  rocksdb::Status status = rocksdb::DB::Open(options, db_path, column_families, &handles, &db);

  if (!status.ok()) {
    std::cerr << "Failed to open RocksDB: " << status.ToString() << std::endl;
    throw std::runtime_error("Could not open RocksDB");
  }

  assert(handles.size() == 4);
  // We ignore handles[0] (default) for this design
  site_cf      = handles[1];
  fwd_links_cf = handles[2];
  back_links_cf= handles[3];

  std::cout << "Database opened successfully with 3 Column Families." << std::endl;
}

URLDatabase::~URLDatabase() {
  // Destroy handles first
  db->DestroyColumnFamilyHandle(site_cf);
  db->DestroyColumnFamilyHandle(fwd_links_cf);
  db->DestroyColumnFamilyHandle(back_links_cf);
  // Then close the DB
  delete db;
  std::cout << "Database closed." << std::endl;
}

std::string URLDatabase::serialize_links(const std::vector<std::string>& links){
  if(links.empty()){
    return "";
  }
  std::stringstream ss;
  for(auto i : links){
    ss << i << link_delim;
  }
  return ss.str();
}

std::vector<std::string> URLDatabase::deserialize_links(const std::string& serialized){
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = serialized.find(link_delim);

  size_t delim_size = link_delim.length();

  while(end != std::string::npos){
    tokens.push_back(serialized.substr(start, end-start));

    start = end + delim_size;

    end = serialized.find(link_delim, start);
  }

  std::string last_token = serialized.substr(start);

  if(!last_token.empty()){
    tokens.push_back(last_token);
  }

  return tokens;
}

std::string URLDatabase::serialize_site(const Site& site){
  return std::to_string(site.status_code) + site_delim +
         std::to_string(site.last_crawled) + site_delim +
         site.HTML + site_delim;
}

Site URLDatabase::deserialize_site(const std::string& site){
  std::string HTML;
  std::string status_code;
  std::string last_crawled;

  std::stringstream ss(site);

  std::getline(ss, status_code, site_delimiter);
  std::getline(ss, last_crawled, site_delimiter);
  std::getline(ss, HTML, site_delimiter);

  return {HTML, std::stoi(status_code), std::stoll(last_crawled)};
}

// Public Methods

// All the individual setters
bool add_url(const std::string& url, const Site& site){
  std::string site_ = serialize_site(site);
  rocksdb::Status s = db->Put(rocksdb::WriteOptions(), site_cf, url, site_);
  return s.ok();
}

bool append_forward_link(const std::string& url, const std::string& link){
  std::vector<std::string> links_;
  get_forward_links(url, links_);
  links_.push_back(link);
  return add_forward_links(url, links_);
}

bool append_back_link(const std::string& url, const std::string& link){
  std::vector<std::string> links_;
  get_back_links(url, links_);
  links_.push_back(link);
  return add_back_links(url, links_);
}

// All the large setters
bool add_forward_links(const std::string& url, const std::vector<std::string>& links){
  std::string links_ = serialize_links(links);
  rocksdb::Status s = db->Put(rocksdb::WriteOptions(), fwd_links_cf, links_);
  return s.ok();
}

bool add_back_links(const std::string& url, const std::vector<std::string>& links){
  std::string links_ = serialize_links(links);
  rocksdb::Status s = db->Put(rocksdb::WriteOptions(), back_links_cf, links_);
  return s.ok();
}

// All the getters
int get_site(const std::string& url, Site& site){
  std::string site_;
  rocksdb::Status s = db->Get(rocksdb::ReadOptions(), site_cf, url, &site_);

  if (s.IsNotFound()) return 1;
  if (!s.ok()) return -1;

  site = deserialize_site(site_);
  return 0;
}

int get_forward_links(const std::string& url, std::vector<std::string>& links){
  std::string links_;
  rocksdb::Status s = db->Get(rocksdb::ReadOptions(), fwd_links_cf, url, &links_);

  if (s.IsNotFound()) return 1;
  if (!s.ok()) return -1;

  site = deserialize_site(links_);
  return 0;
}

int get_back_links(const std::string& url, std::vector<std::string>& links){
  std::string links_;
  rocksdb::Status s = db->Get(rocksdb::ReadOptions(), back_links_cf, url, &links_);

  if (s.IsNotFound()) return 1;
  if (!s.ok()) return -1;

  site = deserialize_site(links_);
  return 0;
}
