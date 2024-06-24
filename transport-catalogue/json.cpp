#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            if (!input.get(c)) {
                throw ParsingError("No data for Array");
            }
            else {
                input.putback(c);
                for (; input >> c && c != ']';) {
                    if (c != ',') {
                        input.putback(c);
                    }
                    result.push_back(LoadNode(input));
                }
            }
            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == '-' || isdigit(c) || c == 'e' || c == 'E' || c == '.' || c == '+') {
                    line += c;
                }
                else if (c == ' ' || c == ',' || c == '}' || c == ']' || c == '\n' || c == '\t') {
                    input.putback(c);
                    break;
                }
                else {
                    throw ParsingError("Not found char of parce line");
                }
            }
            if (line.empty()) {
                throw ParsingError("No data for parce");
            }
            auto ch1 = line.find("e");
            auto ch2 = line.find("E");
            auto ch3 = line.find(".");
            if (ch1 != std::string::npos || ch2 != std::string::npos || ch3 != std::string::npos) {
                return Node(stod(line));
            }
            return Node(stoi(line));
        }

        Node LoadBool(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == ' ' || c == ',' || c == '}' || c == ']' || c == '\n' || c == '\t') {
                    input.putback(c);
                    break;
                }
                line += c;
            }
            if (line == "true"s) {
                return Node(true);
            }
            else if (line == "false"s) {
                return Node(false);
            }
            else {
                throw ParsingError(line + " no bool"s);
            }
        }

        Node LoadNull(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == ' ' || c == ',' || c == '}' || c == ']' || c == '\n' || c == '\t') {
                    input.putback(c);
                    break;
                }
                line += c;
            }
            if (line == "null"s) {
                return Node{};
            }
            else {
                throw ParsingError(line + " no null"s);
            }
        }

        Node LoadString(std::istream& input)
        {
            std::string result;
            char c;
            while (input.get(c)) {
                if (c == '"') {
                    return Node(std::move(result));
                }
                else if (c == '\\') {
                    input.get(c);
                    switch (c) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    default:
                        result += c;
                        break;
                    }
                }
                else
                {
                    result += c;
                }
            }
            throw ParsingError("Missing \" at the end of string");
        }

        Node LoadDict(istream& input) {
            Dict dict;
            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                }
                else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }

            return Node(move(dict));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(std::nullptr_t)
        : value_(nullptr) {

    }
    Node::Node(Array array)
        : value_(move(array)) {
    }

    Node::Node(Dict map)
        : value_(move(map)) {
    }

    Node::Node(int value)
        : value_(value) {
    }

    Node::Node(string value)
        : value_(move(value)) {
    }

    Node::Node(double value)
        : value_(value) {

    }

    Node::Node(bool value)
        : value_(value) {

    }
    const Node::Value& Node::GetValue() const {
        return value_;
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return std::holds_alternative<string>(value_);
    }
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Invalid type");
        }
        else {
            return std::get<int>(value_);
        }
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Invalid type");
        }
        else {
            return std::get<bool>(value_);
        }
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw  std::logic_error("Invalid type");
        }
        else {
            if (IsInt()) {
                return static_cast<double>(std::get<int>(value_));
            }
            else {
                return std::get<double>(value_);
            }
        }

    }
    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw  std::logic_error("Invalid type");
        }
        else {
            return std::get<string>(value_);
        }
    }
    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw  std::logic_error("Invalid type");
        }
        else {
            return std::get<Array>(value_);
        }
    }
    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw  std::logic_error("Invalid type");
        }
        else {
            return std::get<Dict>(value_);
        }
    }

    bool operator==(const Node& left, const Node& right) {
        return left.GetValue() == right.GetValue();
    }
    bool operator!=(const Node& left, const Node& right) {
        return !(left == right);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator==(const Document& left, const Document& right) {
        return left.GetRoot() == right.GetRoot();
    }

    bool operator!=(const Document& left, const Document& right) {
        return !(left == right);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out << "\""sv;
        for (const char ch : value) {
            switch (ch) {
            case '\n':
                out << R"(\n)";
                break;
            case '\r':
                out << R"(\r)";
                break;
            case '\"':
                out << R"(\")";
                break;
            case '\t':
                out << R"(\t)";
                break;
            case '\\':
                out << R"(\\)";
                break;
            default:
                out << ch;
                break;
            }
        }
        out << "\""sv;
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }


    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json