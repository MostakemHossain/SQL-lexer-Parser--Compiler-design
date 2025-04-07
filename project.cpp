#include<bits/stdc++.h>
using namespace std;

// Token types
enum class TokenType {
    // Keywords
    SELECT, FROM, WHERE, INSERT, INTO, VALUES, UPDATE, SET, DELETE,
    // Operators
    EQUAL, LESS_THAN, GREATER_THAN, LESS_EQUAL, GREATER_EQUAL, NOT_EQUAL,
    // Symbols
    COMMA, SEMICOLON, LEFT_PAREN, RIGHT_PAREN, ASTERISK,
    // Others
    IDENTIFIER, STRING_LITERAL, NUMBER, END_OF_FILE,
    // Logical operators
    AND, OR, NOT
};

// Data types for columns
enum class DataType {
    STRING,
    NUMBER,
    BOOLEAN,
    DATE,
    UNKNOWN
};

// Token structure
struct Token {
    TokenType type;
    std::string lexeme;
    
    Token(TokenType type, const std::string& lexeme) : type(type), lexeme(lexeme) {}
};

// Column structure
struct Column {
    std::string name;
    DataType type;
    
    Column(const std::string& name, DataType type) : name(name), type(type) {}
};

// Lexer class for tokenizing SQL input
class Lexer {
private:
    std::string input;
    size_t position = 0;
    std::unordered_map<std::string, TokenType> keywords;

    char peek() const {
        if (position >= input.length()) return '\0';
        return input[position];
    }

    char advance() {
        if (position >= input.length()) return '\0';
        return input[position++];
    }

    bool isAtEnd() const {
        return position >= input.length();
    }

    bool match(char expected) {
        if (isAtEnd() || peek() != expected) return false;
        position++;
        return true;
    }

    void skipWhitespace() {
        while (!isAtEnd() && std::isspace(peek())) {
            advance();
        }
    }

    Token identifier() {
        size_t start = position - 1;
        while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
            advance();
        }

        std::string text = input.substr(start, position - start);
        std::string upperText = text;
        std::transform(upperText.begin(), upperText.end(), upperText.begin(), ::toupper);

        if (keywords.find(upperText) != keywords.end()) {
            return Token(keywords[upperText], text);
        }

        return Token(TokenType::IDENTIFIER, text);
    }

    Token number() {
        size_t start = position - 1;
        while (!isAtEnd() && std::isdigit(peek())) {
            advance();
        }

        // Look for a decimal point
        if (peek() == '.' && std::isdigit(input[position + 1])) {
            advance(); // Consume the '.'

            while (!isAtEnd() && std::isdigit(peek())) {
                advance();
            }
        }

        return Token(TokenType::NUMBER, input.substr(start, position - start));
    }

    Token string() {
        size_t start = position;
        while (!isAtEnd() && peek() != '\'') {
            advance();
        }

        if (isAtEnd()) {
            throw std::runtime_error("Unterminated string.");
        }

        // The closing '
        advance();

        // Trim the surrounding quotes
        return Token(TokenType::STRING_LITERAL, input.substr(start, position - start - 1));
    }

public:
    Lexer(const std::string& input) : input(input) {
        // Initialize keywords map
        keywords["SELECT"] = TokenType::SELECT;
        keywords["FROM"] = TokenType::FROM;
        keywords["WHERE"] = TokenType::WHERE;
        keywords["INSERT"] = TokenType::INSERT;
        keywords["INTO"] = TokenType::INTO;
        keywords["VALUES"] = TokenType::VALUES;
        keywords["UPDATE"] = TokenType::UPDATE;
        keywords["SET"] = TokenType::SET;
        keywords["DELETE"] = TokenType::DELETE;
        keywords["AND"] = TokenType::AND;
        keywords["OR"] = TokenType::OR;
        keywords["NOT"] = TokenType::NOT;
    }

    Token nextToken() {
        skipWhitespace();

        if (isAtEnd()) return Token(TokenType::END_OF_FILE, "");

        char c = advance();

        if (std::isalpha(c) || c == '_') {
            return identifier();
        }

        if (std::isdigit(c)) {
            return number();
        }

        switch (c) {
            case '\'': return string();
            case ',': return Token(TokenType::COMMA, ",");
            case ';': return Token(TokenType::SEMICOLON, ";");
            case '(': return Token(TokenType::LEFT_PAREN, "(");
            case ')': return Token(TokenType::RIGHT_PAREN, ")");
            case '*': return Token(TokenType::ASTERISK, "*");
            case '=': return Token(TokenType::EQUAL, "=");
            case '<':
                if (match('=')) return Token(TokenType::LESS_EQUAL, "<=");
                if (match('>')) return Token(TokenType::NOT_EQUAL, "<>");
                return Token(TokenType::LESS_THAN, "<");
            case '>':
                if (match('=')) return Token(TokenType::GREATER_EQUAL, ">=");
                return Token(TokenType::GREATER_THAN, ">");
            case '!':
                if (match('=')) return Token(TokenType::NOT_EQUAL, "!=");
                throw std::runtime_error("Unexpected character: !");
        }

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            Token token = nextToken();
            tokens.push_back(token);
            if (token.type == TokenType::END_OF_FILE) break;
        }
        return tokens;
    }

    // Helper method to get all keywords
    std::vector<std::string> getKeywords() const {
        std::vector<std::string> result;
        for (const auto& pair : keywords) {
            result.push_back(pair.first);
        }
        return result;
    }
};

