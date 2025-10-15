#pragma once

// # ---- ADDING DEPENDENCIES ----
// echo "Adding dependencies..."
//
// # nlohmann/json
// git clone https://github.com/nlohmann/json libs/nlohmann/json
//
// # -----------------------------

#include "../libs/nlohmann/json/single_include/nlohmann/json.hpp"
#include "explode.hpp"
#include "implode.hpp"
#include "str_diffs_show.hpp"
#include "str_contains.hpp"
#include "ERROR.hpp"

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <regex>

using namespace std;
using namespace nlohmann;

string json_last_error = "";

// Function to remove single-line and multi-line comments
string json_remove_comments(const string& json) {
    string result;
    bool inString = false;
    bool escapeNext = false;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;

    for (size_t i = 0; i < json.size(); ++i) {
        char ch = json[i];

        if (inString && ch == '\\' && !escapeNext) {
            escapeNext = true;
            result += ch;
            continue;
        }

        if (ch == '"' && !escapeNext && !inSingleLineComment && !inMultiLineComment) {
            inString = !inString;
        }

        if (!inString && !inMultiLineComment && ch == '/' && i + 1 < json.size() && json[i + 1] == '/') {
            inSingleLineComment = true;
            i++;
            continue;
        }

        if (!inString && !inSingleLineComment && ch == '/' && i + 1 < json.size() && json[i + 1] == '*') {
            inMultiLineComment = true;
            i++;
            continue;
        }

        if (inSingleLineComment && ch == '\n') {
            inSingleLineComment = false;
        }

        if (inMultiLineComment && ch == '*' && i + 1 < json.size() && json[i + 1] == '/') {
            inMultiLineComment = false;
            i++;
            continue;
        }

        if (!inSingleLineComment && !inMultiLineComment) {
            result += ch;
        }

        escapeNext = false;
    }

    return result;
}

// Function to fix JSON by removing trailing commas
string json_fix(string json) {
    json = json_remove_comments(json);
    string result;
    bool inString = false;
    bool escapeNext = false;
    stack<char> scopeStack;

    for (size_t i = 0; i < json.size(); ++i) {
        char ch = json[i];

        if (ch == '"' && !escapeNext) {
            inString = !inString;
        }

        if (ch == '\\' && !escapeNext) {
            escapeNext = true;
        } else {
            escapeNext = false;
        }

        if (!inString) {
            if (ch == '{' || ch == '[') {
                scopeStack.push(ch);
            } else if (ch == '}' || ch == ']') {
                if (!scopeStack.empty()) {
                    scopeStack.pop();
                }
            }
        }

        if (!inString && ch == ',') {
            size_t j = i + 1;
            while (j < json.size() && (json[j] == ' ' || json[j] == '\t' || json[j] == '\n' || json[j] == '\r')) {
                ++j;
            }
            if (j < json.size() && (json[j] == '}' || json[j] == ']')) {
                continue;
            }
        }

        result += ch;
    }

    return result;
}

// Enum to represent JSON value types
enum json_type {
    JSON_TYPE_UNDEFINED,
    JSON_TYPE_NULL,
    JSON_TYPE_STRING,
    JSON_TYPE_INTEGER,
    JSON_TYPE_REAL,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT,
};

// Global variable to store the last JSON parsing error
extern string json_last_error;

// Function to convert jq-style or JavaScript-style selector to json_pointer
nlohmann::json::json_pointer _json_selector(string jselector) {
    if (jselector.empty()) return nlohmann::json::json_pointer("/");
    if (jselector[0] != '.') jselector = "." + jselector;

    vector<string> splits = explode(".", jselector);

    for (size_t i = 1; i < splits.size(); i++) {
        if (splits[i].empty())
            throw ERROR("Invalid json selector: " + jselector);

        regex valid_brackets("\\[\\s*(\\d+)\\s*\\]$");
        regex invalid_brackets("\\[[^\\]]+\\]$");
        smatch match;

        if (regex_search(splits[i], match, valid_brackets)) {
            splits[i] = regex_replace(splits[i], valid_brackets, "/$1");
            continue;
        }

        if (regex_search(splits[i], match, invalid_brackets)) {
            throw ERROR("Invalid json selector: " + jselector);
        }
    }

    int open_brackets = 0, close_brackets = 0;
    for (char ch : jselector) {
        if (ch == '[') open_brackets++;
        if (ch == ']') close_brackets++;
    }
    if (open_brackets != close_brackets) {
        throw ERROR("Invalid json selector: " + jselector);
    }

    return nlohmann::json::json_pointer(implode("/", splits));
}

