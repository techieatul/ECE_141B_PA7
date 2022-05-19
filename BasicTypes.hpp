//
//  BasicTypes.hpp
//  RGAssignement1
//
//  Created by rick gessner on 3/9/21.
//
#pragma once
#ifndef BasicTypes_h
#define BasicTypes_h

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace ECE141 {

enum class DataTypes {
    no_type = 'N',
    bool_type = 'B',
    datetime_type = 'D',
    float_type = 'F',
    int_type = 'I',
    varchar_type = 'V',
};

using StringList = std::vector<std::string>;
using StringMap = std::map<std::string, std::string>;
using StringOpt = std::optional<std::string>;
using IntOpt = std::optional<uint32_t>;
using Value = std::variant<bool, int, double, std::string>;
using KeyValues = std::map<const std::string, Value>;

}  // namespace ECE141
#endif /* BasicTypes_h */
