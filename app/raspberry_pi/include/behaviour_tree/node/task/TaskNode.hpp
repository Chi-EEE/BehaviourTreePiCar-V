#ifndef BEHAVIOUR_TREE_TASK_NODE_HPP
#define BEHAVIOUR_TREE_TASK_NODE_HPP

#pragma once

#include "../Node.hpp"

namespace behaviour_tree::node::task
{
	class TaskNode : public Node
	{
	public:
		TaskNode(const std::string& name) : Node(name) {

		}
	};
}

#endif