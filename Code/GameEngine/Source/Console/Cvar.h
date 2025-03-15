#pragma once

#include <string>

/*
 * wwCVarFlags - bitmask flags for cvars
 */
enum wwCVarFlags : unsigned int {
    CVAR_NONE = 0,
    CVAR_BOOL = 1 << 0,   // 0001
    CVAR_INT = 1 << 1,   // 0010
    CVAR_FLOAT = 1 << 2,   // 0100
    CVAR_STRING = 1 << 3,   // 1000
    // Additional flags (e.g. CVAR_ARCHIVE) can be added here
    CVAR_ARCHIVE = 1 << 4
};

/*
 * wwCVar - A single configuration variable
 */
class wwCVar {
public:
    // Constructor
    wwCVar(const std::string& name,
        const std::string& defaultValue,
        const std::string& description = "",
        unsigned int flags = CVAR_NONE);

    // Accessors
    const std::string& GetName() const;
    const std::string& GetDescription() const;
    const std::string& GetDefaultValue() const;
    const std::string& GetString() const;

    // Type conversions
    bool  GetBool()  const;
    int   GetInt()   const;
    float GetFloat() const;

    // Set from string
    void SetString(const std::string& newValue);

    // Resets the cvar to its default value
    void Reset();

    // Returns true if the given flag is set
    bool IsFlagSet(unsigned int checkFlag) const;

    void ResetChanged() { isChanged = false; }
private:
    bool isChanged = false;
    std::string  name;
    std::string  description;
    std::string  defaultValue;
    unsigned int flags;
    std::string  currentValue;
};

