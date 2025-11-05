#include <string>
#include <vector>
#include "gumbo.h"
#include "cpr/cpr.h"
#include <iostream>

static void gather_links(GumboNode* node, std::vector<std::string>& fwd_links){
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    // Check if this element is an <a> tag
    if (node->v.element.tag == GUMBO_TAG_A) {
        // Find the 'href' attribute
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            // Add the attribute's value to our vector
            fwd_links.push_back(href->value);
        }
    }

     // Recursively search all children of this node
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        // Gumbo stores children as void*, so we must cast back to GumboNode*
        gather_links(static_cast<GumboNode*>(children->data[i]), fwd_links);
    }
}

static void extract_text(GumboNode* node, std::stringstream& text_buffer) {
    // If this is a text node, append its content
    if (node->type == GUMBO_NODE_TEXT) {
        text_buffer << node->v.text.text << " ";
        return;
    }

    // If this is an element, check that it's NOT a script or style tag
    if (node->type == GUMBO_NODE_ELEMENT &&
        node->v.element.tag != GUMBO_TAG_SCRIPT &&
        node->v.element.tag != GUMBO_TAG_STYLE) {

        // Recursively search all children
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            extract_text(static_cast<GumboNode*>(children->data[i]), text_buffer);
        }
    }
}

int parse_page(const std::string& url,
                std::string& page,
                std::vector<std::string>& fwd_links){
    cpr::Response resp = cpr::Get(cpr::Url{url});

    if(resp.status_code != 200){
        std::cerr << "HTTP request failed. Code: " << resp.status_code << std::endl;
        return 1;
    }

    GumboOutput* output = gumbo_parse(resp.text.c_str());
    if(!output){
        std::cerr << "Gumbo failed to parse HTML" << std::endl;
        return 2;
    }

    gather_links(output->root, fwd_links);

    std::stringstream ss;
    extract_text(output->root, ss);
    page = ss.str();

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return 0;
}