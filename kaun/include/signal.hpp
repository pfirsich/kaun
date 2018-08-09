#pragma once

#include <cstdio>
#include <vector>
#include <functional>

// eraz on ##c++@freenode helped me with this, by pointing me to std::function!

// Here we use std::function like template syntax, by defining a generic Signal and
// using template specialization to extract the return and argument types of the single
// function pointer type that's passed
template<class T>
class Signal;

template<class returnType, class... argTypes>
class Signal<returnType (argTypes...)> {
public:
    // We use std::function so we can use both static  functions and member functions, that we bind using
    // lambdas, that we cannot directly cast to a function pointer
    using slotType = std::function<returnType(argTypes...)>;
private:
    std::vector<slotType> mSlots;

public:
    void connect(slotType slot) {
        mSlots.push_back(slot);
    }

    template<class objType>
    void connect(objType* obj, returnType (objType::*slot)(argTypes...)) {
        // [=] i.e. capture by value is necessary
        // if capture by reference is used obj will be captured from the stack of connect
        // this will result in a segfault
        auto f = [=](argTypes... args) -> returnType {return (obj->*slot)(args...);};
        mSlots.push_back(f);
    }

    returnType emit(argTypes... args) {
        for(auto it = mSlots.begin(); it != mSlots.end(); ++it) {
            if(it + 1 == mSlots.end()) {
                return (*it)(args...);
            } else {
                (*it)(args...);
            }
        }
    }
};

/*
int someSlot(float a, int b, float c) {
    printf("slot: %f, %d, %f\n", a, b, c);
    return 5;
}

class ListenClass {
public:
    int listenSlot(float a, int b, float c) {
        printf("listenslot: %f, %d, %f\n", a, b, c);
        return 4;
    }
};

int main(int argc, char** argv) {
    using SomeSignalType = Signal<int(float, int, float)>;
    SomeSignalType someSignal;
    someSignal.connect(someSlot);

    ListenClass listener;
    auto f = [&](float a, int b, float c) -> int {return listener.listenSlot(a, b, c);};
    someSignal.connect(f);
    someSignal.connect(&listener, ListenClass::listenSlot);

    printf("ret: %d", someSignal.emit(1.0f, 2, 2.0f));

    return 0;
}
 */