// Check if a JSON string is valid
bool is_valid_json(string jstring) {
    json_last_error = "";
    try {
        auto json = nlohmann::json::parse(jstring);
        (void)json;
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        json_last_error = e.what();
        return false;
    }
}

// Get the last JSON parsing error
string get_json_error(string jstring) {
    if (is_valid_json(jstring)) return "";
    return json_last_error;
}

// Get the type of a JSON value at a given selector
json_type get_json_value_type(string jstring, string jselector) {
    try {
        auto json = nlohmann::json::parse(jstring);
        auto ptr = _json_selector(jselector);

        if (!json.contains(ptr))
            return JSON_TYPE_UNDEFINED;

        const auto& value = json.at(ptr);
        if (value.is_null()) return JSON_TYPE_NULL;
        if (value.is_string()) return JSON_TYPE_STRING;
        if (value.is_boolean()) return JSON_TYPE_BOOLEAN;
        if (value.is_number_integer()) return JSON_TYPE_INTEGER;
        if (value.is_number_float()) return JSON_TYPE_REAL;
        if (value.is_array()) return JSON_TYPE_ARRAY;
        if (value.is_object()) return JSON_TYPE_OBJECT;

        return JSON_TYPE_UNDEFINED;
    } catch (...) {
        return JSON_TYPE_UNDEFINED;
    }
}

// Convert a json_type enum to its string representation
string json_type_to_string(json_type type) {
    switch (type) {
        case JSON_TYPE_UNDEFINED: return "undefined";
        case JSON_TYPE_NULL: return "null";
        case JSON_TYPE_STRING: return "string";
        case JSON_TYPE_INTEGER: return "integer";
        case JSON_TYPE_REAL: return "real";
        case JSON_TYPE_BOOLEAN: return "boolean";
        case JSON_TYPE_ARRAY: return "array";
        case JSON_TYPE_OBJECT: return "object";
        default:
            throw ERROR("Invalid JSON type: " + to_string(type));
    }
}

// Retrieve specific JSON value types
string json_get_string(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_STRING)
        throw ERROR("Expected string type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).get<string>();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving string: " + string(e.what()));
    }
}

int json_get_int(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_INTEGER)
        throw ERROR("Expected integer type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).get<int>();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving integer: " + string(e.what()));
    }
}

double json_get_double(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_REAL)
        throw ERROR("Expected real type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).get<double>();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving double: " + string(e.what()));
    }
}

bool json_get_bool(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_BOOLEAN)
        throw ERROR("Expected boolean type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).get<bool>();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving boolean: " + string(e.what()));
    }
}

string json_get_array(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_ARRAY)
        throw ERROR("Expected array type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).dump();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving array: " + string(e.what()));
    }
}

string json_get_object(string jstring, string jselector) {
    if (get_json_value_type(jstring, jselector) != JSON_TYPE_OBJECT)
        throw ERROR("Expected object type at " + jselector);
    try {
        auto json = nlohmann::json::parse(jstring);
        return json.at(_json_selector(jselector)).dump();
    } catch (const nlohmann::json::exception& e) {
        throw ERROR("Error retrieving object: " + string(e.what()));
    }
}

// JSON class to manage JSON data
class JSON {
protected:
    string* _error = nullptr;
    nlohmann::json j;

public:
    JSON(const nlohmann::json& j) : j(j) {}
    JSON(const char* j) : JSON(string(j)) {}
    JSON(string jstring = "{}") {
        try {
            j = nlohmann::json::parse(jstring.empty() ? "{}" : json_fix(jstring));
        } catch (const exception& e) {
            _error = new string(e.what());
        }
    }

