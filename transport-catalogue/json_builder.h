#pragma once
#include "json.h"
#include <memory>
#include <string>

class KeyContext;
class DictContext;
class ArrayContext;

class Builder{
public:
    KeyContext Key(std::string key);
    Builder& Value(Node node);

    DictContext StartDict();
    Builder& EndDict();

    ArrayContext StartArray();
    Builder& EndArray();

    Node Build();
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> stack_node_;

    void AddNode(Node&& node);
};

class Context{
public:
    Context(Builder& builder) : builder_(builder){}

    KeyContext Key(std::string key);
    Builder& Value(Node node);
    DictContext StartDict();
    Builder& EndDict();
    ArrayContext StartArray();
    Builder& EndArray();
private:
    Builder& builder_;
};

class KeyContext : public Context{
public:
    KeyContext(Builder& builder) : Context(builder){}
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    DictContext Value(Node node);
};

class DictContext : public Context{
public:
    DictContext(Builder& builder) : Context(builder){}
    Builder& Value(Node node) = delete;
    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class ArrayContext : public Context{
public:
    ArrayContext(Builder& builder) : Context(builder){}
    ArrayContext Value(Node node);
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
};