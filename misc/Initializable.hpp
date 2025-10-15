#pragma once

#include "Logger.hpp"
#include "IniData.hpp"
#include "IniFile.hpp"

class Initializable {
public:
    Initializable() {
        inifile.onLoad = [this]() { 
            onLoad(); 
        };
    }
    virtual ~Initializable() {}
    
    virtual void init(const string& inifname, bool createIfNotExists, bool throwsIfNotExists) {
        inifile.load(inifname, createIfNotExists, throwsIfNotExists);
    }

    virtual void reset(const IniData* inidata = nullptr) {
        if (inidata) inifile.setData(*inidata);
        onReload();
    }    

    const IniFile& getIniFileCRef() const { return inifile; }
    IniFile& getIniFileRef() { return inifile; }

    template<typename T>
    T get(const string& key, const string& section = "") const {
        try {
            return inifile.get<T>(key, section);
        } catch (exception &e) {
            throw ERROR("Unable to get init value at key " + EMPTY_OR(key) + " at section [" + EMPTY_OR(section, "<global>") + "]" + EWHAT);
        }
    }

    virtual void onLoad() = 0;

    virtual void onReload() = 0;

protected:
    IniFile inifile;
};
