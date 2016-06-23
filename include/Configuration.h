// This example configuration is obviously too simple to justify this struct
// but is designed to show cross-platform design concepts

#pragma once

#include <string>

struct Configuration {
    std::string accessToken;
    std::string username;
    bool showUsernames;
    bool includePopularPhotos;
    bool includeTaggedPhotos;
    bool onlyIncludeTaggedPhotos;
    bool onlyIncludeLikedPhotos;    
    std::string photoTags;
};

