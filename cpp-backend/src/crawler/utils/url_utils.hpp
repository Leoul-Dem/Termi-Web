#pragma once

#include <boost/url.hpp>
#include <string>

inline std::string normalize_url(const std::string& url) {
    boost::urls::url url_ = boost::urls::parse_uri_reference(url).value();

    // Apply various normalization steps
    url_.normalize();

    if(url_.has_fragment()){
        url_.remove_fragment();
    }

    return url_.buffer();
}
// TODO
inline int validate_url(const std::string& url){
    // makes a HEAD cUrl request to the url
    return 0;
}