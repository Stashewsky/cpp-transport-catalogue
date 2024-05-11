#include "json_builder.h"

KeyContext Builder::Key(std::string key){
    if(stack_node_.empty()){
        throw std::logic_error("Can not create key!");
    }

    if(stack_node_.back()->IsMap()){
        stack_node_.emplace_back(std::move(std::make_unique<Node>(std::move(key))));
    }
    return KeyContext(*this);
}

Builder& Builder::Value(Node node){
    AddNode(std::move(node));
    return *this;
}

DictContext Builder::StartDict(){
    stack_node_.emplace_back(std::move(std::make_unique<Node>(Dict())));
    return DictContext(*this);
}

Builder& Builder::EndDict(){
    if(stack_node_.empty()){
        throw std::logic_error("Dict does not exist! Stack is empty!");
    }

    Node dict = *stack_node_.back();
    if(!dict.IsMap()){
        throw std::logic_error("Current object is not a dictionary!");
    }
    stack_node_.pop_back();
    AddNode(std::move(dict));
    return *this;
}

ArrayContext Builder::StartArray(){
    stack_node_.emplace_back(std::move(std::make_unique<Node>(Array())));
    return ArrayContext(*this);
}

Builder& Builder::EndArray(){
    if(stack_node_.empty()){
        throw std::logic_error("Array does not exist! Stack is empty!");
    }
    Node arr = *stack_node_.back();
    if(!arr.IsArray()){
        throw std::logic_error("Current object is not array!");
    }
    stack_node_.pop_back();
    AddNode(std::move(arr));
    return *this;
}

Node Builder::Build(){
    if(root_.IsNull()){
        throw std::logic_error("This root is empty!");
    }
    if(!stack_node_.empty()){
        throw std::logic_error("Stack is not empty! Root can't be constructed!");
    }
    return root_;
}

void Builder::AddNode(Node&& node) {
    if(stack_node_.empty()){
        if(!root_.IsNull()){
            throw std::logic_error("Root is not empty!");
        }
        root_ = std::move(node);
        return;
    }else{
        if(!stack_node_.back()->IsArray() && !stack_node_.back()->IsString()){
            throw std::logic_error("unable to create node");
        }

        if(stack_node_.back()->IsArray()){
            Array arr = stack_node_.back()->AsArray();
            arr.emplace_back(std::move(node));

            stack_node_.pop_back();
            stack_node_.emplace_back(std::move(std::make_unique<Node>(std::move(arr))));
            return;
        }

        if(stack_node_.back()->IsString()){
            std::string str = stack_node_.back()->AsString();
            stack_node_.pop_back();

            if (stack_node_.back()->IsMap()) {
                Dict dictionary = stack_node_.back()->AsMap();
                dictionary.emplace(std::move(str), node);

                stack_node_.pop_back();
                auto dictionary_ptr = std::make_unique<Node>(dictionary);
                stack_node_.emplace_back(std::move(dictionary_ptr));
            }

            return;
        }
    }
}

KeyContext Context::Key(std::string key){
    return builder_.Key(std::move(key));
}
Builder& Context::Value(Node value){
    return builder_.Value(std::move(value));
}
DictContext Context::StartDict(){
    return DictContext(builder_.StartDict());
}
Builder& Context::Context::EndDict(){
    return builder_.EndDict();
}
ArrayContext Context::StartArray(){
    return ArrayContext(builder_.StartArray());
}
Builder& Context::EndArray(){
    return builder_.EndArray();
}

DictContext KeyContext::Value(Node node){
    return DictContext(Context::Value(std::move(node)));
}

ArrayContext ArrayContext::Value(Node node){
    return ArrayContext(Context::Value(std::move(node)));
}