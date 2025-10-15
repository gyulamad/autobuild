#pragma once

#include <stdint.h>
#include "Value.hpp"
#include "Loadable.hpp"
#include "Saveable.hpp"
#include "file_get_contents.hpp"
#include "file_put_contents.hpp"
#include "vector_load.hpp"
#include "vector_save.hpp"
#include "vector_concat.hpp"
#include "safe.hpp"
#include "IniData.hpp"
#include "Initializable.hpp"
#include "VECTOR_WRAPPER.hpp"

template<typename real = float> // , typename S = uint32_t>
class ValuesT: public Initializable {
public:
    ValuesT(): Initializable() {}

    virtual ~ValuesT() {}

    VECTOR_WRAPPER(ValueT<real>, values)

    void onLoad() override {
        LOG_DEBUG("ValuesT are loaded, converts..");
        convert();
    }

    void onReload() override {
        LOG_DEBUG("ValuesT are reloaded, converts...");
        convert();
    }

    // // TODO: depricate or fix, name is ini file section not a key 'name=...'
    // template<typename S>
    // void load(const string& filename, Filemode mode) {
    //     vector<string> valueStrs;
    //     vector<S> bin;
    //     ValueT<real> value;
    //     size_t nxt = 0;

    //     switch (mode) {
    //         case TEXT:
    //             valueStrs = explode("\n\n", file_get_contents(filename));
    //             values.clear();
    //             for (const string& valueStr: valueStrs) {
    //                 ValueT<real> value;
    //                 value.fromString(valueStr);
    //                 values.push_back(value);
    //             }
    //             break;

    //         case BINARY:
    //             vector_load(bin, filename);
    //             values.clear();
                
    //             while (value.deserialize(bin, nxt))
    //                 values.push_back(value);
    //             break;

    //         default:
    //             throw ERROR("Invalid file mode");
    //     }
    // }

    // // TODO: depricate or fix, name is ini file section not a key 'name=...'
    // template<typename S>
    // void save(const string& filename, Filemode mode) {
    //     vector<string> valueStrs;
    //     vector<S> bin;

    //     switch (mode) {
    //         case TEXT:
    //             for (const ValueT<real>& value: values)
    //                 valueStrs.push_back(value.toString());
    //             file_put_contents(filename, implode("\n\n", valueStrs), false, true);
    //             break;

    //         case BINARY:
    //             for (const ValueT<real>& value: values)
    //                 bin = vector_concat<S>(bin, value.serialize());
    //             vector_save(bin, filename);
    //             break;

    //         default:
    //             throw ERROR("Invalid file mode");
    //     }
    // }


    // void add(const ValueT<real>& value) {
    //     values.push_back(value);
    // }

    // -------------

    template<typename T>
    T add(const string& name, T value) {
        if (has(name))
            throw ERROR("Already has: " + EMPTY_OR(name));
        Value v(name, (real)value);
        values.push_back(v);
        return v.getValue();
    }

    bool has(const string& name) {
        size_t size = values.size();
        for (size_t i = 0; i < size; i++)
            if (values[i].getNameCRef() == name) return true;
        return false;
    }

    template<typename T>
    T set(const string& name, T value) {
        size_t size = values.size();
        for (size_t i = 0; i < size; i++)
            if (values[i].getNameCRef() == name) {
                return values[i] = value;
            }
        throw ERROR("Not found: " + EMPTY_OR(name));
    }

    template<typename T>
    T get(const string& name) {
        size_t size = values.size();
        for (size_t i = 0; i < size; i++)
            if (values[i].getNameCRef() == name) return values[i];
        throw ERROR("Not found: " + EMPTY_OR(name));
    }
    
    // ValueT<real>& operator[](size_t index) {
    //     return values[index];
    // }
    
    // const ValueT<real>& operator[](size_t index) const {
    //     return values[index];
    // }
    
    ValueT<real>& operator[](const string& name) {
        size_t i = getIndexByName(name);
        return values[i];
    }
    
    const ValueT<real>& operator[](const string& name) const {
        size_t i = getIndexByName(name);
        return values[i];
    }
    
    ValueT<real>& getValueByName(const string& name) {
        for (ValueT<real>& value: values)
            if (value.getNameCRef() == name) return value;
        throw ERROR("Value not found");
    }
    
    size_t getIndexByName(const string& name, bool create = true) {
        size_t size = values.size();
        for (size_t i = 0; i < size; i++)
            if (values[i].getNameCRef() == name) return i;
        if (create) {
            values.push_back(ValueT<real>(name, .0f));
            return values.size() - 1;
        }
        throw ERROR("Value not found");
    }
    
