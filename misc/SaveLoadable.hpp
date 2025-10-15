#pragma once

#include "Saveable.hpp"
#include "Loadable.hpp"

class SaveLoadable: public Saveable, public Loadable {
public:
    SaveLoadable(): Saveable(), Loadable() {}
    virtual ~SaveLoadable() {}
};