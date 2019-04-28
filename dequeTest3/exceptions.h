#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace sjtu {

    class exception {
    protected:
        const std::string variant = "";
        std::string detail = "";
    public:
        exception() {}
        exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
        virtual std::string what() {
            return variant + " " + detail;
        }
    };

/**
 * TODO
 * Please complete them.
 */
    class index_out_of_bound : public exception {
    public:
        index_out_of_bound():exception(){
            detail="the index is out of bound";
        }
    };

    class runtime_error : public exception {
        /* __________________________ */
    };

    class invalid_iterator : public exception {
    public:
        invalid_iterator():exception(){
            detail="the iterator is invalid";
        }
    };

    class container_is_empty : public sjtu::exception {
    public:
        container_is_empty():exception(){
            detail="the container is empty";
        }
    };
}

#endif
