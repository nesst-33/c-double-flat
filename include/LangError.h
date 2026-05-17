#ifndef _LANG_ERROR_H
#define _LANG_ERROR_H

#include <stdexcept>

enum class Severity { WARNING, ERROR, PANIC };

class LangError : public std::runtime_error {
public:
    LangError(const std::string& msg, Severity severity)
        : std::runtime_error(msg), m_severity(severity) {}
    virtual ~LangError() = default;
    virtual void raise() const = 0;
    Severity getSeverity() const { return m_severity; }
private:
    Severity m_severity;
};

class SyntaxError : public LangError {
public:
    void raise() const override { throw *this; }
};

class LexerError : public LangError {
public:
    void raise() const override { throw *this; }
};

class InterpreterError : public LangError {
public:
    void raise() const override { throw *this; }
};

#endif