    vector<real> getVariables() const {
        vector<real> reals;
        for (const ValueT<real>& value: values)
            if (!value.isConstant()) reals.push_back(value);
        return reals;
    }
    
    void setVariables(const vector<real>& variables) {
        size_t activeIndex = 0;
        size_t variablesSize = variables.size();
        for (ValueT<real>& value: values)
            if (!value.isConstant() && activeIndex < variablesSize)
                value = variables[activeIndex++];
        if (variablesSize != activeIndex)
            throw ERROR("Values size mismatch, " + to_string(variablesSize) + " != " + to_string(activeIndex));
    }
    
    vector<real> getLowerBounds() const {
        vector<real> bounds;
        for (const ValueT<real>& value: values)
            if (!value.isConstant()) bounds.push_back(value.getLower());
        return bounds;
    }
    
    vector<real> getUpperBounds() const {
        vector<real> bounds;
        for (const ValueT<real>& value: values)
            if (!value.isConstant()) bounds.push_back(value.getUpper());
        return bounds;
    }

    IniData getAsIniData() const {
        IniData iniData;
        for (const ValueT<real>& value: values) {
            string section = value.getNameCRef();
            unordered_map<string, string> map = value.getAsUMapStr();
            for (const auto& [key, val]: map)
                iniData.set(key, val, section);
        }
        return iniData;
    }
    
    // size_t size() const { return values.size(); }

    // void clear() { values.clear(); }
    
    string dump(bool show = false) const {
        vector<string> dumps;
        for (const ValueT<real>& value: values)
            dumps.push_back(value.dump());
        string output = implode("\n", dumps);
        if (show) cout << output << endl;
        return output;
    }

private:
    vector<ValueT<real>> values;

    void convert() {
        static const string sectionKey = "name";
        static const string ksep = "=";
        static const string vsep = "\n";

        values.clear();
        for (const string& section: inifile.sections()) {
            if (section.empty())
                // ... (globals in the ini file are not a part of a ValueT)
                continue;

            string valueStr = sectionKey + ksep + section;
            for (const auto& [key, value]: inifile.list<string>(section))
                valueStr += vsep + key + ksep + value;
            
            ValueT<real> value;
            value.fromString(valueStr, ksep, vsep);
            values.push_back(value);
        }
    }
};

// template<typename... Ts>
// ValuesT(...) -> ValuesT<float, uint32_t>;

using Values = ValuesT<float>; //, uint32_t>;

// using real = float;

#ifdef TEST

#include "unlink.hpp"

// Test struct for Values class
struct test_Values_TestData {
    string filename;
    vector<Value> values;
};

// Test for adding a value and retrieving it by index
TEST(test_Values_add_and_operator_index) {
    Values values;
    values["test"] = 42;

    assert(values["test"] == 42.0f && "Value should be 42.0 at name 'test'");
}

// Test for retrieving a value by name
TEST(test_Values_getValueByName_valid_name) {
    Values values;
    values["test_name"] = 100;

    Value& retrieved = values.getValueByName("test_name");
    assert(retrieved == 100.0f && "Value should match at the name");
}

// Test for getByName throwing exception for invalid name
TEST(test_Values_getValueByName_invalid_name_expected_to_throw) {
    Values values;
    bool threw = false;
    try {
        values.getValueByName("non_existent");
    } catch (const exception& e) {
        assert(str_contains(e.what(), "Value not found") && "Exception should contain 'Value not found'");
        threw = true;
    }
    assert(threw && "getByName should throw for invalid name");
}

// // Test for saving and loading in TEXT mode
// TEST(test_Values_save_and_load_text_mode) {
//     test_Values_TestData data;
//     data.filename = "test_values.txt";
//     unlink(data.filename);

//     Values values;
//     values["test"] = 123.45f;
//     values.save<float>(data.filename, Filemode::TEXT);

//     Values loaded;
//     loaded.load<float>(data.filename, Filemode::TEXT);

//     assert(loaded.size() == 1 && "Loaded values should have size 1");
//     assert(loaded["test"] == 123.45f  && "Loaded value name should match");
//     unlink(data.filename);
// }

// // Test for saving and loading in BINARY mode
// TEST(test_Values_save_and_load_binary_mode) {
//     test_Values_TestData data;
//     data.filename = "test_values.bin";
//     unlink(data.filename);

