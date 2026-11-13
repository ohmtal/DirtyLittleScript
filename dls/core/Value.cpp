
#ifndef IMPLEMENT_DLS
#include "Value.h"
#endif


namespace DLS {


    const std::string Value::toString() const{
        switch (type) {
            case ValueType::Double: return std::to_string(d);
            case ValueType::Long: return std::to_string(l);
            case ValueType::String: return getString();
            case ValueType::Pointer: return  std::format("{:p}", static_cast<void*>(p));
            case ValueType::Array: return  "*Array*";
            case ValueType::Function: return  "*Function*";
            default:
                static const std::string empty = "";
                return empty;
        }
    }

    S64 Value::getLongSlowPath(ValueType type, S64 payload_l, F64 payload_d) const{
        if (type == ValueType::Double) {
            return static_cast<S64>(payload_d);
        }
        if (type == ValueType::Array) {
            return static_cast<S64>(payload_l);
        }
        if (type == ValueType::Pointer) {
            return (payload_l != 0) ? 1 : 0; // p != nullptr
        }
        return 0;
    }
}