// Helper function to calculate Levenshtein distance between two strings
int levenshteinDistance(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));

    for (size_t i = 0; i <= len1; ++i)
        d[i][0] = i;
    for (size_t j = 0; j <= len2; ++j)
        d[0][j] = j;

    for (size_t i = 1; i <= len1; ++i)
        for (size_t j = 1; j <= len2; ++j)
            d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, 
                              d[i - 1][j - 1] + (s1[i - 1] != s2[j - 1]) });

    return d[len1][len2];
}

// Helper function to find the closest keyword match
std::string findClosestKeyword(const std::string& word, const std::vector<std::string>& keywords) {
    std::string closest;
    int minDistance = std::numeric_limits<int>::max();

    for (const auto& keyword : keywords) {
        int distance = levenshteinDistance(word, keyword);
        if (distance < minDistance) {
            minDistance = distance;
            closest = keyword;
        }
    }

    // Only suggest if the distance is small enough (relative to word length)
    if (minDistance <= std::max(2, static_cast<int>(word.length() / 3))) {
        return closest;
    }
    return "";
}

// Helper function to infer data type from column name
DataType inferDataTypeFromColumnName(const std::string& columnName) {
    std::string lowerName = columnName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // Common string column names
    static const std::vector<std::string> stringColumns = {
        "name", "firstname", "lastname", "email", "address", "city", "state", 
        "country", "description", "title", "username", "password", "phone", 
        "status", "type", "color", "url", "code"
    };
    
    // Common numeric column names
    static const std::vector<std::string> numericColumns = {
        "id", "age", "count", "amount", "price", "quantity", "total", "number",
        "size", "width", "height", "weight", "duration", "score", "rating"
    };
    
    // Check if the column name matches or contains any of the string column patterns
    for (const auto& pattern : stringColumns) {
        if (lowerName == pattern || lowerName.find(pattern) != std::string::npos) {
            return DataType::STRING;
        }
    }
    
    // Check if the column name matches or contains any of the numeric column patterns
    for (const auto& pattern : numericColumns) {
        if (lowerName == pattern || lowerName.find(pattern) != std::string::npos) {
            return DataType::NUMBER;
        }
    }
    
    // Default to unknown type
    return DataType::UNKNOWN;
}

// Helper function to check if a value matches the expected data type
bool isValueTypeValid(TokenType valueType, DataType expectedType) {
    switch (expectedType) {
        case DataType::STRING:
            return valueType == TokenType::STRING_LITERAL || valueType == TokenType::IDENTIFIER;
        case DataType::NUMBER:
            return valueType == TokenType::NUMBER || valueType == TokenType::IDENTIFIER;
        case DataType::BOOLEAN:
            // Booleans can be represented as strings, numbers, or identifiers
            return true;
        case DataType::DATE:
            // Dates are typically represented as strings
            return valueType == TokenType::STRING_LITERAL;
        case DataType::UNKNOWN:
            // If type is unknown, accept any value
            return true;
        default:
            return true;
    }
}

// Helper function to get data type name as string
std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::STRING: return "string";
        case DataType::NUMBER: return "number";
        case DataType::BOOLEAN: return "boolean";
        case DataType::DATE: return "date";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

