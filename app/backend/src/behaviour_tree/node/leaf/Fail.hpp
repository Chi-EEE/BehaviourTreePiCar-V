#ifndef BEHAVIOUR_TREE_FAIL_HPP
#define BEHAVIOUR_TREE_FAIL_HPP

#pragma once

#include "Leaf.hpp"

namespace behaviour_tree::node::leaf
{
	class Fail : public Leaf
	{
	public:
		Fail(const std::string& name) : Leaf(name) {}

		const LeafType type() const override { return LeafType::Fail; }

		const Status run() override
		{
			return Status::FAILURE;
		}

		const std::string toString() const override {
			return fmt::format(R"(<Fail name="{}"/>)", this->getName());
		}
	};
}

#endif