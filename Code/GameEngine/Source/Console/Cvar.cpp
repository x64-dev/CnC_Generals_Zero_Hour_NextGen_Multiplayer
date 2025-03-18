#include "PreRTS.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib> // for std::stoi, std::stof

wwCVar::wwCVar(const std::string& name,
	const std::string& defaultValue,
	const std::string& description,
	unsigned int flags)
	: name(name)
	, description(description)
	, defaultValue(defaultValue)
	, flags(flags)
	, currentValue(defaultValue)
{
	// Parse defaultValue once at construction
	if (IsFlagSet(CVAR_BOOL)) {
		boolValue = StringToBool(defaultValue);
	}
	if (IsFlagSet(CVAR_INT)) {
		intValue = StringToInt(defaultValue);
	}
	if (IsFlagSet(CVAR_FLOAT)) {
		floatValue = StringToFloat(defaultValue);
	}

	GetCVarManager().RegisterCVar(this);
}

const std::string& wwCVar::GetName() const {
	return name;
}

const std::string& wwCVar::GetDescription() const {
	return description;
}

const std::string& wwCVar::GetDefaultValue() const {
	return defaultValue;
}

const std::string& wwCVar::GetString() const {
	return currentValue;
}

bool wwCVar::GetBool() const {
	return boolValue;
}

int wwCVar::GetInt() const {
	return intValue;
}

float wwCVar::GetFloat() const {
	return floatValue;
}

void wwCVar::SetString(const std::string& newValue) {
	// Optionally, you could clamp or validate values here before setting
	currentValue = newValue;
	// Re-parse as needed based on flags
	if (IsFlagSet(CVAR_BOOL)) {
		boolValue = StringToBool(newValue);
	}
	if (IsFlagSet(CVAR_INT)) {
		intValue = StringToInt(newValue);
	}
	if (IsFlagSet(CVAR_FLOAT)) {
		floatValue = StringToFloat(newValue);
	}

	isChanged = true;
}

void wwCVar::Reset() {
	currentValue = defaultValue;
}

bool wwCVar::IsFlagSet(unsigned int checkFlag) const {
	return (flags & checkFlag) != 0;
}


// Helper for string -> bool. You can tailor the exact conditions:
bool wwCVar::StringToBool(const std::string& s)
{
	// A simple case-insensitive convert. You might want to add more checks or rules.
	std::string lower;
	lower.resize(s.size());
	std::transform(s.begin(), s.end(), lower.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (lower == "1" || lower == "true" || lower == "yes" || lower == "on")
		return true;
	return false;
}

// Helper for string -> int
int wwCVar::StringToInt(const std::string& s)
{
	// Use std::stoi and handle errors if you want
	return std::stoi(s);
}

// Helper for string -> float
float wwCVar::StringToFloat(const std::string& s)
{
	// Use std::stof and handle errors if you want
	return std::stof(s);
}