    virtual ~JSON() {
        if (_error) { delete _error; _error = nullptr; }
    }

    nlohmann::json get_json() const {
        return j;
    }

    bool isValid(string* error = nullptr) {
        if (error) *error = _error ? *_error : "";
        return !_error;
    }

    string dump(int indent = -1, char indent_char = ' ') const {
        try {
            return j.dump(indent, indent_char);
        } catch (const exception& e) {
            throw ERROR("JSON dump error: " + string(e.what()));
        }
    }
   
    bool isDefined(string jselector) const {
        try {
            nlohmann::json::json_pointer ptr = _json_selector(jselector);
            return j.contains(ptr);
        } catch (...) {
            return false;
        }
    }

    bool has(string jselector) const {
        return isDefined(jselector);
    }

    bool isNull(string jselector) {
        try {
            nlohmann::json::json_pointer ptr = _json_selector(jselector);
            return j.at(ptr).is_null();
        } catch (...) {
            return false;
        }
    }

    bool isArray(string jselector) {
        try {
            nlohmann::json::json_pointer ptr = _json_selector(jselector);
            return j.at(ptr).is_array();
        } catch (...) {
            return false;
        }
    }

    bool isObject(string jselector) {
        try {
            nlohmann::json::json_pointer ptr = _json_selector(jselector);
            return j.at(ptr).is_object();
        } catch (...) {
            return false;
        }
    }
    

    template<typename T>
    T get(string jselector) const {
        try {
            json::json_pointer ptr = _json_selector(jselector);  // Convert selector to pointer
            if constexpr (is_same_v<T, JSON>) return JSON(j.at(ptr));
            return j.at(ptr).get<T>();
        } catch (const exception& e) {
            // DEBUG(j.dump());
            throw ERROR("JSON Error at: " + jselector + ", reason: " + string(e.what()));
        }
    }

    // void set(string value) {
    //     jstring = value;
    // }

    template<typename T>
    void set(string jselector, T value) {
        try {
            // json j = json::parse(jstring);
            json::json_pointer ptr = _json_selector(jselector);
            // j[ptr] = value;
            // jstring = j.dump();
            j[ptr] = value;
        } catch (const json::exception& e) {
            //DEBUG(j.dump());
            throw ERROR("JSON Error at: " + jselector + ", reason: " + string(e.what()));
        }
    }

    // Commented-out validation helpers (retained for future use)
    // void need(const string& field) const;
    // void need(const vector<string>& fields) const;
};

namespace nlohmann {
    template<>
    struct adl_serializer<JSON> {
        static JSON from_json(const json& j) {
            return JSON(j);
        }
        
        static void to_json(json& j, const JSON& jsonObj) {
            j = jsonObj.get_json();
        }
    };
}


#ifdef TEST

TEST(test_json_remove_comments_no_comments) {
    string input = R"({"key": "value"})";
    string expected = R"({"key": "value"})";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "No comments should remain unchanged");
}

TEST(test_json_remove_comments_single_line_comment) {
    string input = R"({"key": "value"} // This is a comment)";
    string expected = R"({"key": "value"} )";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Single-line comment should be removed");
}

TEST(test_json_remove_comments_multi_line_comment) {
    string input = R"({"key": "value"} /* This is a multi-line comment */)";
    string expected = R"({"key": "value"} )";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Multi-line comment should be removed");
}

TEST(test_json_remove_comments_mixed_comments) {
    string input = R"({
        "key": "value", // Single-line comment
        /* Multi-line comment */
        "anotherKey": "anotherValue"
    })";
    string expected = R"({
        "key": "value", 
        
        "anotherKey": "anotherValue"
    })";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Mixed comments should be removed");
}

TEST(test_json_remove_comments_escaped_characters) {
    string input = R"({"key": "value with escaped \/ and \\"})";
    string expected = R"({"key": "value with escaped \/ and \\"})";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Escaped characters should remain intact");
}

