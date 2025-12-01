#include "lexer.h"
#include "source.h"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <memory>
#include <string>


namespace minilang {
    class FileSource : public Source {
    public:
        explicit FileSource(const std::string &path) : in_(path), line_(1), col_(1), lastChar_(0) {
            if (!in_) throw std::runtime_error("Cannot open file: " + path);
        }

        int get() override {
            int c = in_.get();
            if (c == EOF) {
                lastChar_ = EOF;
                return -1;
            }
            if (c == '\n') {
                ++line_;
                col_ = 1;
            } else ++col_;
            lastChar_ = c;
            return c;
        }

        int peek() override {
            int c = in_.peek();
            return c == EOF ? -1 : c;
        }

        size_t line() const override { return line_; }
        size_t column() const override { return col_; }

        void unget() override {
            if (lastChar_ != 0 && lastChar_ != EOF) {
                in_.unget();
                if (lastChar_ == '\n') { --line_; } else --col_;
            }
        }

    private:
        std::ifstream in_;
        size_t line_, col_;
        int lastChar_;
    };


    class StringSource : public Source {
    public:
        explicit StringSource(const std::string &s) : s_(s), idx_(0), line_(1), col_(1) {
        }

        int get() override {
            if (idx_ >= s_.size()) return -1;
            unsigned char c = s_[idx_++];
            if (c == '\n') {
                ++line_;
                col_ = 1;
            } else ++col_;
            lastChar_ = c;
            return c;
        }

        int peek() override {
            if (idx_ >= s_.size()) return -1;
            return static_cast<unsigned char>(s_[idx_]);
        }

        size_t line() const override { return line_; }
        size_t column() const override { return col_; }

        void unget() override {
            if (idx_ > 0) {
                --idx_;
                if (lastChar_ == '\n') {
                    --line_;
                } else --col_;
            }
        }

    private:
        std::string s_;
        size_t idx_;
        size_t line_, col_;
        int lastChar_;
    };

    class StreamSource : public Source {
    public:
        explicit StreamSource(std::istream &in)
            : in_(in), line_(1), col_(1), lastChar_(0) {
        }

        int get() override {
            int c = in_.get();
            if (c == EOF) {
                lastChar_ = EOF;
                return -1;
            }
            if (c == '\n') {
                ++line_;
                col_ = 1;
            } else ++col_;
            lastChar_ = c;
            return c;
        }

        int peek() override {
            int c = in_.peek();
            return c == EOF ? -1 : c;
        }

        size_t line() const override {
            return line_;
        }

        size_t column() const override {
            return col_;
        }

        void unget() override {
            if (lastChar_ != 0 && lastChar_ != EOF) {
                in_.unget();
                if (lastChar_ == '\n') {
                    --line_;
                } else {
                    --col_;
                }
            }
        }

    private:
        std::istream &in_;
        size_t line_, col_;
        int lastChar_;
    };


    std::unique_ptr<Source> makeFileSource(const std::string &path) { return std::make_unique<FileSource>(path); }
    std::unique_ptr<Source> makeStringSource(const std::string &s) { return std::make_unique<StringSource>(s); }
}
