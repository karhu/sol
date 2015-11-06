#pragma once

#include <vector>

namespace sol {

template<typename T>
bool bag_remove_first(std::vector<T>& vector, const T& value)
{
    auto iter = std::find(vector.begin(), vector.end(), value);
    if (iter == vector.end()) return false;

    std::swap(*iter,vector.back());
    vector.pop_back();
    return true;
}

}