TEST(test_json_remove_comments_strings_with_slashes) {
    string input = R"({"key": "http://example.com", "comment": "/* not a comment */"})";
    string expected = R"({"key": "http://example.com", "comment": "/* not a comment */"})";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Slashes inside strings should not trigger comment removal");
}

TEST(test_json_remove_comments_nested_comments) {
    string input = R"({
        /* Outer comment /* Nested comment */ */
        "key": "value"
    })";
    string expected = R"({
         */
        "key": "value"
    })";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Nested comments are not allowed");
}

TEST(test_json_remove_comments_empty_input) {
    string input = "";
    string expected = "";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Empty input should remain empty");
}

TEST(test_json_remove_comments_only_comments) {
    string input = R"(// Only a comment)";
    string expected = "";
    string actual = json_remove_comments(input);
    assert(str_diffs_show(actual, expected).empty() && "Input with only comments should be empty");
}


TEST(test_json_fix_no_trailing_commas) {
    string input = R"({"key": "value", "array": [1, 2, 3]})";
    string expected = R"({"key": "value", "array": [1, 2, 3]})";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "JSON without trailing commas should remain unchanged");
}

TEST(test_json_fix_with_trailing_commas) {
    string input = R"({"key": "value", "array": [1, 2, 3,],})";
    string expected = R"({"key": "value", "array": [1, 2, 3]})";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Trailing commas should be removed");
}

TEST(test_json_fix_with_comments) {
    string input = R"({
        "key": "value", // This is a comment
        "array": [1, 2, 3,], /* Another comment */
    })";
    string expected = R"({
        "key": "value", 
        "array": [1, 2, 3] 
    })";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Comments and trailing commas should be removed");
}

TEST(test_json_fix_empty_input) {
    string input = "";
    string expected = "";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Empty input should remain empty");
}

TEST(test_json_fix_nested_objects_and_arrays) {
    string input = R"({
        "object": {
            "key": "value",
            "array": [1, 2, 3,],
        },
        "array": [4, 5, 6,],
    })";
    string expected = R"({
        "object": {
            "key": "value",
            "array": [1, 2, 3]
        },
        "array": [4, 5, 6]
    })";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Nested objects and arrays should handle trailing commas");
}

TEST(test_json_fix_malformed_json) {
    string input = R"({"key": "value", "array": [1, 2, 3,])";
    string expected = R"({"key": "value", "array": [1, 2, 3])";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Malformed JSON should still remove trailing commas");
}

TEST(test_json_fix_escaped_backslash) {
    string input = R"({"key": "value\\",})";
    string expected = R"({"key": "value\\"})";
    string actual = json_fix(input);
    assert(str_diffs_show(actual, expected).empty() && "Escaped backslash should be preserved and trailing comma removed");
}

TEST(test_json_selector_empty) {
    json::json_pointer actual = _json_selector("");
    json::json_pointer expected("/");
    assert(actual == expected && "Empty selector should return '/'");
}

TEST(test_json_selector_simple_key) {
    json::json_pointer actual = _json_selector(".key");
    json::json_pointer expected("/key");
    assert(actual == expected && "Simple key selector should work");
}

TEST(test_json_selector_nested_keys) {
    json::json_pointer actual = _json_selector(".key1.key2.key3");
    json::json_pointer expected("/key1/key2/key3");
    assert(actual == expected && "Nested keys selector should work");
}

TEST(test_json_selector_array_indexing) {
    json::json_pointer actual = _json_selector(".array[0]");
    json::json_pointer expected("/array/0");
    assert(actual == expected && "Array indexing should be converted to '/N'");
}

TEST(test_json_selector_mixed_keys_and_indices) {
    json::json_pointer actual = _json_selector(".key1.key2[3].key3[5]");
    json::json_pointer expected("/key1/key2/3/key3/5");
    assert(actual == expected && "Mixed keys and indices should work");
}

TEST(test_json_selector_invalid_empty_part) {
    bool thrown = false;
    try {
        _json_selector(".key..key2");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Invalid json selector: .key..key2";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Empty part in selector should throw an exception");
}

