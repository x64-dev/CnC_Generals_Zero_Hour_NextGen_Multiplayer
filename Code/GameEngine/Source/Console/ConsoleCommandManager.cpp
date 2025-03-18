#include "PreRTS.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

WWCONSOLE_COMMAND(TestCommand, "Tests the commandManager system") {
	DevConsole.AddLog("The Test command is working");
}

WWCONSOLE_COMMAND(Quit, "Quits the game") {
	ExitProcess(0);
}

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

void ConsoleCommandManager::ExecuteInput(const std::string& input) {
	std::vector<std::string> tokens = SplitString(input, ' ');
	if (tokens.empty()) {
		return;
	}

	std::string commandName = tokens[0];
	tokens.erase(tokens.begin());  // remove the command name from the args

	for (auto& kv : commands_)
	{
		// Compare the map key vs. the requested name, ignoring case
		if (CaseInsensitiveEqual(kv.first, commandName))
		{
			kv.second->Execute(tokens);
			return;
		}
	}


	DevConsole.AddLog("Unknown command: '%s'", input.c_str());
}