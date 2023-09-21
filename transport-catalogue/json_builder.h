#include <optional>
#include <utility>

#include "json.h"

namespace json {
class DictItemContext; 
class ArrayItemCotext;
class KeyItemContext;
class BaseContext;
class Builder {
 public:
  Builder() {
    nodes_stacks_.push_back(&root_);
  }

  DictItemContext StartDict();
  BaseContext EndDict();
  KeyItemContext Key(std::string key);
  BaseContext Value(Node val);
  ArrayItemCotext StartArray();
  BaseContext EndArray();
  Node Build();

 private:
  Node root_;
  std::vector<Node*> nodes_stacks_;
  std::optional<std::string> cur_key;
};

class BaseContext {
 public:
  BaseContext(Builder& builder) : builder_(builder){}
  Node Build();
  KeyItemContext Key(std::string key);
  BaseContext Value(Node value);
  DictItemContext StartDict();
  ArrayItemCotext StartArray();
  BaseContext EndDict();
  BaseContext EndArray();
  Builder& builder_;
};

class DictItemContext :  public BaseContext {
 public:
  explicit DictItemContext(Builder& base): BaseContext(base){}
  Node Build() = delete;
  BaseContext Value(Node value) = delete;
  DictItemContext StartDict() = delete;
  ArrayItemCotext StartArray() = delete;
  BaseContext EndArray() = delete;

};

class ArrayItemCotext: public BaseContext {
 public:
  explicit ArrayItemCotext(Builder& base) : BaseContext(base){}
  Node Build() = delete;
  KeyItemContext Key(std::string key) = delete;
  BaseContext EndDict() = delete;
  ArrayItemCotext Value(Node value) {
    builder_.Value(std::move(value));
    return ArrayItemCotext(builder_);
  }

};

class KeyItemContext: public BaseContext {
 public:
  explicit KeyItemContext(Builder& base) : BaseContext(base){}

  Node Build() = delete;
  KeyItemContext Key(std::string key) = delete;
  BaseContext EndDict() = delete;
  BaseContext EndArray() = delete;

  DictItemContext Value(Node value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
  }
};



} // namespace json
