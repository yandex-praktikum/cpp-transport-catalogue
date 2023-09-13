#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <fstream>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node : private  std::variant<std::nullptr_t, int, double, std::string, bool, Array , Dict > {
 public:

  using variant::variant;
  using Value = variant;
  bool IsNull() const;
  bool IsBool() const;
  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsString() const;
  bool IsArray() const;
  bool IsMap() const;

  const Value& GetValue() const;
  /* Реализуйте Node, используя std::variant */


  const Array& AsArray() const;
  const Dict& AsMap() const;
  int AsInt() const;
  const std::string& AsString() const;
  double AsDouble() const;
  bool AsBool() const;


  friend bool operator == (const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
  }
  friend bool operator != (const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
  }


};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

  friend bool operator == (const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
  }
  friend bool operator != (const Document& lhs, const Document& rhs) {
    return !(lhs.GetRoot() == rhs.GetRoot());
  }

 private:
  Node root_;
};

Document Load(std::istream& input);


void Print(const Document& doc, std::ostream& output);

}  // namespace json