#ifndef ACOUSEA_INFRASTRUCTURE_MKR_SDPATH_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_SDPATH_HPP

#define SD_PATH(p) \
([]() constexpr { \
static_assert((p)[0] == '/', "SD path must start with '/'"); \
static_assert(sizeof(p) <= 12, "SD filename exceeds FAT 8.3 limit"); \
return p; \
}())

#endif
