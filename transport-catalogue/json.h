#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json
{

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node
    {
    public:
        /* Реализуйте Node, используя std::variant */
        using Keep = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;
        Node()=default;
        Node(std::nullptr_t)
        :Node(){};
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);
        Node(Array array);
        Node(Dict map);

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;     // Возвращает true, если в Node хранится int либо double.
        bool IsPureDouble() const; // Возвращает true, если в Node хранится double.
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const; // Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
        const std::string &AsString() const;
        const Array &AsArray() const;
        const Dict &AsMap() const;

        const Keep &GetValue() const { return keep_; }

    private:
        Keep keep_;
    };

    bool operator ==(const Node& left, const Node& right);

    bool operator !=(const Node& left, const Node& right);

    template <typename Value>
    void PrintValue(const Value &value, std::ostream &out)
    {
        out << value;
    }
    void PrintValue(std::nullptr_t, std::ostream &out);
    void PrintValue(const bool value, std::ostream &out);
    void PrintValue(const std::string &value, std::ostream &out);
    void PrintValue(const Array &array, std::ostream &out);
    void PrintValue(const Dict &array, std::ostream &out);
    void PrintNode(const Node &node, std::ostream &out);

    class Document
    {
    public:
        Document(Node root);

        const Node &GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

} // namespace json