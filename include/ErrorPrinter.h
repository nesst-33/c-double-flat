#pragma once

#include "LangError.h"
#include <memory>
#include <ostream>
#include <vector>

class ErrorPrinter {
public:
    ErrorPrinter(const std::vector<std::unique_ptr<LangError>>& errors)
        : m_errors(errors) {}

    void reportError(std::ostream& os, const std::unique_ptr<LangError>& error,
            const std::string& filename) const;
    void printErrors(std::ostream& os, const std::string& filename) const;

    size_t MAX_ERRS = 25;
private:
    const std::vector<std::unique_ptr<LangError>>& m_errors;
};
