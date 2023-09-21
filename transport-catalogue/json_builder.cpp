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
    return DictItemContext(*this);
  } else if(nodes_stacks_.back()->IsDict()) {
    if(!cur_key.has_value()) {
      throw std::logic_error("dict in dict must be a value");
    }
    std::get<Dict>(nodes_stacks_.back()->GetValue())[cur_key.value()] = empty_dict;
    nodes_stacks_.push_back(&std::get<Dict>(nodes_stacks_.back()->GetValue()).at(cur_key.value()));
    cur_key.reset();
    return DictItemContext(*this);
  } else if (nodes_stacks_.back()->IsNull()) {
    *nodes_stacks_.back() = empty_dict;
    return DictItemContext(*this);

  }

  throw std::logic_error("error in dict");
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
  } else if(root_.IsNull() && nodes_stacks_.back() == &root_){
    *nodes_stacks_.back() = empty_array;
    return ArrayItemCotext(*this);
  }
  throw std::logic_error("error in array");

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

json::BaseContext json::Builder::Value(Node val) {
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
    nodes_stacks_.pop_back();
  } else {
    throw std::logic_error("last must be array");
  }
  return *this;

}
json::Node json::Builder::Build() {
  if(nodes_stacks_.size() == 0 && root_ != nullptr) {
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
BaseContext BaseContext::Value(Node value) {
  return builder_.Value(value);
}
KeyItemContext BaseContext::Key(std::string key) {
  return builder_.Key(std::move(key));
}


}
