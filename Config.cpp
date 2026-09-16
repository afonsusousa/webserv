#include "Config.hpp"

Config::~Config() {
    for (size_t i = 0; i < all_blocks.size(); ++i) {
        delete all_blocks[i];
    }
}
