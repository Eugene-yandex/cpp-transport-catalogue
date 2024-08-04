#include "json_builder.h"
#include <variant>

namespace json {
	using namespace std::literals;

	Builder::Builder() {
		nodes_stack_.push_back(&root_);
	}

	ValueItemContext Builder::Key(std::string str) {
		if (!nodes_stack_.empty()) {
			if (std::holds_alternative<Dict>(nodes_stack_.back()->GetNotConstValue())) {
				nodes_stack_.push_back(
					&std::get<Dict>(nodes_stack_.back()->GetNotConstValue())[std::move(str)]);
			}
			else {
				throw std::logic_error("Input key not in Dict"s);
			}
		}
		else {
			throw std::logic_error("Input key in nonexistent Dict"s);
		}
		return ValueItemContext{ *this };
	}

	BaseContext Builder::Value(Node::Value val) {
		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsNull()) {
				nodes_stack_.back()->GetNotConstValue() = val;
				nodes_stack_.pop_back();
			}
			else if (nodes_stack_.back()->IsArray()) {
				std::get<Array>(nodes_stack_.back()->GetNotConstValue()).emplace_back(std::move(val));
			}
			else {
				throw std::logic_error("Attantion! We can't put value in unknown JSON"s);
			}
		}
		else {
			throw std::logic_error("Attantion! We can't put value in nonexistent JSON"s);
		}
		return *this;
	}

	DictItemContext Builder::StartDict() {
		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsNull()) {
				nodes_stack_.back()->GetNotConstValue() = Dict{};
			}
			else if (nodes_stack_.back()->IsArray()) {
				nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetNotConstValue()).emplace_back(Dict{}));
			}
			else {
				throw std::logic_error("Attantion! We can't put Dict in unknown JSON"s);
			}
		}
		else {
			throw std::logic_error("Can't put Dict in nonexistent JSON"s);
		}
		return  DictItemContext{ *this };
	}

	ArrayItemContext Builder::StartArray() {
		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsNull()) {
				nodes_stack_.back()->GetNotConstValue() = Array{};
			}
			else if (nodes_stack_.back()->IsArray()) {
				nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetNotConstValue()).emplace_back(Array{}));
			}
			else {
				throw std::logic_error("Attantion! We can't put Array in unknown JSON"s);
			}
		}
		else {
			throw std::logic_error("Can't put Array in nonexistent JSON"s);
		}
		return ArrayItemContext{ *this };
	}

	BaseContext Builder::EndDict() {
		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsMap()) {
				nodes_stack_.pop_back();
			}
			else {
				throw std::logic_error("Attantion! We can't close Dict except Dict"s);
			}
		}
		else {
			throw std::logic_error("Can't close Dict"s);
		}
		return BaseContext{ *this };
	}
	BaseContext Builder::EndArray() {
		if (!nodes_stack_.empty()) {
			if (nodes_stack_.back()->IsArray()) {
				nodes_stack_.pop_back();
			}
			else {
				throw std::logic_error("Attantion! We can't close Array except Array"s);
			}
		}
		else {
			throw std::logic_error("Can't close Array"s);
		}
		return BaseContext{ *this };
	}
	Node Builder::Build() {
		if (!root_.IsNull() && nodes_stack_.empty()) {
			return std::move(root_);
		}
		else {
			throw std::logic_error("Can't complite Array"s);
		}
	}

	BaseContext::BaseContext(Builder& builder) :
		builder_(builder) {
	}

	Node BaseContext::Build() {
		return builder_.Build();
	}

	ValueItemContext BaseContext::Key(std::string str) {
		return builder_.Key(std::move(str));
	}

	BaseContext BaseContext::Value(Node::Value val) {
		return builder_.Value(std::move(val));
	}

	DictItemContext BaseContext::StartDict() {
		return builder_.StartDict();

	}

	ArrayItemContext BaseContext::StartArray() {
		return builder_.StartArray();

	}

	BaseContext BaseContext::EndDict() {
		return builder_.EndDict();

	}
	BaseContext BaseContext::EndArray() {
		return builder_.EndArray();
	}

	ValueItemContext::ValueItemContext(BaseContext base_context) :
		BaseContext(base_context) {
	}

	DictItemContext ValueItemContext::Value(Node::Value val) {
		return BaseContext::Value(std::move(val));
	}

	ArrayItemContext::ArrayItemContext(BaseContext base_context) :
		BaseContext(base_context) {
	}


	ArrayItemContext ArrayItemContext::Value(Node::Value val) {
		return BaseContext::Value(std::move(val));
	}

	DictItemContext::DictItemContext(BaseContext base_context) : BaseContext(base_context) {};

}