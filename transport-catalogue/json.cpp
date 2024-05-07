#include "json.h"
#include <iomanip>

using namespace std;
namespace {
    Node LoadNode(std::istream &input);

    Node LoadString(std::istream &input);

    std::string LoadLiteral(std::istream &input) {
        std::string s;

        while (std::isalpha(input.peek())) {
            s.push_back(static_cast<char>(input.get()));
        }
        return s;
    }

    Node LoadArray(std::istream &input) {
        std::vector<Node> result;

        for (char c; input >> c && c != ']';) {

            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        if (!input) {
            throw ParsingError("Array parsing error"s);
        }

        return Node(std::move(result));
    }

    Node LoadDict(std::istream &input) {
        Dict dict;

        for (char c; input >> c && c != '}';) {

            if (c == '"') {
                std::string key = LoadString(input).AsString();

                if (input >> c && c == ':') {

                    if (dict.find(key) != dict.end()) {
                        throw ParsingError("Duplicate key '"s + key + "' have been found");
                    }

                    dict.emplace(std::move(key), LoadNode(input));

                } else {

                    throw ParsingError(": is expected but '"s + c + "' has been found"s);
                }

            } else if (c != ',') {
                throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
            }
        }

        if (!input) {
            throw ParsingError("Dictionary parsing error"s);
        }

        return Node(std::move(dict));
    }

    Node LoadString(std::istream &input) {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;

        while (true) {

            if (it == end) {
                throw ParsingError("String parsing error");
            }

            const char ch = *it;
            if (ch == '"') {
                ++it;
                break;

            } else if (ch == '\\') {
                ++it;

                if (it == end) {
                    throw ParsingError("String parsing error");
                }

                const char escaped_char = *(it);
                switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);

                }

            } else if (ch == '\n' || ch == '\r') {
                throw ParsingError("Unexpected end of line"s);

            } else {
                s.push_back(ch);

            }

            ++it;
        }

        return Node(std::move(s));
    }

    Node LoadBool(std::istream &input) {
        const auto s = LoadLiteral(input);

        if (s == "true"sv) {
            return Node{true};

        } else if (s == "false"sv) {
            return Node{false};

        } else {
            throw ParsingError("Failed to parse '"s + s + "' as bool"s);
        }
    }

    Node LoadNull(std::istream &input) {
        if (auto literal = LoadLiteral(input); literal == "null"sv) {
            return Node{nullptr};

        } else {
            throw ParsingError("Failed to parse '"s + literal + "' as null"s);
        }
    }

    Node LoadNumber(std::istream &input) {
        std::string parsed_num;

        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());

            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }

            while (std::isdigit(input.peek())) {
                read_char();
            }

        };

        if (input.peek() == '-') {
            read_char();

        }

        if (input.peek() == '0') {
            read_char();

        } else {
            read_digits();

        }

        bool is_int = true;
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;

        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();

            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }

            read_digits();
            is_int = false;
        }

        try {

            if (is_int) {

                try {
                    return std::stoi(parsed_num);
                } catch (...) {

                }
            }

            return std::stod(parsed_num);

        } catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }

    }

    Node LoadNode(std::istream &input) {
        char c;

        if (!(input >> c)) {
            throw ParsingError("Unexpected EOF"s);
        }

        switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
        }
    }
}  // namespace

//CLASS NODE

Node::Node(std::nullptr_t) : data_(nullptr) {}

Node::Node(Array array) : data_(std::move(array)) {}

Node::Node(Dict map) : data_(std::move(map)) {}

Node::Node(int value) : data_(value) {}

Node::Node(std::string value) : data_(std::move(value)) {}

Node::Node(double value) : data_(value) {}

Node::Node(bool value) : data_(value) {}

bool Node::IsInt() const {
    return std::holds_alternative<int>(data_);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(data_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(data_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(data_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(data_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(data_);
}

int Node::AsInt() const {
    if (IsInt()) {
        return get<int>(data_);
    } else {
        throw std::logic_error("not int!");
    }
}

bool Node::AsBool() const {
    if (IsBool()) {
        return get<bool>(data_);
    } else {
        throw std::logic_error("not bool!");
    }
}

double Node::AsDouble() const {
    if (IsInt()) {
        return get<int>(data_) * 1.0;
    } else if (IsDouble()) {
        return get<double>(data_);
    } else {
        throw std::logic_error("not double!");
    }
}

const std::string &Node::AsString() const {
    if (IsString()) {
        return get<std::string>(data_);
    } else {
        throw std::logic_error("not string!");
    }
}

const Array &Node::AsArray() const {
    if (IsArray()) {
        return get<Array>(data_);
    } else {
        throw std::logic_error("not array!");
    }
}

const Dict &Node::AsMap() const {
    if (IsMap()) {
        return get<Dict>(data_);
    } else {
        throw std::logic_error("not map!");
    }
}

const Node::Variant &Node::GetData() const { return data_; }

bool Node::operator==(const Node &other) const {
    return data_ == other.data_;
}

bool Node::operator!=(const Node &other) const {
    return !(*this == other);
}

//-----------------
Document::Document(Node root)
        : root_(std::move(root)) {
}

const Node &Document::GetRoot() const {
    return root_;
}

Document Load(istream &input) {
    return Document{LoadNode(input)};
}

class PrintContext {
public:
    PrintContext(ostream &out, int step = 4, int indent = 0) : out(out), step(step), indent(indent) {}

    PrintContext Indented() const { return PrintContext(out, step, step + indent); }

    void PrintIndent() const { out << std::setw(indent) << ' '; }

    void PrintNode(const Node node) {
        visit([this](const auto &value) { PrintValue(value); }, node.GetData());
    }

private:
    void PrintValue(const bool value) { out << boolalpha << value; }

    void PrintValue(const int value) { out << value; }

    void PrintValue(const double value) { out << value; }

    void PrintValue(const nullptr_t) { out << "null"sv; }

    void PrintValue(const std::string &value) {
        out << '"';
        for (const char c: value) {
            switch (c) {
                case '\t':
                    out << "\\t"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '\r':
                    out << "\\r"sv;
                    break;
                case '"':
                case '\\':
                    out << '\\';
                    [[fallthrough]];
                default:
                    out << c;
            }
        }
        out << '"';
    }

    void PrintValue(const Array &value) {
        PrintContext nested_ctx = Indented();

        out << "[\n"sv;
        if (!value.empty()) {
            auto it = value.begin();
            nested_ctx.PrintIndent();
            nested_ctx.PrintNode(*it);
            while (++it != value.end()) {
                out << ",\n"sv;
                nested_ctx.PrintIndent();
                nested_ctx.PrintNode(*it);
            }
        }
        out << '\n';
        PrintIndent();
        out << ']';
    }

    void PrintValue(const Dict &value) {
        PrintContext nested_ctx = Indented();

        out << "{\n"sv;
        if (!value.empty()) {
            auto it = value.begin();
            nested_ctx.PrintIndent();
            nested_ctx.PrintValue(it->first); // std::string
            out << ": "sv;
            nested_ctx.PrintNode(it->second); // Node
            while (++it != value.end()) {
                out << ",\n"sv;
                nested_ctx.PrintIndent();
                nested_ctx.PrintValue(it->first);
                out << ": "sv;
                nested_ctx.PrintNode(it->second);
            }
        }
        out << '\n';
        PrintIndent();
        out << '}';
    }

    std::ostream &out;
    int step;
    int indent;
};

void Print(const Document &doc, std::ostream &out) {
    PrintContext context(out);
    context.PrintNode(doc.GetRoot());
}