TEST(test_json_selector_missing_dot_prefix) {
    json::json_pointer actual = _json_selector("key");
    json::json_pointer expected("/key");
    assert(actual == expected && "Missing dot prefix should be handled");
}

TEST(test_json_selector_invalid_bracket_syntax) {
    bool thrown = false;
    try {
        _json_selector(".array[invalid]");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Invalid json selector: .array[invalid]"; // [not-numeric] is invalid
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Invalid bracket syntax should throw an exception");
}

TEST(test_json_selector_invalid_bracket_syntax2) {
    bool thrown = false;
    try {
        _json_selector(".array]invalid]");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Invalid json selector: .array]invalid]"; // number of closing and opening braces mismacth
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Invalid bracket syntax should throw an exception");
}

TEST(test_is_valid_json_empty_string) {
    bool actual = is_valid_json("");
    assert(!actual && "Empty string should be invalid JSON");
    assert(str_contains(json_last_error, "parse error") && "Error message should match");
}

TEST(test_is_valid_json_valid_object) {
    bool actual = is_valid_json("{\"key\": \"value\"}");
    assert(actual && "Valid JSON object should pass");
    assert(json_last_error.empty() && "No error message should be set");
}

TEST(test_is_valid_json_valid_array) {
    bool actual = is_valid_json("[1, 2, 3]");
    assert(actual && "Valid JSON array should pass");
    assert(json_last_error.empty() && "No error message should be set");
}

TEST(test_is_valid_json_invalid_syntax) {
    bool actual = is_valid_json("{");
    assert(!actual && "Incomplete JSON should fail");
    assert(str_contains(json_last_error, "parse error") && "Error message should match");
}

TEST(test_is_valid_json_mismatched_brackets) {
    bool actual = is_valid_json("[}");
    assert(!actual && "Mismatched brackets should fail");
    assert(str_contains(json_last_error, "parse error") && "Error message should match");
}

TEST(test_is_valid_json_empty_object) {
    bool actual = is_valid_json("{}");
    assert(actual && "Empty JSON object should pass");
    assert(json_last_error.empty() && "No error message should be set");
}

TEST(test_is_valid_json_empty_array) {
    bool actual = is_valid_json("[]");
    assert(actual && "Empty JSON array should pass");
    assert(json_last_error.empty() && "No error message should be set");
}

TEST(test_is_valid_json_malformed_json) {
    bool actual = is_valid_json("not a json");
    assert(!actual && "Malformed JSON should fail");
    assert(str_contains(json_last_error, "parse error") && "Error message should match");
}


TEST(test_get_json_error_valid_object) {
    string actual = get_json_error("{\"key\": \"value\"}");
    assert(actual.empty() && "Valid JSON object should return no error");
}

TEST(test_get_json_error_valid_array) {
    string actual = get_json_error("[1, 2, 3]");
    assert(actual.empty() && "Valid JSON array should return no error");
}

TEST(test_get_json_error_empty_string) {
    string actual = get_json_error("");
    assert(str_contains(actual, "parse error") && "Empty string should return an error message");
}

TEST(test_get_json_error_invalid_syntax) {
    string actual = get_json_error("{");
    assert(str_contains(actual, "parse error") && "Incomplete JSON should return an error message");
}

TEST(test_get_json_error_mismatched_brackets) {
    string actual = get_json_error("[}");
    assert(str_contains(actual, "parse error") && "Mismatched brackets should return an error message");
}

TEST(test_get_json_error_empty_object) {
    string actual = get_json_error("{}");
    assert(actual.empty() && "Empty JSON object should return no error");
}

TEST(test_get_json_error_empty_array) {
    string actual = get_json_error("[]");
    assert(actual.empty() && "Empty JSON array should return no error");
}

TEST(test_get_json_error_malformed_json) {
    string actual = get_json_error("not a json");
    assert(str_contains(actual, "parse error") && "Malformed JSON should return an error message");
}

TEST(test_get_json_value_type_null) {
    json_type actual = get_json_value_type("{\"key\": null}", ".key");
    assert(actual == JSON_TYPE_NULL && "Null value should return JSON_TYPE_NULL");
}

