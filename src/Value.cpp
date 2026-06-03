#include "Value.h"
#include <stdexcept>

std::variant<int, double> Value::asNumber() const {
    if (const int* val = std::get_if<int>(&m_data)) {
        return *val;
    }
    if (const double* val = std::get_if<double>(&m_data))
        return *val;
    if (const bool* val = std::get_if<bool>(&m_data))
        return *val ? 1 : 0;
    if (const std::string* val = std::get_if<std::string>(&m_data)) {
        try {
            if (val->find('.') != std::string::npos)
                return std::stod(*val);
            return std::stoi(*val);
        } catch (std::invalid_argument& e) {
            throw std::runtime_error("Cannot implicitly convert string '" 
                    + *val + "' into a number type");
        } catch (std::out_of_range& e) {
            throw std::runtime_error("Cannot implicitly convert string '" 
                    + *val + "' into a number type - value is out of range");
        }
    }

    throw std::runtime_error("Cannot convert null to a number");
}
