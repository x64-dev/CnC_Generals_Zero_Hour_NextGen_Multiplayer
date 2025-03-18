#include "PreRTS.h"

#include <stdexcept>
#include <iostream>

wwCVarManager::wwCVarManager() {

}

void wwCVarManager::RegisterCVar(wwCVar* cvar) {
	if (!cvar) {
		throw std::runtime_error("wwCVarManager::RegisterCVar - cvar is null");
	}
	auto it = cvars.find(cvar->GetName());
	if (it != cvars.end()) {
		// Already registered. Log a warning or handle as needed.
		std::cerr << "wwCVarManager: Warning - CVar '"
			<< cvar->GetName()
			<< "' already registered.\n";
	}
	// Insert/overwrite in map
	cvars[cvar->GetName()] = cvar;
}

// A simple helper for case-insensitive string equality:
static bool CaseInsensitiveEqual(const std::string& a, const std::string& b)
{
	// Quick length check:
	if (a.size() != b.size())
		return false;

	// Compare each character case-insensitively:
	for (size_t i = 0; i < a.size(); i++)
	{
		if (std::tolower(static_cast<unsigned char>(a[i])) !=
			std::tolower(static_cast<unsigned char>(b[i])))
		{
			return false;
		}
	}
	return true;
}

wwCVar* wwCVarManager::FindCVar(const std::string& name)
{
	// Iterate over all entries in the map
	for (auto& kv : cvars)
	{
		// Compare the map key vs. the requested name, ignoring case
		if (CaseInsensitiveEqual(kv.first, name))
		{
			return kv.second;
		}
	}
	return nullptr;
}

wwCVar* wwCVarManager::FindCVarConst(const std::string& name) const
{
	for (const auto& kv : cvars)
	{
		if (CaseInsensitiveEqual(kv.first, name))
		{
			return kv.second;
		}
	}
	return nullptr;
}

std::string wwCVarManager::GetCVarString(const std::string& name) const {
	wwCVar* cvar = FindCVarConst(name);
	return (cvar ? cvar->GetString() : "");
}

float wwCVarManager::GetCVarFloat(const std::string& name) const {
	wwCVar* cvar = FindCVarConst(name);
	return (cvar ? cvar->GetFloat() : 0.0f);
}

int wwCVarManager::GetCVarInt(const std::string& name) const {
	wwCVar* cvar = FindCVarConst(name);
	return (cvar ? cvar->GetInt() : 0);
}

bool wwCVarManager::GetCVarBool(const std::string& name) const {
	wwCVar* cvar = FindCVarConst(name);
	return (cvar ? cvar->GetBool() : false);
}

void wwCVarManager::SetCVar(const std::string& name, const std::string& value) {
	wwCVar* cvar = FindCVar(name);
	if (!cvar) {
		std::cerr << "wwCVarManager::SetCVar - CVar '"
			<< name << "' not found.\n";
		return;
	}
	cvar->SetString(value);
}

void wwCVarManager::ResetCVar(const std::string& name) {
	wwCVar* cvar = FindCVar(name);
	if (cvar) {
		cvar->Reset();
	}
}

void wwCVarManager::ResetAll() {
	for (auto& kv : cvars) {
		kv.second->Reset();
	}
}
