#include "val.h"

// Multiplication operator*
Value Value::operator*(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Returns error value for any operations involving error or string types
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp * op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left * right);
    }
    return Value(); // Fallback to error for any undefined behavior
}

// Less than operator<
Value Value::operator<(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Return error when any operand is an error or a string
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp < op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left < right);
    }
    return Value(); // Return error if types are not comparable
}

// String concatenation
Value Value::Catenate(const Value& op) const {
    if (IsErr() || op.IsErr()) {
        return Value(); // Returns error value for any operations involving error values
    } else if (IsString() && op.IsString()) {
        return Value(Stemp + op.Stemp);
    } else {
        return Value(); // Return an error Value if either operand is not a string
    }
}

Value Value::Power(const Value& op) const {
    if (IsErr() || op.IsErr()) {
        return Value(); // Returns error value for any operations involving error values
    } else if ((IsReal() || IsInt()) && (op.IsReal() || op.IsInt())) {
        double base = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double exponent = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(std::pow(base, exponent));
    } else {
        return Value(); // Return an error Value for incompatible types
    }
}

// Addition operator+
Value Value::operator+(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Returns error value for any operations involving error or string types
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp + op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left + right);
    }
    return Value(); // Fallback to error for any undefined behavior
}

// Subtraction operator-
Value Value::operator-(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Returns error value for any operations involving error or string types
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp - op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left - right);
    }
    return Value(); // Fallback to error for any undefined behavior
}

// Division operator/
Value Value::operator/(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Returns error value for any operations involving error or string types
    } else if (op.IsInt() && op.GetInt() == 0) {
        throw std::runtime_error("Division by zero");
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp / op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left / right);
    }
    return Value(); // Fallback to error for any undefined behavior
}

// Equality operator==
Value Value::operator==(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() || op.IsString()) {
        return Value(); // Return error when any operand is an error or a string
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp == op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(fabs(left - right) < 0.0001);
    }
    return Value(); // Return error if types are not comparable
}

// Greater than operator>
Value Value::operator>(const Value& op) const {
    if (IsErr() || op.IsErr() || IsString() or op.IsString()) {
        return Value(); // Return error when any operand is an error or a string
    } else if (IsInt() && op.IsInt()) {
        return Value(Itemp > op.Itemp);
    } else if (IsReal() || op.IsReal()) {
        double left = IsInt() ? static_cast<double>(Itemp) : Rtemp;
        double right = op.IsInt() ? static_cast<double>(op.Itemp) : op.Rtemp;
        return Value(left > right);
    }
    return Value(); // Return error if types are not comparable
}