TEST(test_get_json_value_type_string) {
    json_type actual = get_json_value_type("{\"key\": \"value\"}", ".key");
    assert(actual == JSON_TYPE_STRING && "String value should return JSON_TYPE_STRING");
}

TEST(test_get_json_value_type_boolean) {
    json_type actual = get_json_value_type("{\"key\": true}", ".key");
    assert(actual == JSON_TYPE_BOOLEAN && "Boolean value should return JSON_TYPE_BOOLEAN");
}

TEST(test_get_json_value_type_integer) {
    json_type actual = get_json_value_type("{\"key\": 42}", ".key");
    assert(actual == JSON_TYPE_INTEGER && "Integer value should return JSON_TYPE_INTEGER");
}

TEST(test_get_json_value_type_real) {
    json_type actual = get_json_value_type("{\"key\": 3.14}", ".key");
    assert(actual == JSON_TYPE_REAL && "Real value should return JSON_TYPE_REAL");
}

TEST(test_get_json_value_type_array) {
    json_type actual = get_json_value_type("{\"key\": []}", ".key");
    assert(actual == JSON_TYPE_ARRAY && "Array value should return JSON_TYPE_ARRAY");
}

TEST(test_get_json_value_type_object) {
    json_type actual = get_json_value_type("{\"key\": {}}", ".key");
    assert(actual == JSON_TYPE_OBJECT && "Object value should return JSON_TYPE_OBJECT");
}

TEST(test_get_json_value_type_undefined_key) {
    json_type actual = get_json_value_type("{\"key\": \"value\"}", ".missing");
    assert(actual == JSON_TYPE_UNDEFINED && "Undefined key should return JSON_TYPE_UNDEFINED");
}

TEST(test_get_json_value_type_invalid_json) {
    json_type actual = get_json_value_type("invalid json", ".key");
    assert(actual == JSON_TYPE_UNDEFINED && "Invalid JSON should return JSON_TYPE_UNDEFINED");
}

TEST(test_get_json_value_type_empty_json) {
    json_type actual = get_json_value_type("", ".key");
    assert(actual == JSON_TYPE_UNDEFINED && "Empty JSON should return JSON_TYPE_UNDEFINED");
}

TEST(test_json_type_to_string_undefined) {
    string actual = json_type_to_string(JSON_TYPE_UNDEFINED);
    assert(actual == "undefined" && "JSON_TYPE_UNDEFINED should return 'undefined'");
}

TEST(test_json_type_to_string_null) {
    string actual = json_type_to_string(JSON_TYPE_NULL);
    assert(actual == "null" && "JSON_TYPE_NULL should return 'null'");
}

TEST(test_json_type_to_string_string) {
    string actual = json_type_to_string(JSON_TYPE_STRING);
    assert(actual == "string" && "JSON_TYPE_STRING should return 'string'");
}

TEST(test_json_type_to_string_integer) {
    string actual = json_type_to_string(JSON_TYPE_INTEGER);
    assert(actual == "integer" && "JSON_TYPE_INTEGER should return 'integer'");
}

TEST(test_json_type_to_string_real) {
    string actual = json_type_to_string(JSON_TYPE_REAL);
    assert(actual == "real" && "JSON_TYPE_REAL should return 'real'");
}

TEST(test_json_type_to_string_boolean) {
    string actual = json_type_to_string(JSON_TYPE_BOOLEAN);
    assert(actual == "boolean" && "JSON_TYPE_BOOLEAN should return 'boolean'");
}

TEST(test_json_type_to_string_array) {
    string actual = json_type_to_string(JSON_TYPE_ARRAY);
    assert(actual == "array" && "JSON_TYPE_ARRAY should return 'array'");
}

TEST(test_json_type_to_string_object) {
    string actual = json_type_to_string(JSON_TYPE_OBJECT);
    assert(actual == "object" && "JSON_TYPE_OBJECT should return 'object'");
}

