#include "ErrorPrinter.h"
#include <fstream>
#include <string>
#include <string_view>

namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string YELLOW  = "\033[33m";
    const std::string CYAN    = "\033[36m";
    const std::string GRAY    = "\033[90m";
}

void ErrorPrinter::reportError(std::ostream& os, const std::unique_ptr<LangError>& error,
        const std::string& filename) const {
    Position pos = error->getPosition();


    os << "\n" << Color::BOLD;
    switch (error->getSeverity()) {
        case Severity::WARNING:
            os << Color::YELLOW;
            break;
        case Severity::ERROR:
        case Severity::PANIC:
            os << Color::RED;
    }

    os << error->getPrefix() << Color::RESET << error->getMsg() 
        << Color::RESET << "\n" << Color::GRAY << " --> " << Color::CYAN << filename 
        << " (line: " << pos.line << ", column: " << pos.column << ")"
        << Color::RESET << "\n";

    std::ifstream file(filename);
    if (!file.is_open()) {
        os << "Couldn't reopen file for context\n";
        return;
    }

    std::string lineSnippet;
    size_t currLine{1};
    for (; currLine <= pos.line && std::getline(file, lineSnippet); ++currLine);
    file.close();

    std::string caretPadding = "";
    for (size_t i = 0; i < (pos.column - 1) && i < lineSnippet.size(); ++i) {
        if (lineSnippet[i] == '\t') 
            caretPadding += '\t';
        else 
            caretPadding += ' ';
    }
    std::string caretLine = caretPadding + "^";

    std::string linePrefix = std::to_string(pos.line) + " | ";
    std::string emptyPrefix = std::string(std::to_string(pos.line).size(), ' ') + " | ";
    
    os << Color::GRAY << emptyPrefix << "\n"
        << linePrefix << Color::RESET << lineSnippet << "\n"
        << Color::GRAY << emptyPrefix << Color::RESET << Color::RED << caretLine << Color::RESET << "\n"
        << Color::GRAY << emptyPrefix << "\n";  

}

void ErrorPrinter::printErrors(std::ostream& os, const std::string& filename) const {
    for (size_t i{}; i < m_errors.size(); i++) {
        if (i >= MAX_ERRS)
            os << "Max. number of errors exceeded (" << MAX_ERRS
                << "). Stopping now...";
        reportError(os, m_errors[i], filename);
    }
}
