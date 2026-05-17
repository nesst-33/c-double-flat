#ifndef _LANG_ERROR_H
#define _LANG_ERROR_H

#include <sstream>
#include <string>
#include <stdexcept>
#include <Token.h>

enum class Severity { WARNING, ERROR, PANIC };

class LangError : public std::runtime_error {
public:
    LangError(const std::string& msg, Severity severity, Position pos)
        : std::runtime_error("Unknown exception")
        , m_msg(msg)
        , m_severity(severity)
        , m_pos(pos) {}

    virtual ~LangError() = default;
    virtual void raise() const = 0;
    Severity getSeverity() const { return m_severity; }
    const Position& getPosition() const { return m_pos; }

    const char* what() const noexcept override {
        std::ostringstream formatted;
        formatted << getPrefix() << m_msg << " (" << m_pos.print() << ")";
        return m_msg.c_str();
    }

    virtual std::string getPrefix() const {
        return (m_severity == Severity::WARNING) ? "WARNING: " : "Unknown Error: ";
    }

protected:
    Severity m_severity;
private:
    Position m_pos;
    std::string m_msg;
};

class SyntaxError : public LangError {
public:
    void raise() const override { throw *this; }

    std::string getPrefix() const override {
        return (m_severity == Severity::WARNING) ? "WARNING: " : "Syntax Error: ";
    }
};

class LexerError : public LangError {
public:
    void raise() const override { throw *this; }

    std::string getPrefix() const override {
        return (m_severity == Severity::WARNING) ? "WARNING: " : "Syntax Error: ";
    }
};

class InterpreterError : public LangError {
public:
    void raise() const override { throw *this; }
};

#endif
