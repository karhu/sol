#pragma once

#include <util.hpp>
#include <FastDelegate.hpp>

namespace sol
{
    // We are using the FastDelegate library by Don Clugston.
    template<typename FuncSig>
    using delegate = fastdelegate::FastDelegate< FuncSig >;


    template <typename... Args>
    auto make_delegate(Args&&... args) -> decltype(fastdelegate::MakeDelegate(std::forward<Args>(args)...)) {
      return fastdelegate::MakeDelegate(std::forward<Args>(args)...);
    }

}
