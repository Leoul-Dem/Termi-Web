#include <boost/url.hpp>
#include <iostream>

std::string normalize_with_boost(const std::string& url) {
    boost::urls::url url_ = boost::urls::parse_uri_reference(url).value();

    // Apply various normalization steps
    url_.normalize();

    if(url_.has_fragment()){
        url_.remove_fragment();
    }

    return url_.buffer();
}