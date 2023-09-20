#include "json_builder.h"

#include <utility>

json::DictItemContext json::Builder::StartDict() {
  Dict empty_dict;
  if(nodes_stacks_.empty()) {
    throw std::logic_error("json have been built");
  }
  if(nodes_stacks_.back()->IsArray()) {
    std::get<json::Array>(nodes_stacks_.back()->GetValue()).emplace_back(empty_dict);
    nodes_stacks_.push_back(&std::get<Array>(nodes_stacks_.back()->GetValue()).back());
  } else if(nodes_stacks_.back()->IsDict()) {
    if(!cur_key.has_value()) {
      throw std::logic_error("dict in dict must be a value");
    }
    std::get<Dict>(nodes_stacks_.back()->GetValue())[cur_key.value()] = empty_dict;
    nodes_stacks_.push_back(&std::get<Dict>(nodes_stacks_.back()->GetValue()).at(cur_key.value()));
    cur_key.reset();
  } else {
    static Node tmp(empty_dict);
    nodes_stacks_.push_back(&tmp);
  }

  return DictItemContext(*this);
}
json::ArrayItemCotext json::Builder::StartArray() {
  Array empty_array;
  if (nodes_stacks_.empty()) {

    throw std::logic_error("no matching to call start array");
  } else if (nodes_stacks_.back()->IsDict()) {
    if(!cur_key.has_value()) {
      throw std::logic_error("dict in dict must be a value");
    }
    std::get<Dict>(nodes_stacks_.back()->GetValue())[cur_key.value()] = empty_array;
    nodes_stacks_.push_back(&std::get<Dict>(nodes_stacks_.back()->GetValue()).at(cur_key.value()));
    cur_key.reset();
    return ArrayItemCotext(*this);
  } else if(nodes_stacks_.back()->IsArray()) {
    std::get<Array>(nodes_stacks_.back()->GetValue()).emplace_back(empty_array);
    nodes_stacks_.push_back(&std::get<Array>(nodes_stacks_.back()->GetValue()).back());
    return ArrayItemCotext(*this);
  } else {
    static Node tmp(empty_array);
    nodes_stacks_.push_back(&tmp);
  }
  return ArrayItemCotext(*this);

}

json::KeyItemContext json::Builder::Key(std::string key) {
  if(nodes_stacks_.empty()) {
    throw std::logic_error("there must be dict");
  }
  if (nodes_stacks_.back()->IsDict()) {
    if(!cur_key.has_value()) {
      cur_key = std::move(key);
    }
    else {
      throw std::logic_error("two keys in row");
    }
  }else {
    throw std::logic_error("Key is for Dict()");
  }
  return KeyItemContext(*this);
}

json::BaseContext json::Builder::Value(Node::Value value) {
  auto val = CastValueToNode(value);
  if(nodes_stacks_.size() == 1 && nodes_stacks_.back()->IsNull()) {
    nodes_stacks_.back() = &val;
    root_ = val;
    return *this;
  }
  else if(nodes_stacks_.back()->IsDict()){
    if(cur_key.has_value()) {
      std::get<json::Dict >(nodes_stacks_.back()->GetValue()).emplace(std::move(cur_key.value()), val);
      cur_key.reset();
      return *this;
    }
  }
  else if(nodes_stacks_.back()->IsArray()) {
    std::get<json::Array>(nodes_stacks_.back()->GetValue()).push_back(val);

    return *this;
  }
  throw std::logic_error("json has been constructed");
}
json::BaseContext json::Builder::EndDict() {
  if(nodes_stacks_.empty()) {
    throw std::logic_error("try to close closed dict");
  }
  if(nodes_stacks_.back()->IsDict()) {
    root_.GetValue() = nodes_stacks_.back()->GetValue();
    nodes_stacks_.pop_back();
  } else {
    throw std::logic_error("last must be dict");
  }
  return *this;
}

json::BaseContext json::Builder::EndArray() {
  if(nodes_stacks_.empty()) {
    throw std::logic_error("try to close closed arr");
  }
  if(nodes_stacks_.back()->IsArray()) {
    root_.GetValue() = nodes_stacks_.back()->GetValue();
    nodes_stacks_.pop_back();
  } else {
    throw std::logic_error("last must be array");
  }
  return *this;

}
json::Node json::Builder::Build() {
  if(nodes_stacks_.size() == 1 && root_ != nullptr) {
    nodes_stacks_.pop_back();
    return root_;
  } else {
    throw std::logic_error("stack must be empty");
  }
}

namespace json {
DictItemContext BaseContext::StartDict() {
  return builder_.StartDict();
}
ArrayItemCotext BaseContext::StartArray() {
  return builder_.StartArray();
}
BaseContext BaseContext::EndArray() {
  return builder_.EndArray();
}
BaseContext BaseContext::EndDict() {
  return builder_.EndDict();
}
Node BaseContext::Build() {
  return builder_.Build();
}
BaseContext BaseContext::Value(Node::Value value) {
  return builder_.Value(value);
}
KeyItemContext BaseContext::Key(std::string key) {
  return builder_.Key(std::move(key));
}

Node CastValueToNode(const Node::Value& value) {
  Node node;
  if (std::holds_alternative<int>(value) ){
    node = std::get<int>(value);
  }else if (std::holds_alternative<bool>(value)) {
    node= std::get<bool>(value);
  } else if(std::holds_alternative<double>(value)) {
    node = std::get<double>(value);
  }else if (std::holds_alternative<std::nullptr_t>(value) ) {
    node = std::get<std::nullptr_t>(value);
  } else if (std::holds_alternative<std::string>(value)) {
    node = std::get<std::string>(value);
  } else if (std::holds_alternative<Array>(value)) {
    node = std::get<Array>(value);
  } else{
    node = std::get<Dict>(value);
  }
  return node;
}
}