// Use -Wconversion flag instead!
// TEST(test_json_type_to_string_invalid_type) {
//     bool thrown = false;
//     try {
//         json_type_to_string(static_cast<json_type>(99)); // Invalid type
//     } catch (const exception& e) {
//         thrown = true;
//         string actual = e.what();
//         string expected = "Invalid JSON type: 99";
//         assert(str_contains(actual, expected) && "Exception message should match");
//     }
//     assert(thrown && "Invalid JSON type should throw an exception");
// }

TEST(test_json_get_string_valid) {
    string actual = json_get_string("{\"key\": \"value\"}", ".key");
    assert(actual == "value" && "Valid string should be retrieved");
}

TEST(test_json_get_string_invalid_type) {
    bool thrown = false;
    try {
        json_get_string("{\"key\": 42}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected string type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-string type should throw an exception");
}

TEST(test_json_get_string_undefined_key) {
    bool thrown = false;
    try {
        json_get_string("{\"key\": \"value\"}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected string type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_string_invalid_json) {
    bool thrown = false;
    try {
        json_get_string("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected string type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_string_empty_json) {
    bool thrown = false;
    try {
        json_get_string("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected string type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_json_get_int_valid) {
    int actual = json_get_int("{\"key\": 42}", ".key");
    assert(actual == 42 && "Valid integer should be retrieved");
}

TEST(test_json_get_int_invalid_type) {
    bool thrown = false;
    try {
        json_get_int("{\"key\": \"value\"}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected integer type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-integer type should throw an exception");
}

TEST(test_json_get_int_undefined_key) {
    bool thrown = false;
    try {
        json_get_int("{\"key\": 42}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected integer type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_int_invalid_json) {
    bool thrown = false;
    try {
        json_get_int("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected integer type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_int_empty_json) {
    bool thrown = false;
    try {
        json_get_int("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected integer type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_json_get_double_valid) {
    double actual = json_get_double("{\"key\": 3.14}", ".key");
    assert(actual == 3.14 && "Valid double should be retrieved");
}

TEST(test_json_get_double_invalid_type) {
    bool thrown = false;
    try {
        json_get_double("{\"key\": \"value\"}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected real type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-double type should throw an exception");
}

TEST(test_json_get_double_undefined_key) {
    bool thrown = false;
    try {
        json_get_double("{\"key\": 3.14}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected real type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_double_invalid_json) {
    bool thrown = false;
    try {
        json_get_double("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected real type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_double_empty_json) {
    bool thrown = false;
    try {
        json_get_double("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected real type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_json_get_bool_true) {
    bool actual = json_get_bool("{\"key\": true}", ".key");
    assert(actual && "Valid boolean 'true' should be retrieved");
}

TEST(test_json_get_bool_false) {
    bool actual = json_get_bool("{\"key\": false}", ".key");
    assert(!actual && "Valid boolean 'false' should be retrieved");
}

TEST(test_json_get_bool_invalid_type) {
    bool thrown = false;
    try {
        json_get_bool("{\"key\": \"value\"}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected boolean type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-boolean type should throw an exception");
}

TEST(test_json_get_bool_undefined_key) {
    bool thrown = false;
    try {
        json_get_bool("{\"key\": true}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected boolean type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_bool_invalid_json) {
    bool thrown = false;
    try {
        json_get_bool("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected boolean type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_bool_empty_json) {
    bool thrown = false;
    try {
        json_get_bool("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected boolean type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_json_get_array_valid) {
    string actual = json_get_array("{\"key\": [1, 2, 3]}", ".key");
    assert(actual == "[1,2,3]" && "Valid array should be retrieved");
}

TEST(test_json_get_array_invalid_type) {
    bool thrown = false;
    try {
        json_get_array("{\"key\": \"value\"}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected array type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-array type should throw an exception");
}

TEST(test_json_get_array_undefined_key) {
    bool thrown = false;
    try {
        json_get_array("{\"key\": [1, 2, 3]}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected array type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_array_invalid_json) {
    bool thrown = false;
    try {
        json_get_array("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected array type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_array_empty_json) {
    bool thrown = false;
    try {
        json_get_array("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected array type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_json_get_object_valid) {
    string actual = json_get_object("{\"key\": {\"nested\": \"value\"}}", ".key");
    assert(actual == "{\"nested\":\"value\"}" && "Valid object should be retrieved");
}

TEST(test_json_get_object_invalid_type) {
    bool thrown = false;
    try {
        json_get_object("{\"key\": \"value\"}", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected object type at .key";
        assert(str_contains(actual, expected) && "Exception message should match");
    }
    assert(thrown && "Non-object type should throw an exception");
}

TEST(test_json_get_object_undefined_key) {
    bool thrown = false;
    try {
        json_get_object("{\"key\": {\"nested\": \"value\"}}", ".missing");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected object type at .missing";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Undefined key should throw an exception");
}

TEST(test_json_get_object_invalid_json) {
    bool thrown = false;
    try {
        json_get_object("invalid json", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected object type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Invalid JSON should throw an exception");
}

TEST(test_json_get_object_empty_json) {
    bool thrown = false;
    try {
        json_get_object("", ".key");
    } catch (const exception& e) {
        thrown = true;
        string actual = e.what();
        string expected = "Expected object type at .key";
        assert(str_contains(actual, expected) && "Exception message should contain expected substring");
    }
    assert(thrown && "Empty JSON should throw an exception");
}

TEST(test_JSON_constructor_valid) {
    JSON json("{\"key\": \"value\"}");
    assert(json.isValid() && "Valid JSON should be parsed without errors");
}

TEST(test_JSON_constructor_invalid) {
    JSON json("{invalid json}");
    assert(!json.isValid() && "Invalid JSON should set an error");
}

TEST(test_JSON_dump) {
    JSON json("{\"key\": \"value\"}");
    string actual = json.dump();
    assert(actual == "{\"key\":\"value\"}" && "Dump should return simulated JSON string");
}

TEST(test_JSON_isDefined_true) {
    JSON json("{\"key\": \"value\"}");
    bool actual = json.isDefined(".key");
    assert(actual && "Defined key should return true");
}

TEST(test_JSON_isDefined_false) {
    JSON json("{\"key\": \"value\"}");
    bool actual = json.isDefined(".missing");
    assert(!actual && "Undefined key should return false");
}

TEST(test_JSON_isNull_true) {
    JSON json("{\"key\": null}");
    bool actual = json.isNull(".key");
    assert(actual && "Null value should return true");
}

TEST(test_JSON_isNull_false) {
    JSON json("{\"key\": \"value\"}");
    bool actual = json.isNull(".key");
    assert(!actual && "Non-null value should return false");
}

TEST(test_JSON_isArray_true) {
    JSON json("{\"key\": []}");
    bool actual = json.isArray(".key");
    assert(actual && "Array value should return true");
}

TEST(test_JSON_isArray_false) {
    JSON json("{\"key\": \"value\"}");
    bool actual = json.isArray(".key");
    assert(!actual && "Non-array value should return false");
}

TEST(test_JSON_isObject_true) {
    JSON json("{\"key\": {}}");
    bool actual = json.isObject(".key");
    assert(actual && "Object value should return true");
}

TEST(test_JSON_isObject_false) {
    JSON json("{\"key\": \"value\"}");
    bool actual = json.isObject(".key");
    assert(!actual && "Non-object value should return false");
}

TEST(test_JSON_get_int) {
    JSON json("{\"key\": 42}");
    int actual = json.get<int>(".key");
    assert(actual == 42 && "Integer value should be retrieved correctly");
}

TEST(test_JSON_get_string) {
    JSON json("{\"key\": \"value\"}");
    string actual = json.get<string>(".key");
    assert(actual == "value" && "String value should be retrieved correctly");
}

TEST(test_JSON_set) {
    JSON json("{\"key\": \"value\"}");
    json.set(".key", 42);
    int actual = json.get<int>(".key");
    assert(actual == 42 && "Set value should update the JSON object");
}

#endif
