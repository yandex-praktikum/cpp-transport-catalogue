#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  Array result;

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

Node LoadDouble(istream& input, std::string& res) {
  char c;
  while (input >> c) {
    if (std::isdigit(c) || c == '-' || c == '+' || c == 'e' || c == 'E') {
      res.push_back(c);
    } else break;
  }
  try {
    input.putback(c);
    return Node{std::stod(res)};
  } catch (...) {
    throw ParsingError("error in double");
  }

}

Node LoadInt(istream& input) {
  char c;
  std::string res;
  while (input >> c) {
    if (c == '.' || c == 'e' || c == 'E') {
      res.push_back(c);
      return LoadDouble(input, res);
    }
    if (std::isdigit(c) || c == '-' || c == '+') {
      res.push_back(c);
    } else break;
  }
  try {
    input.putback(c);
    return Node{std::stoi(res)};
  } catch (...) {
    throw ParsingError("error in int");
  }
}

Node LoadString(istream& input) {
  string line;
  char c;
  while (input.get(c)) {
    if (c == '"') {
      break;
    } else if (c == '\\') {
      input.get(c);
      if (c == 'n') {
        line.push_back('\n');
      } else if (c == '"') {
        line.push_back('"');
      } else if (c == '\\') {
        line.push_back('\\');
      } else if (c == 'r') {
        line.push_back('\r');
      } else if (c == 't') {
        line.push_back('\t');
      }
    } else {
      line.push_back(c);
    }
  }
  if (!input) {
    throw ParsingError("string parsing error"s);
  }
  return Node{line};

}

Node LoadDict(istream& input) {
  Dict result;

  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.insert({std::move(key), LoadNode(input)});
  }
  if (!input) {
    throw ParsingError("Array parsing error"s);
  }

  return Node{std::move(result)};
}
Node LoadNull(istream& input) {
  std::string res;
  char c;
  while (input >> c) {
    if (std::isalpha(c)) {
      res.push_back(c);
    }
    if (res == "null") {
      return Node{nullptr};
    }
    if (res.size() == 5) {
      break;
    }
  }
  throw ParsingError("error");
}
Node LoadBool(istream& input) {
  std::string res;
  char c;
  while (input >> c) {
    if (std::isalpha(c)) {
      res.push_back(c);
      if (res == "true") {
        return Node{true};
      }
      if (res == "false") {
        return Node{false};
      }
      if (res.size() == 6) {
        break;
      }
    }
  }
  throw ParsingError("error in bool");
}
Node LoadNode(istream& input) {
  char c;
  input >> c;
  if (c == 'n') {
    input.putback(c);
    return LoadNull(input);

  } else if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else if (c == 't' || c == 'f') {
    input.putback(c);
    return LoadBool(input);
  } else {
    input.putback(c);
    return LoadInt(input);
  }
}

}  // namespace


bool Node::IsNull() const {
  return std::holds_alternative<std::nullptr_t>(*this);
}
bool Node::IsBool() const {
  return std::holds_alternative<bool>(*this);
}
bool Node::IsInt() const {
  return std::holds_alternative<int>(*this);
}

bool Node::IsString() const {
  return std::holds_alternative<std::string>(*this);
}

bool Node::IsPureDouble() const {
  return std::holds_alternative<double>(*this);
}
bool Node::IsDouble() const {
  return IsInt() || IsPureDouble();
}
bool Node::IsMap() const {
  return std::holds_alternative<Dict>(*this);
}
bool Node::IsArray() const {
  return std::holds_alternative<Array>(*this);
}

const Node::Value& Node::
GetValue() const {
  return *this;
}
const Array& Node::AsArray() const {
  if (std::get_if<Array>(this)) {
    return *std::get_if<Array>(this);
  }
  throw std::logic_error("logic in array");
}

const Dict& Node::AsMap() const {
  if (std::get_if<Dict>(this)) {
    return *std::get_if<Dict>(this);
  }
  throw std::logic_error("logic in map");
}

int Node::AsInt() const {
  if (std::get_if<int>(this)) {
    return *std::get_if<int>(this);
  }
  throw std::logic_error("logic in int");
}
double Node::AsDouble() const {
  if (std::get_if<double>(this)) {
    return *std::get_if<double>(this);
  }
  if (std::get_if<int>(this)) {
    return *std::get_if<int>(this);
  }
  throw std::logic_error("logic in double");
}

const string& Node::AsString() const {
  if (std::get_if<std::string>(this)) {
    return *std::get_if<std::string>(this);
  }
  throw std::logic_error("logic in string");
}
bool Node::AsBool() const {
  if (std::get_if<bool>(this)) {
    return *std::get_if<bool>(this);
  }
  throw std::logic_error("logic in bool");
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
  return root_;
}

Document Load(istream& input) {
  return Document{LoadNode(input)};
}
struct Printer {
  std::ostream& out;
  void operator()(std::nullptr_t) const {
    out << "null";
  }
  void operator()(int num) const {
    out << num;
  }
  void operator()(bool x) const {
    (x ? out << "true" : out << "false");
  }
  void operator()(const Array& arr) const {
    out << "[";
    bool flag = true;
    for (const auto& item : arr) {
      if (!flag) {
        out << ", ";
      }
      Print(Document{item}, out);
      flag = false;
    }
    out << "]";
  }
  void operator()(const Dict& map) const {
    out << "{";
    bool flag = true;
    for (const auto& [key, value] : map) {
      if (!flag) {
        out << ", ";
      }
      Print(Document{key}, out);
      out << ": ";
      Print(Document{value}, out);
      flag = false;
    }
    out << "}";
  }
  void operator()(double num) const {
    out << num;
  }
  void operator()(const std::string& str) const {
    out << "\"";
    for (auto symbol : str) {
      switch (symbol) {
        case '\"':out << "\\\"";
          break;
        case '\r':out << "\\r";
          break;
        case '\n':out << "\\n";
          break;
        case '\\':out << "\\\\";
          break;
        case '\t':out << "\t";
          break;
        default:out << symbol;
      }

    }
    out << "\"";
  }

};

void Print(const Document& doc, std::ostream& output) {
  std::ofstream strm;
  std::visit(Printer{output}, doc.GetRoot().GetValue());

  // Реализуйте функцию самостоятельно
}

}  // namespace json