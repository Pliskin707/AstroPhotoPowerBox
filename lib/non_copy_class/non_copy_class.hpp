#ifndef __NON_COPYABLE_HPP__
#define __NON_COPYABLE_HPP__

class nonCopyable {
    public:
        nonCopyable (const nonCopyable &) = delete;
        nonCopyable & operator = (const nonCopyable &) = delete;

    protected:
        nonCopyable() = default;   // this template cannot be instantiated 
        ~nonCopyable() = default;  // ... and not be destructed
};

#endif