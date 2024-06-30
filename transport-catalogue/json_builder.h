#pragma once
#include "json.h"


namespace json {
	class BaseContext;
	class DictItemContext;
	class ArrayItemContext;
	class ValueItemContext;

	class Builder {
	public:
		Builder();
		ValueItemContext Key(std::string);

		BaseContext Value(Node::Value val);

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		BaseContext EndDict();
		BaseContext EndArray();
		Node Build();
	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
	};

	class BaseContext {
	public:
		BaseContext(Builder& builder);
		Node Build();
		ValueItemContext Key(std::string);

		BaseContext Value(Node::Value val);

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		BaseContext EndDict();
		BaseContext EndArray();

	private:
		Builder& builder_;
	};


	class DictItemContext : public BaseContext {
	public:
		DictItemContext(BaseContext base_context);

		Node Build() = delete;

		BaseContext Value(Node::Value val) = delete;

		DictItemContext StartDict() = delete;

		ArrayItemContext StartArray() = delete;

		BaseContext EndArray() = delete;
	};

	class ArrayItemContext : public BaseContext {
	public:
		ArrayItemContext(BaseContext base_context);

		Node Build() = delete;
		ValueItemContext Key(std::string) = delete;

		ArrayItemContext Value(Node::Value val);

		BaseContext EndDict() = delete;
	};

	class ValueItemContext : public BaseContext {
	public:
		ValueItemContext(BaseContext base_context);
		Node Build() = delete;
		ValueItemContext Key(std::string) = delete;

		DictItemContext Value(Node::Value val);

		BaseContext EndDict() = delete;
		BaseContext EndArray() = delete;
	};

}
