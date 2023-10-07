#include "json.h"
#include <typeinfo>

using namespace std;

namespace json
{

    namespace
    {

        Node LoadNode(istream &input);

        Node LoadConst(istream &input)
        {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    break;
                }
                const char c = *it;
                if (!isalpha(c))
                {
                    break;
                }
                else
                {
                    s.push_back(c);
                }
                ++it;
            }
            if (s == "null"s || s == "nullptr")
            {
                return Node();
            }
            else if (s == "true"s)
            {
                return Node(true);
            }
            else if (s == "false"s)
            {
                return Node(false);
            }
            else
            {
                throw ParsingError("Unrecognized element");
            }
        }

        Node LoadNumber(istream &input)
        {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char]
            {
                if (!std::isdigit(input.peek()))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            if (input.peek() == '0')
            {
                read_char();
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    try
                    {
                        return std::stoi(parsed_num);
                    }
                    catch (...)
                    {
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream &input)
        {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end)
                    {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char)
                    {
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
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(move(s));
        }

        Node LoadArray(istream &input)
        {
            Array result;
            bool bad_json = true;
            for (char c; input >> c;)
            {
                if (c == ']')
                {
                    bad_json = false;
                    break;
                }
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (bad_json)
            {
                throw ParsingError("bad array section"s);
            }
            return Node(move(result));
        }

        Node LoadDict(istream &input)
        {
            Dict result;
            bool bad_json = true;
            for (char c; input >> c;)
            {
                if (c == '}')
                {
                    bad_json = false;
                    break;
                }
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if (bad_json)
            {
                throw ParsingError("bad map section"s);
            }
            return Node(move(result));
        }

        Node LoadNode(istream &input)
        {
            char c;
            input >> c;

            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f' || c == 'n')
            {
                input.putback(c);
                return LoadConst(input);
            }
            else
            {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    } // namespace

    void PrintContext::MakeIndent() const
    {
        for (int step = 0; step < indent_; step++)
        {
            out << ' ';
        }
    }
    void PrintContext::IndentMore()
    {
        indent_ += indent_step_;
    }
    void PrintContext::IndentLess()
    {
        indent_ -= indent_step_;
        if (indent_ < 0)
        {
            indent_ = 0;
        }
    }
    int PrintContext::IndentLevel() const
    {
        return indent_ / indent_step_;
    }

    Node::Node(bool value)
        : keep_(std::move(Keep(value)))
    {
    }
    Node::Node(int value)
        : keep_(std::move(Keep(value)))
    {
    }

    Node::Node(double value)
        : keep_(std::move(Keep(value)))
    {
    }

    Node::Node(string value)
        : keep_(std::move(Keep(value)))
    {
    }

    Node::Node(Array array)
        : keep_(std::move(Keep(array)))
    {
    }

    Node::Node(Dict map)
        : keep_(std::move(Keep(map)))
    {
    }

    bool Node::IsNull() const
    {
        return std::holds_alternative<std::nullptr_t>(keep_);
    }

    bool Node::IsInt() const
    {
        // альтернативная версия проверки- пока оставлю чтобы не забыть, хочу разобраться в тонкостях
        // bool out;
        // std::visit([&out](auto &value)
        //            {
        //     using T = std::decay_t<decltype(value)>;
        //     if (std::is_same_v<T, int>){
        //         out=true;
        //     } else{
        //         out=false;
        //     } },
        //            keep_);
        // return out;
        return std::holds_alternative<int>(keep_);
    }

    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(keep_);
    }

    bool Node::IsDouble() const
    {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(keep_);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(keep_);
    }
    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(keep_);
    }
    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(keep_);
    }

    int Node::AsInt() const
    {
        if (!IsInt())
        {
            throw invalid_argument("type is not int");
        }
        else
        {
            return std::get<int>(keep_);
        }
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw invalid_argument("type is not bool");
        }
        else
        {
            return std::get<bool>(keep_);
        }
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
        {
            throw invalid_argument("value is not a digit");
        }
        else
        {
            return IsInt() ? std::get<int>(keep_) : std::get<double>(keep_);
        }
    }

    const std::string &Node::AsString() const
    {
        if (!IsString())
        {
            throw invalid_argument("type is not std::string");
        }
        else
        {
            return std::get<std::string>(keep_);
        }
    }

    const Array &Node::AsArray() const
    {
        if (!IsArray())
        {
            throw invalid_argument("type is not Array");
        }
        else
        {
            return std::get<Array>(keep_);
        }
    }

    const Dict &Node::AsMap() const
    {
        if (!IsMap())
        {
            throw invalid_argument("type is not Dict");
        }
        else
        {
            return std::get<Dict>(keep_);
        }
    }

    bool operator==(const Node &left, const Node &right)
    {

        return left.GetValue() == right.GetValue();
    }

    bool operator!=(const Node &left, const Node &right)
    {
        return !(left == right);
    }

    void PrintValue(std::nullptr_t, PrintContext &ctx)
    {
        ctx.out << "null"sv;
    }
    void PrintValue(const bool value, PrintContext &ctx)
    {

        ctx.out << std::boolalpha << value;
    }

    void PrintValue(const std::string &value, PrintContext &ctx)
    {
        std::string out_str = "\"";
        for (char ch : value)
        {
            switch (ch)
            {
            case '"':
                out_str += "\\\""sv;
                break;
            case '\\':
                out_str += "\\\\"sv;
                break;
            case '\n':
                out_str += "\\n"sv;
                break;
            case '\r':
                out_str += "\\r"sv;
                break;
            default:
                out_str.push_back(ch);
            }
        }
        out_str.push_back('\"');
        ctx.out << out_str;
    }

    void PrintValue(const Array &arr, PrintContext &ctx)
    {
        ctx.out << "["sv;
        auto it = arr.begin();
        if (arr.size() > 0)
        {
            PrintNode(*it, ctx);
            ++it;
        }
        while (it != arr.end())
        {
            ctx.out << ", "sv;
            PrintNode(*it, ctx);
            ++it;
        }
        ctx.out << "]"sv;
    }

    void PrintValue(const Dict &dict, PrintContext &ctx)
    {
        ctx.out << "{"sv;
        auto it = dict.begin();
        if (dict.size() > 0)
        {
            PrintValue(it->first, ctx);
            ctx.out << ": "sv;
            PrintNode(it->second, ctx);
            ++it;
        }
        while (it != dict.end())
        {
            ctx.out << ","sv;
            PrintValue(it->first, ctx);
            ctx.out << ":";
            PrintNode(it->second, ctx);
            ++it;
        }
        ctx.out << "}"sv;
    }

    void PrintNode(const Node &node, PrintContext &ctx)
    {
        std::visit(
            [&ctx](const auto &value)
            { PrintValue(value, ctx); },
            node.GetValue());
    }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    bool operator==(const Document &left, const Document &right)
    {
        return left.GetRoot() == right.GetRoot();
    }
    bool operator!=(const Document &left, const Document &right)
    {
        return !(left == right);
    }

    void Print(const Document &doc, std::ostream &output)
    {
        PrintContext ctx(output);
        PrintNode(doc.GetRoot(), ctx);
    }

} // namespace json