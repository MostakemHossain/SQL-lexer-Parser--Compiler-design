#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

vector<string> tokenize(const string& input) {
    vector<string> tokens;
    string token;
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (isspace(ch)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else if (ch == ',' || ch == '(' || ch == ')' || ch == ';' || ch == '.' || ch == '=') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.emplace_back(1, ch);
        } else {
            token += ch;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

string toUpper(const string& str) {
    string upper;
    for (char ch : str) upper += toupper(ch);
    return upper;
}

// Parse CREATE statement
bool parseCreate(const vector<string>& tokens) {
    if (tokens.size() < 5 || toUpper(tokens[1]) != "TABLE") return false;
    if (tokens[3] != "(" || tokens.back() != ";") return false;
    int parens = 0;
    for (size_t i = 3; i < tokens.size(); ++i) {
        if (tokens[i] == "(") ++parens;
        else if (tokens[i] == ")") --parens;
    }
    return parens == 0;
}

// Parse INSERT statement
bool parseInsert(const vector<string>& tokens) {
    if (tokens.size() < 6) return false;
    if (toUpper(tokens[1]) != "INTO") return false;
    auto it = find(tokens.begin(), tokens.end(), "VALUES");
    if (it == tokens.end()) return false;
    return tokens.back() == ";";
}

// Parse SELECT statement
bool parseSelect(const vector<string>& tokens) {
    if (tokens.size() < 4) return false;
    if (toUpper(tokens[2]) != "FROM") return false;

    // Reject JOIN queries explicitly
    for (const string& token : tokens) {
        if (toUpper(token) == "JOIN") return false;
    }

    // Allow multiple columns in the SELECT clause
    size_t index = 1;
    while (index < tokens.size() - 2 && tokens[index] != "FROM") {
        if (tokens[index] == ",") continue; // Skip commas
        ++index;
    }

    return tokens.back() == ";";
}

// Parse DELETE statement
bool parseDelete(const vector<string>& tokens) {
    if (tokens.size() < 4) return false;
    if (toUpper(tokens[1]) != "FROM") return false;
    return tokens.back() == ";";
}

// Parse UPDATE statement
bool parseUpdate(const vector<string>& tokens) {
    if (tokens.size() < 6) return false;
    if (toUpper(tokens[2]) != "SET") return false;
    return tokens.back() == ";";
}

// Main SQL Parser
bool parseSQL(const string& sql) {
    auto tokens = tokenize(sql);
    cout << "\n🔍 Tokens:\n";
    for (const auto& token : tokens) {
        cout << "[" << token << "] ";
    }
    cout << endl;

    if (tokens.empty()) return false;

    string command = toUpper(tokens[0]);
    if (command == "CREATE") return parseCreate(tokens);
    if (command == "INSERT") return parseInsert(tokens);
    if (command == "SELECT") return parseSelect(tokens);
    if (command == "DELETE") return parseDelete(tokens);
    if (command == "UPDATE") return parseUpdate(tokens);

    return false;
}

int main() {
    cout << "📘 SQL Syntax Validator\n";
    cout << "Enter your SQL query (end with a semicolon `;`) or type 'exit;' to quit:\n";

    string input, line;
    while (true) {
        cout << "\nSQL> ";
        getline(cin, line);

        input += line + " ";
        if (line.find(';') == string::npos) continue;

        string trimmed = trim(input);
        if (toUpper(trimmed) == "EXIT;") {
            cout << "Goodbye! 👋\n";
            break;
        }

        if (parseSQL(trimmed)) {
            cout << "✅ Query is VALID\n";
        } else {
            cout << "❌ Query is INVALID\n";
        }

        input.clear();
    }

    return 0;
}
