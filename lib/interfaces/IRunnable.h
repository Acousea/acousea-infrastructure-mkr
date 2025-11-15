#ifndef IOPERATIONMODE_H
#define IOPERATIONMODE_H


class IRunnable
{
protected:
    ~IRunnable() = default;

public:
    // Constructor that receives a reference to the display
    virtual void init() = 0;
    virtual void run() = 0;
};

#endif // IOPERATIONMODE_H
