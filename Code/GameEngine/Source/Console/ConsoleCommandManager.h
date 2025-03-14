#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>

/*
 * The interface that all console commands must implement.
 * `Execute(args)` will be called with the parameters the user typed.
 */
class IConsoleCommand {
public:
    virtual ~IConsoleCommand() = default;
    virtual void Execute(const std::vector<std::string>& args) = 0;
    virtual std::string GetHelpText() const = 0;
};

/*
 * A simple function-based command wrapper:
 * The console system calls `Execute()`, which in turn invokes the function pointer.
 */
using ConsoleCommandFn = void(*)(const std::vector<std::string>&);

class FunctionConsoleCommand : public IConsoleCommand {
private:
    ConsoleCommandFn       callback_;
    std::string           helpText_;

public:
    FunctionConsoleCommand(ConsoleCommandFn fn, const std::string& help)
        : callback_(fn), helpText_(help) {
    }

    void Execute(const std::vector<std::string>& args) override {
        if (callback_) {
            callback_(args);
        }
    }

    std::string GetHelpText() const override {
        return helpText_;
    }
};

class ConsoleCommandManager {
private:
	// Map: command name -> command object
	std::unordered_map<std::string, std::unique_ptr<IConsoleCommand>> commands_;

public:
	// Register a command by name
	void RegisterCommand(const std::string& name, ConsoleCommandFn fn, const std::string& help = "") {
		commands_[name] = std::make_unique<FunctionConsoleCommand>(fn, help);
	}


    void ExecuteInput(const std::string& input);

	// For tab completion or listing
	std::vector<std::string> GetAllCommandNames() const {
		std::vector<std::string> names;
		names.reserve(commands_.size());
		for (auto& kv : commands_) {
			names.push_back(kv.first);
		}
		return names;
	}

private:
	// Helper to split strings by a delimiter
	static std::vector<std::string> SplitString(const std::string& s, char delimiter) {
		std::vector<std::string> tokens;
		std::istringstream ss(s);
		std::string token;
		while (std::getline(ss, token, delimiter)) {
			if (!token.empty()) {
				tokens.push_back(token);
			}
		}
		return tokens;
	}
};

inline ConsoleCommandManager& GetConsoleManager() {
	static ConsoleCommandManager instance;  // Constructed on first call
	return instance;
}

#define WWCONSOLE_COMMAND(cmdName, helpText)                                       \
    /* Forward-declare the actual function that will implement the command */      \
    static void cmdName##_cmdFunc(const std::vector<std::string>& args);           \
                                                                                   \
    /* A small struct whose constructor registers the command with the manager */  \
    struct cmdName##_CommandRegistrar {                                            \
        cmdName##_CommandRegistrar() {                                             \
            GetConsoleManager().RegisterCommand(#cmdName, &cmdName##_cmdFunc, helpText);  \
        }                                                                          \
    };                                                                             \
                                                                                   \
    /* A static instance of that struct to force registration at startup */        \
    static cmdName##_CommandRegistrar s_##cmdName##_CommandRegistrar;              \
                                                                                   \
    /* Now define the actual command function body */                              \
    static void cmdName##_cmdFunc(const std::vector<std::string>& args)

