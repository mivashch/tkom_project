#pragma once
#include <string>
#include <istream>
#include <memory>


namespace minilang {
    class Source {
    public:
        virtual ~Source() = default;

        virtual int get() = 0;

        virtual int peek() = 0;

        virtual size_t line() const = 0;

        virtual size_t column() const = 0;

        virtual void unget() = 0;
    };


    std::unique_ptr<Source> makeFileSource(const std::string &path);

    std::unique_ptr<Source> makeStringSource(const std::string &s);
}
