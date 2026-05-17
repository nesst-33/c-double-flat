#include <vector>
#include <memory>
#include "LangError.h"

// TODO: implement the option to choose an error handling policy
class ErrorHandler {
public:
    void report(std::unique_ptr<LangError> error) {
        if (!error)
            throw std::runtime_error("Errors passes to the handler can't be nullptr");

        errors.push_back(std::move(error)); 

        if (errors.back()->getSeverity() == Severity::ERROR)
            errors.back()->raise();
    }
private:
    std::vector<std::unique_ptr<LangError>> errors;
};
