#ifndef ACOUSEA_CLASSNAME_H
#define ACOUSEA_CLASSNAME_H

// ---------------------------------------------------------------------------
// Fallback global function: returns a default class name when not overridden (class does not use CLASS_NAME macro)
// ---------------------------------------------------------------------------
inline constexpr const char* getClassNameCString(bool withSuffix = true)
{
    return withSuffix ? "[UNKNOWN CLASS]: " : "[UNKNOWN CLASS]";
}

// ---------------------------------------------------------------------------
// Macro to declare the static method in each class that uses it
// ---------------------------------------------------------------------------
#define CLASS_NAME(name) \
static constexpr const char* getClassNameCString(bool withSuffix = true) { \
return withSuffix ? "[" #name "]: " : "[" #name "]"; \
}

/** -------------------------- ONLY FOR C++20 AND ABOVE --------------------------
 * #define CLASS_NAME_CSTR_SAFE() \
 * ([]() constexpr -> const char* { \
 * if constexpr (requires { getClassNameCString(true); }) \
 * return getClassNameCString(true); \
 * else \
 * return "[UNKNOWN CLASS]: "; \
 * }())

-------------------------- **/

#endif // ACOUSEA_CLASSNAME_H
