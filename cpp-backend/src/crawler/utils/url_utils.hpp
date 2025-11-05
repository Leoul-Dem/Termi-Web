#pragma once

#include <boost/url.hpp>
#include <string>
#include "cpr/cpr.h"

inline std::string normalize_url(const std::string& url) {
    boost::urls::url url_ = boost::urls::parse_uri_reference(url).value();

    // Apply various normalization steps
    url_.normalize();

    if(url_.has_fragment()){
        url_.remove_fragment();
    }

    return url_.buffer();
}

inline int get_url_status_code(const std::string& url){
    return cpr::Head(cpr::Url{url}).status_code;
}

inline bool is_status_good(int status_code){
    if(status_code == 200){
        return true;
    }
    return false;
}