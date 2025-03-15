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
	// If flagged as bool, interpret "0" / "false" as false; else true
	// Otherwise, fallback to a simple "0"/"false" check as well
	if (flags & CVAR_BOOL) {
		return (currentValue != "0" && currentValue != "false");
	}
	// fallback
	return (currentValue != "0" && currentValue != "false");
}

int wwCVar::GetInt() const {
	// If flagged as int, parse as int
	// fallback is also stoi, but you could throw or handle differently
	return std::stoi(currentValue);
}

float wwCVar::GetFloat() const {
	// If flagged as float, parse as float
	// fallback is also stof
	return std::stof(currentValue);
}

void wwCVar::SetString(const std::string& newValue) {
	// Optionally, you could clamp or validate values here before setting
	currentValue = newValue;
}

void wwCVar::Reset() {
	currentValue = defaultValue;
}

bool wwCVar::IsFlagSet(unsigned int checkFlag) const {
	return (flags & checkFlag) != 0;
}