// Parser class for validating SQL grammar
class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;
    std::vector<std::string> keywords;

    Token peek() const {
        return tokens[current];
    }

    Token previous() const {
        return tokens[current - 1];
    }

    bool isAtEnd() const {
        return peek().type == TokenType::END_OF_FILE;
    }

    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool match(std::initializer_list<TokenType> types) {
        for (auto type : types) {
            if (match(type)) return true;
        }
        return false;
    }

    void consume(TokenType type, const std::string& message) {
        if (check(type)) {
            advance();
            return;
        }

        throw std::runtime_error(message);
    }

    // Grammar rules
    void statement() {
        if (match(TokenType::SELECT)) {
            selectStatement();
        } else if (match(TokenType::INSERT)) {
            insertStatement();
        } else if (match(TokenType::UPDATE)) {
            updateStatement();
        } else if (match(TokenType::DELETE)) {
            deleteStatement();
        } else {
            // Check if the current token is an identifier that might be a misspelled keyword
            if (peek().type == TokenType::IDENTIFIER) {
                std::string token = peek().lexeme;
                std::transform(token.begin(), token.end(), token.begin(), ::toupper);
                
                std::string suggestion = findClosestKeyword(token, keywords);
                if (!suggestion.empty()) {
                    throw std::runtime_error("Unknown keyword '" + peek().lexeme + 
                                           "'. Did you mean '" + suggestion + "'?");
                }
            }
            
            throw std::runtime_error("Expected a SQL statement.");
        }
    }

    void selectStatement() {
        // SELECT column1, column2, ... FROM table_name WHERE condition;
        columnList();
        consume(TokenType::FROM, "Expected 'FROM' after SELECT columns.");
        tableList();
        
        if (match(TokenType::WHERE)) {
            condition();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' at the end of SELECT statement.");
    }

    void insertStatement() {
        // INSERT INTO table_name (column1, column2, ...) VALUES (value1, value2, ...);
        consume(TokenType::INTO, "Expected 'INTO' after INSERT.");
        consume(TokenType::IDENTIFIER, "Expected table name after INTO.");
        
        std::vector<Column> columns;
        int columnCount = 0;
        
        if (match(TokenType::LEFT_PAREN)) {
            columns = parseColumnList();
            columnCount = columns.size();
            consume(TokenType::RIGHT_PAREN, "Expected ')' after column list.");
        }
        
        consume(TokenType::VALUES, "Expected 'VALUES' after table name or column list.");
        consume(TokenType::LEFT_PAREN, "Expected '(' after VALUES.");
        
        // Parse and validate values
        std::vector<TokenType> valueTypes = parseValueList();
        int valueCount = valueTypes.size();
        
        consume(TokenType::RIGHT_PAREN, "Expected ')' after value list.");
        
        // Check if column count matches value count
        if (columnCount > 0 && columnCount != valueCount) {
            throw std::runtime_error("Column count (" + std::to_string(columnCount) + 
                                    ") does not match value count (" + std::to_string(valueCount) + ").");
        }
        
        // Check if value types match column types
        for (size_t i = 0; i < columns.size() && i < valueTypes.size(); ++i) {
            if (!isValueTypeValid(valueTypes[i], columns[i].type)) {
                throw std::runtime_error("Type mismatch for column '" + columns[i].name + 
                                        "'. Expected " + dataTypeToString(columns[i].type) + 
                                        " but got " + (valueTypes[i] == TokenType::STRING_LITERAL ? "string" : 
                                                      valueTypes[i] == TokenType::NUMBER ? "number" : "identifier") + ".");
            }
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' at the end of INSERT statement.");
    }

    void updateStatement() {
        // UPDATE table_name SET column1 = value1, column2 = value2, ... WHERE condition;
        consume(TokenType::IDENTIFIER, "Expected table name after UPDATE.");
        consume(TokenType::SET, "Expected 'SET' after table name.");
        assignmentList();
        
        if (match(TokenType::WHERE)) {
            condition();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' at the end of UPDATE statement.");
    }

    void deleteStatement() {
        // DELETE FROM table_name WHERE condition;
        consume(TokenType::FROM, "Expected 'FROM' after DELETE.");
        consume(TokenType::IDENTIFIER, "Expected table name after FROM.");
        
        if (match(TokenType::WHERE)) {
            condition();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' at the end of DELETE statement.");
    }

    void columnList() {
        if (match(TokenType::ASTERISK)) {
            return;
        }
        
        do {
            consume(TokenType::IDENTIFIER, "Expected column name.");
        } while (match(TokenType::COMMA));
    }

    std::vector<Column> parseColumnList() {
        std::vector<Column> columns;
        
        if (match(TokenType::ASTERISK)) {
            columns.push_back(Column("*", DataType::UNKNOWN));
            return columns;
        }
        
        do {
            Token columnToken = peek();
            consume(TokenType::IDENTIFIER, "Expected column name.");
            
            // Infer data type from column name
            DataType dataType = inferDataTypeFromColumnName(columnToken.lexeme);
            columns.push_back(Column(columnToken.lexeme, dataType));
            
        } while (match(TokenType::COMMA));
        
        return columns;
    }

    int countColumnList() {
        if (match(TokenType::ASTERISK)) {
            return 1; // * represents all columns
        }
        
        int count = 1; // Start with 1 for the first column
        consume(TokenType::IDENTIFIER, "Expected column name.");
        
        while (match(TokenType::COMMA)) {
            consume(TokenType::IDENTIFIER, "Expected column name after comma.");
            count++;
        }
        
        return count;
    }

    void tableList() {
        do {
            consume(TokenType::IDENTIFIER, "Expected table name.");
        } while (match(TokenType::COMMA));
    }

    void valueList() {
        do {
            if (match({TokenType::STRING_LITERAL, TokenType::NUMBER, TokenType::IDENTIFIER})) {
                // Valid value
            } else {
                throw std::runtime_error("Expected a value (string, number, or identifier).");
            }
        } while (match(TokenType::COMMA));
    }

    std::vector<TokenType> parseValueList() {
        std::vector<TokenType> valueTypes;
        
        do {
            if (match(TokenType::STRING_LITERAL)) {
                valueTypes.push_back(TokenType::STRING_LITERAL);
            } else if (match(TokenType::NUMBER)) {
                valueTypes.push_back(TokenType::NUMBER);
            } else if (match(TokenType::IDENTIFIER)) {
                valueTypes.push_back(TokenType::IDENTIFIER);
            } else {
                throw std::runtime_error("Expected a value (string, number, or identifier).");
            }
        } while (match(TokenType::COMMA));
        
        return valueTypes;
    }

    int countValueList() {
        int count = 1; // Start with 1 for the first value
        
        if (!match({TokenType::STRING_LITERAL, TokenType::NUMBER, TokenType::IDENTIFIER})) {
            throw std::runtime_error("Expected a value (string, number, or identifier).");
        }
        
        while (match(TokenType::COMMA)) {
            if (!match({TokenType::STRING_LITERAL, TokenType::NUMBER, TokenType::IDENTIFIER})) {
                throw std::runtime_error("Expected a value after comma.");
            }
            count++;
        }
        
        return count;
    }

    void assignmentList() {
        do {
            consume(TokenType::IDENTIFIER, "Expected column name.");
            consume(TokenType::EQUAL, "Expected '=' after column name.");
            
            if (!match({TokenType::STRING_LITERAL, TokenType::NUMBER, TokenType::IDENTIFIER})) {
                throw std::runtime_error("Expected a value (string, number, or identifier).");
            }
        } while (match(TokenType::COMMA));
    }

    void condition() {
        expression();
    }

    void expression() {
        term();
        
        while (match({TokenType::AND, TokenType::OR})) {
            term();
        }
    }

    void term() {
        consume(TokenType::IDENTIFIER, "Expected column name in condition.");
        
        if (!match({TokenType::EQUAL, TokenType::LESS_THAN, TokenType::GREATER_THAN, 
                   TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL, TokenType::NOT_EQUAL})) {
            throw std::runtime_error("Expected a comparison operator.");
        }
        
        if (!match({TokenType::STRING_LITERAL, TokenType::NUMBER, TokenType::IDENTIFIER})) {
            throw std::runtime_error("Expected a value (string, number, or identifier).");
        }
    }

public:
    Parser(const std::vector<Token>& tokens, const std::vector<std::string>& keywords) 
        : tokens(tokens), keywords(keywords) {}

    void parse() {
        statement();
        
        // After parsing a statement, we should be at the end of file
        // or at the beginning of another statement
        if (!isAtEnd() && peek().type != TokenType::SEMICOLON) {
            throw std::runtime_error("Unexpected tokens after statement.");
        }
        
        std::cout << "SQL query is valid." << std::endl;
    }
};

// SQL Validator class that combines lexer and parser
class SQLValidator {
public:
    static bool validate(const std::string& sql) {
        try {
            Lexer lexer(sql);
            std::vector<Token> tokens = lexer.tokenize();
            
            // Print tokens for debugging
            std::cout << "Tokens:" << std::endl;
            for (const auto& token : tokens) {
                if (token.type != TokenType::END_OF_FILE) {
                    std::cout << "  Type: " << static_cast<int>(token.type) << ", Lexeme: '" << token.lexeme << "'" << std::endl;
                }
            }
            
            // Get keywords for suggestion
            std::vector<std::string> keywords = lexer.getKeywords();
            
            Parser parser(tokens, keywords);
            parser.parse();
            return true;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    std::string sql;
    std::cout << "Enter SQL query (or 'exit' to quit):" << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, sql);
        
        if (sql == "exit") break;
        
        std::cout << "Validating: " << sql << std::endl;
        SQLValidator::validate(sql);
        std::cout << std::endl;
    }
    
    return 0;
}