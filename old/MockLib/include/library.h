#ifndef MOCKLIB_LIBRARY_H
#define MOCKLIB_LIBRARY_H

class MockLibrary {
public:
    virtual ~MockLibrary() = default;

    virtual int get_value() = 0;

    static void print_with_callback(void (*callback)(const char *));
};


#endif //MOCKLIB_LIBRARY_H