//     Values values;
//     values["binary_test"] = 678.9f;
//     values.save<float>(data.filename, Filemode::BINARY);

//     Values loaded;
//     loaded.load<float>(data.filename, Filemode::BINARY);

//     assert(loaded.size() == 1 && "Loaded values should have size 1");
//     assert(loaded["binary_test"] == 678.9f && "Loaded value name should match");
//     unlink(data.filename);
// }

// Test for getVariables with non-constant values
TEST(test_Values_getVariables_non_constant) {
    Values values;
    values["var1"] = 10;
    values["var1"].setConstant(false);
    values["var2"] = 20;
    values["var2"].setConstant(true);

    vector<float> variables = values.getVariables();
    assert(variables.size() == 1 && "Should return only non-constant values");
    assert(variables[0] == 10.0f && "Non-constant value should be 10.0");
}

// Test for setVariables with matching size
TEST(test_Values_setVariables_matching_size) {
    Values values;
    values["var1"] = 0.0f;
    values["var1"].setConstant(false);
    values["var2"] = 0.0f;
    values["var2"].setConstant(false);

    vector<float> new_values = {55.5f, 66.6f};
    values.setVariables(new_values);

    assert(values["var1"] == 55.5f && "First value should be updated to 55.5");
    assert(values["var2"] == 66.6f && "Second value should be updated to 66.6");
}

// Test for setVariables throwing exception for size mismatch
TEST(test_Values_setVariables_size_mismatch_expected_to_throw) {
    Values values;
    values["var1"] = 0;

    vector<float> new_values = {1.0f, 2.0f};
    bool threw = false;
    try {
        values.setVariables(new_values);
    } catch (const exception& e) {
        assert(str_contains(e.what(), "Values size mismatch") && "Exception should contain 'Values size mismatch'");
        threw = true;
    }
    assert(threw && "setVariables should throw for size mismatch");
}

// Test for getLowerBounds with non-constant values
TEST(test_Values_getLowerBounds_non_constant) {
    Values values;
    values["var1"] = 10;
    values["var1"].setBounds(-10.0f, 10.0f);
    values["var2"] = 20;
    values["var2"].setConstant(true);

    vector<float> bounds = values.getLowerBounds();
    assert(bounds.size() == 1 && "Should return bounds for non-constant values only");
    assert(bounds[0] == -10.0f && "Lower bound should be -10.0");
}

// Test for getUpperBounds with non-constant values
TEST(test_Values_getUpperBounds_non_constant) {
    Values values;
    values["var1"] = 10;
    values["var1"].setBounds(-10.0f, 10.0f);
    values["var1"].setConstant(false);
    
    values["var2"] = 20;
    values["var2"].setConstant(true);

    vector<float> bounds = values.getUpperBounds();
    assert(bounds.size() == 1 && "Should return bounds for non-constant values only");
    assert(bounds[0] == 10.0f && "Upper bound should be 10.0");
}

// Test for size method
TEST(test_Values_size) {
    Values values;
    assert(values.size() == 0 && "Initial size should be 0");

    values["name"] = 0;
    assert(values.size() == 1 && "Size should be 1 after adding one value");
}

// Test for dump method without show
TEST(test_Values_dump_without_show) {
    Values values;
    values["test"] = 42;

    string output = values.dump(false);
    assert(str_contains(output, "test") && "Dump should contain value name 'test'");
    assert(str_contains(output, "42") && "Dump should contain value '42'");
}

// // Test for load with invalid file mode throwing exception
// TEST(test_Values_load_invalid_mode_expected_to_throw) {
//     Values values;
//     bool threw = false;
//     try {
//         values.load<float>("test.txt", static_cast<Filemode>(999));
//     } catch (const exception& e) {
//         assert(str_contains(e.what(), "Invalid file mode") && "Exception should contain 'Invalid file mode'");
//         threw = true;
//     }
//     assert(threw && "load should throw for invalid file mode");
// }

// // Test for save with invalid file mode throwing exception
// TEST(test_Values_save_invalid_mode_expected_to_throw) {
//     Values values;
//     bool threw = false;
//     try {
//         values.save<float>("test.txt", static_cast<Filemode>(999));
//     } catch (const exception& e) {
//         assert(str_contains(e.what(), "Invalid file mode") && "Exception should contain 'Invalid file mode'");
//         threw = true;
//     }
//     assert(threw && "save should throw for invalid file mode");
// }

#endif
