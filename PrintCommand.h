class PrintCommand : public ICommand {
    std::string prefix;
    std::string varName;
    std::string outputInt;

public:
    // Old style: PRINT varName
    PrintCommand(const std::string& varName)
        : prefix(""), varName(varName) {}

    // New style: PRINT("prefix" + varName)
    PrintCommand(const std::string& prefix, const std::string& varName)
        : prefix(prefix), varName(varName) {}

    void execute(Process& process) override {
        const auto& vars = process.getVariables();
        std::string output;

        if (vars.count(varName)) {
            outputInt = std::to_string(vars.at(varName));
            output = prefix + outputInt;
        } else {
            output = "Variable '" + varName + "' is undefined";
        }

        std::ostringstream oss;
        oss << "(" << ScreenInfo::getCurrentTimestamp()
            << ") Core:" << process.getScreenInfo().getCoreID()
            << " \"" << output << "\"";

        process.addLog(oss.str());
    }

    std::string toString() const override {
        if (!prefix.empty() && !outputInt.empty()) {
            return "PRINT " + prefix + " " + outputInt;
        } else if (!prefix.empty() && outputInt.empty()) {
            return "PRINT " + prefix + varName + " is empty";
        } else if (!outputInt.empty()) {
            return "PRINT " + varName + " = " + outputInt;
        } else {
            return "PRINT " + varName + " is empty";
        }
    }

    std::string getName() const override {
        return "PRINT";
    }
};