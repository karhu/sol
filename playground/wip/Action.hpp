#pragma once

#include <stdint.h>
#include "vec2.hpp"
#include <vector>
#include <deque>
#include <mutex>
#include <memory>
#include <tuple>
#include <cstring>
#include <limits>

#include <iostream>

#include "delegate.hpp"

#include "miro/action/ActionType.hpp"

namespace miro {

    namespace action {

        template<typename T, typename S>
        T checked_numeric_cast(S source) {
            using namespace std;
            T target = source;
            if (numeric_limits<T>::max() < numeric_limits<S>::max()) {
                if (S(numeric_limits<T>::max()) < source) {
                    // TODO log error
                    std::cerr << "numeric downcast of too large a value" << std::endl;
                    target = numeric_limits<T>::max();
                }
            }
            if (numeric_limits<T>::min() > numeric_limits<S>::min()) {
                if (S(numeric_limits<T>::min()) > source) {
                    // TODO log error
                    std::cerr << "numeric downcast of too small a value" << std::endl;
                    target = numeric_limits<T>::min();
                }
            }
            return target;
        }




    }

    enum class ActionType : uint8_t{
        Unknown = 0,
        StrokeBegin,
        StrokeUpdate,
        StrokeEnd,
        Viewport,
        User,
    };

    struct StrokeAction {
        vec2f position; // position in the current view frame
        float pressure; // pressure in [0,1]
        uint8_t button; // a button, finger, tip id
    };

    struct ViewportAction {
        Transform2f transform;
    };

    struct UserAction {
        enum class Type : uint8_t {
            Unknown = 0,
            Add = 1,
            Remove = 2,
            Change = 3,
        };

        uint16_t user_id;
        Type type;
        bool is_local_user = false;
        //std::string name;
    };

    struct Action {
        ActionType type = ActionType::Unknown;
        uint16_t timestamp; // timestamp in 10ms steps, rolls over every 10min
        uint16_t user = 0;
        union Data {
            Data() {}
            StrokeAction stroke;
            ViewportAction viewport;
            UserAction user;
        }data;
    };

    class IActionSource;
    class IActionSink;
















}


