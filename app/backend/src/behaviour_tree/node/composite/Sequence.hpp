#ifndef BEHAVIOUR_TREE_SEQUENCE_HPP
#define BEHAVIOUR_TREE_SEQUENCE_HPP

#pragma once

#include "Composite.hpp"

namespace behaviour_tree::node::composite
{
	class Sequence : public Composite
	{
	public:
		Sequence(const std::string &name, std::vector<std::unique_ptr<Node>> children) : Composite(name, std::move(children)) {}

		const CompositeType type() const override { return CompositeType::Sequence; }

		const std::string toString() const override {
			std::string out;
			for (auto& child : this->children)
			{
				out += child->toString();
			}
			const std::string& name = this->getName();
			if (name != "")
				return fmt::format(R"(<Sequence name="{}">{}</Sequence>)", name, out);
			else
				return fmt::format(R"(<Sequence>{}</Sequence>)", out);
		}
	};
}

#endif