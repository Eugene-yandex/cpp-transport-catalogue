#pragma once

#include <iostream>
#include <map>
#include <variant>
#include <string>
#include <vector>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };


    class Node {
    public:
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
        Node() = default;
        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(double value);
        Node(bool value);
        Node(std::string value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const;

    private:
        Value value_;
    };

    bool operator==(const Node& left, const Node& right);
    bool operator!=(const Node& left, const Node& right);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& left, const Document& right);
    bool operator!=(const Document& left, const Document& right);

    Document Load(std::istream& input);

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // ¬озвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintString(const std::string& value, std::ostream& out);

    void PrintNode(const Node& node, const PrintContext& ctx);

    template <typename Value>
    inline void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    inline void PrintValue(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    inline void PrintValue(const std::nullptr_t&, const PrintContext& ctx) {
        using namespace std::literals;
        ctx.out << "null"sv;
    }

    inline void PrintValue(bool value, const PrintContext& ctx) {
        ctx.out << std::boolalpha << value;
    }

    inline  void PrintValue(const Array& value, const PrintContext& ctx) {
        auto& out = ctx.out;
        out << "[\n";
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& val : value) {
            if (!first) {
                out << ",\n";
            }
            inner_ctx.PrintIndent();
            PrintNode(val, inner_ctx);
            first = false;
        }
        out.put('\n');
        ctx.PrintIndent();
        out << ']';
    }

    inline  void PrintValue(const Dict& value, const PrintContext& ctx) {
        auto& out = ctx.out;
        out << "{\n";
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [str, val] : value) {
            if (!first) {
                out << ",\n";
            }
            inner_ctx.PrintIndent();
            PrintString(str, ctx.out);
            out << ": ";
            PrintNode(val, inner_ctx);
            first = false;
        }
        out.put('\n');
        ctx.PrintIndent();
        out << '}';
    }


    void Print(const Document& doc, std::ostream& output);

}  // namespace json