#pragma once
#include <stdint.h>
#include <limits>

namespace fth
{
    template<typename T, typename S = uint32_t>
    class Handle
    {
    private:
        using HandleType = T;
        using InternalType = S;
    public:
        Handle() : id(std::numeric_limits<InternalType>::max()) {};
        Handle(InternalType id) : id(id) {};
        Handle(const Handle& other) : id(other.id) {};

        bool operator!=(const Handle& rhs) { return id != rhs.id; };
        bool operator!=(const Handle& rhs) const { return id != rhs.id; };
        bool operator==(const Handle& rhs) { return id == rhs.id; }
        bool operator==(const Handle& rhs) const { return id == rhs.id; }
        bool operator==(std::size_t rhs) const { return id == rhs; }
        bool isValid() const { return id != std::numeric_limits<InternalType>::max(); }
        void invalidate() { id = std::numeric_limits<InternalType>::max(); }

        Handle<HandleType, InternalType>& operator++() { ++id, return *this; }//prefix
        Handle<HandleType, InternalType>  operator++(int) 
        {
            Handle<HandleType, InternalType> temp = *this;
            ++id;
            return temp;
        }
        //operator std::size_t() const noexcept { return static_cast<std::size_t>(id); }
        operator S() const noexcept { return static_cast<S>(id); }


        static constexpr Handle Invalid() { return { std::numeric_limits<InternalType>::max() }; }
        InternalType id;
    };
}


