#pragma once

#include <string>
#include <unordered_map>

// Forward declaration to avoid circular includes
class wwCVar;

/*
 * wwCVarManager - Singleton manager to register, find, and manipulate wwCVar instances.
 */
class wwCVarManager {
public:
    wwCVarManager();

    // Register a new cvar
    void RegisterCVar(wwCVar* cvar);

    // Find a cvar by name
    wwCVar* FindCVar(const std::string& name);

    // Convenience getters (return 0, false, or empty string if cvar not found)
    std::string GetCVarString(const std::string& name) const;
    float       GetCVarFloat(const std::string& name) const;
    int         GetCVarInt(const std::string& name)   const;
    bool        GetCVarBool(const std::string& name)  const;

    // Set cvar by string
    void SetCVar(const std::string& name, const std::string& value);

    // Reset an individual cvar to default
    void ResetCVar(const std::string& name);

    // Reset all cvars to their defaults
    void ResetAll();

private:
    // No copy or assignment
    wwCVarManager(const wwCVarManager&) = delete;
    wwCVarManager& operator=(const wwCVarManager&) = delete;

    // Helper to find cvar (const version)
    wwCVar* FindCVarConst(const std::string& name) const;

private:
    std::unordered_map<std::string, wwCVar*> cvars;
};

extern wwCVarManager cvarManager;