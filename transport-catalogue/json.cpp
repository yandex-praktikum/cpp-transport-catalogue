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
                //if (c == ' ')
                if(!isalpha(c))
                {
                    break;
                }
                else
                {
                    s.push_back(c);
                }
                ++it;
            }
            if (s == "null"s ||s=="nullptr")
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

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
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
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return std::stoi(parsed_num);
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
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
                throw ParsingError("bad array section"s);
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
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, int>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsDouble() const
    { // Возвращает true, если в Node хранится int либо double.
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if ((std::is_same_v<T, int>) || ((std::is_same_v<T, double>))){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsPureDouble() const
    {
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, double>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsBool() const
    {
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, bool>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsString() const
    {
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, std::string>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsArray() const
    {
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, Array>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
    }
    bool Node::IsMap() const
    {
        bool out;
        std::visit([&out](auto &value)
                   {
            using T = std::decay_t<decltype(value)>;
            if (std::is_same_v<T, Dict>){
                out=true;
            } else{
                out=false;
            } },
                   keep_);
        return out;
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
            if (IsInt())
            {
                return std::get<int>(keep_);
            }
            else
            {
                return std::get<double>(keep_);
            }
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

    void PrintValue(std::nullptr_t, std::ostream &out)
    {
        out << "null"sv;
    }
    void PrintValue(const bool value, std::ostream &out)
    {

        out << std::boolalpha << value;
    }

    void PrintValue(const std::string &value, std::ostream &out)
    {
        std::string out_str = "\"";
        for (char ch : value)
        {
            switch (ch)
            {
            case '"':
                out_str += "\\\""s;
                break;
            case '\\':
                out_str += "\\\\"s;
                break;
            case '\n':
                out_str += "\\n";
                break;
            case '\r':
                out_str += "\\r";
                break;
            default:
                out_str.push_back(ch);
            }
        }
        out_str.push_back('\"');
        out << out_str;
    }

    void PrintValue(const Array &array, std::ostream &out)
    {
        out << "["s;
        auto it = array.begin();
        PrintNode(*it, out);
        ++it;
        while (it != array.end())
        {
            out << ",";
            PrintNode(*it, out);
            ++it;
        }
        out << "]"s;
    }

    void PrintValue(const Dict &map, std::ostream &out)
    {
        out << "{";
        for (auto &[key, value] : map)
        {
            PrintValue(key, out);
            out << ":";
            PrintNode(value, out);
        }
        out << "}"s;
    }

    void PrintNode(const Node &node, std::ostream &out)
    {
        std::visit(
            [&out](const auto &value)
            { PrintValue(value, out); },
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

    void Print(const Document &doc, std::ostream &output) // all document print func
    {
        PrintNode(doc.GetRoot(), output);

        // Реализуйте функцию самостоятельно
    }

} // namespace json