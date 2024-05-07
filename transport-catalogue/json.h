#pragma once
#include <variant>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Variant = std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool>;
    /* Реализуйте Node, используя std::variant */
    /* template<typename T>
      Node(T data) { data_(data); }*/

    Node() = default;

    Node(std::nullptr_t);

    Node(Array array);

    Node(Dict map);

    Node(int value);

    Node(std::string value);

    Node(double value);

    Node(bool value);

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

    const std::string &AsString() const;

    const Array &AsArray() const;

    const Dict &AsMap() const;

    const Variant &GetData() const;

    bool operator==(const Node &other) const;

    bool operator!=(const Node &other) const;

private:
    Variant data_;
};

class Document {
public:
    Document() = default;
    explicit Document(Node root);

    const Node &GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document &lhs, const Document &rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document &lhs, const Document &rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream &input);

void Print(const Document &doc, std::ostream &output);