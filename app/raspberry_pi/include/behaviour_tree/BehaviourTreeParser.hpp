#ifndef BEHAVIOUR_TREE_PARSER_HPP
#define BEHAVIOUR_TREE_PARSER_HPP

#pragma once

#include <filesystem>
#include <string>

#include <tl/expected.hpp>
#include <pugixml.hpp>
#include <fmt/format.h>

#include "BehaviourTree.hpp"
#include "Root.hpp"
#include "Context.h"
#include "node/Node.hpp"

#include "node/composite/Composite.hpp"
#include "node/composite/Sequence.hpp"
#include "node/composite/Selector.hpp"

#include "node/decorator/Decorator.hpp"
#include "node/decorator/Repeat.hpp"
#include "node/decorator/Invert.hpp"
#include "node/decorator/Wait.hpp"

#include "node/leaf/Leaf.hpp"
#include "node/leaf/Task.hpp"
#include "node/leaf/Succeed.hpp"
#include "node/leaf/Fail.hpp"
#include "node/leaf/LogMessage.hpp"
#include "node/leaf/ToRoot.hpp"

#include "node/leaf/Condition.hpp"
#include "node/leaf/condition/NearbyPoints.hpp"

#include "action/Action.hpp"
#include "action/ActionParser.hpp"

using namespace behaviour_tree::node;
using namespace behaviour_tree::node::composite;
using namespace behaviour_tree::node::decorator;
using namespace behaviour_tree::node::leaf;
using namespace behaviour_tree::node::leaf::condition;
using namespace behaviour_tree::action;

namespace behaviour_tree
{
	static constexpr int STARTING_INDEX = 1;

	// https://stackoverflow.com/a/46711735
	static constexpr uint32_t hash(const std::string_view s) noexcept
	{
		uint32_t hash = 5381;

		for (const char* c = s.data(); c < s.data() + s.size(); ++c)
			hash = ((hash << 5) + hash) + (unsigned char)*c;

		return hash;
	}

	class BehaviourTreeParser
	{
	public:
        BehaviourTreeParser(BehaviourTreeParser const&) = delete;
        void operator=(BehaviourTreeParser const&)  = delete;

        static BehaviourTreeParser& instance()
        {
            static BehaviourTreeParser instance;
            return instance;
        }

		void setActionParser(std::unique_ptr<ActionParser> action_parser)
		{
			this->action_parser = std::move(action_parser);
		}

		tl::expected<std::shared_ptr<BehaviourTree>, std::string> parseXML(const std::string& xml)
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_string(xml.c_str());
			if (!result)
			{
				return tl::unexpected("XML parsed with errors");
			}
			return parse(doc);
		}

		tl::expected<std::shared_ptr<BehaviourTree>, std::string> parseFileXML(const std::string& file_path)
		{
			if (std::filesystem::exists(file_path) == false)
			{
				return tl::unexpected("File [" + file_path + "] does not exist");
			}
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_file(file_path.c_str());
			if (!result)
			{
				return tl::unexpected("XML [" + file_path + "] parsed with errors");
			}
			return parse(doc);
		}

	private:
        BehaviourTreeParser() {}
		
        std::unique_ptr<ActionParser> action_parser;

		tl::expected<std::shared_ptr<BehaviourTree>, std::string> parse(pugi::xml_document& doc)
		{
			std::vector<std::shared_ptr<Root>> roots;
			for (pugi::xml_node& node = doc.child("Root"); node; node = node.next_sibling("Root"))
			{
				auto maybe_root = parseRoot(node, STARTING_INDEX + roots.size());
				if (!maybe_root.has_value())
				{
					return tl::unexpected(maybe_root.error());
				}
				roots.push_back(std::move(maybe_root.value()));
			}
			if (roots.size() <= 0)
			{
				return tl::unexpected(R"(No "Root" node found)");
			}
			return std::make_unique<BehaviourTree>(
				BehaviourTree(std::move(roots))
			);
		}

	private:
		tl::expected<std::shared_ptr<Root>, std::string> parseRoot(pugi::xml_node& node, const int& index)
		{
			int child_count = 0;
			for (pugi::xml_node& child = node.first_child(); child; child = child.next_sibling())
				++child_count;
			const std::string id = node.attribute("id").as_string();
			if (child_count != 1)
			{
				return tl::unexpected(fmt::format(R"(Root node must have only one child | Root:["{}",{}])", id, index));
			}
			pugi::xml_node& child = node.first_child();
			auto maybe_child_node = parseChild(child, STARTING_INDEX);
			if (!maybe_child_node.has_value())
			{
				return tl::unexpected(fmt::format(R"({}<-Root:["{}",{}])", maybe_child_node.error(), id, index));
			}
			return std::make_shared<Root>(
				Root(id, std::move(maybe_child_node.value()))
			);
		}

		tl::expected<std::unique_ptr<Node>, std::string> parseChild(pugi::xml_node& node, const int& index)
		{
			const std::string name = node.attribute("name").as_string();
			const std::string& node_name = node.name();
			switch (hash(node_name))
			{
#pragma region Composite Node
			case hash("Sequence"):
			{
				return parseComposite(node, index, CompositeType::Sequence);
			}
			case hash("Selector"):
			{
				return parseComposite(node, index, CompositeType::Selector);
			}
#pragma endregion
#pragma region Decorator Node
			case hash("Repeat"):
			{
				return parseDecorator(node, index, DecoratorType::Repeat);
			}
			case hash("Invert"):
			{
				return parseDecorator(node, index, DecoratorType::Invert);
			}
			case hash("Wait"):
			{
				return parseDecorator(node, index, DecoratorType::Wait);
			}
#pragma endregion
#pragma region Leaf Node
			case hash("Condition"):
			{
				return parseCondition(node, index);
			}
			case hash("Task"):
			{
				return parseTask(node, index);
			}
			case hash("Succeed"):
			{
				return std::make_unique<Succeed>(Succeed(name));
			}
			case hash("Fail"):
			{
				return std::make_unique<Fail>(Fail(name));
			}
			case hash("LogMessage"):
			{
				return std::make_unique<LogMessage>(
					LogMessage(
						name,
						node.attribute("text").as_string()));
			}
			case hash("ToRoot"):
			{
				return std::make_unique<ToRoot>(
					ToRoot(
						name,
						node.attribute("id").as_string()));
			}
#pragma endregion
			default:
			{
				return tl::unexpected(fmt::format(R"(Invalid node | {}:["{}",{}])", node_name, name, index));
			}
			}
		}

		tl::expected<std::unique_ptr<Decorator>, std::string> parseDecorator(pugi::xml_node& node, const int& index, const DecoratorType& decorator_type)
		{
			const std::string name = node.attribute("name").as_string();
			pugi::xml_node& child = node.first_child();
			if (!child)
			{
				return tl::unexpected(fmt::format(R"(Decorator node must have a child | {}:["{}",{}])", std::string(node.name()), name, index));
			}
			auto maybe_child_node = parseChild(child, STARTING_INDEX);
			if (!maybe_child_node.has_value())
			{
				return tl::unexpected(maybe_child_node.error());
			}
			switch (decorator_type)
			{
			case DecoratorType::Invert:
			{
				return std::make_unique<Invert>(
					Invert(
						name,
						std::move(maybe_child_node.value())));
			}
			case DecoratorType::Repeat:
			{
				return std::make_unique<Repeat>(
					Repeat(
						name,
						std::move(maybe_child_node.value()),
						node.attribute("count").as_int(),
						node.attribute("break_on_fail").as_bool()));
			}
			case DecoratorType::Wait:
			{
				return std::make_unique<Wait>(
					Wait(
						name,
						std::move(maybe_child_node.value()),
						node.attribute("ms").as_int()));
			}
			}
		}

		tl::expected<std::unique_ptr<Composite>, std::string> parseComposite(pugi::xml_node& node, const int& index, const CompositeType& composite_type)
		{
			const std::string name = node.attribute("name").as_string();
			std::vector<std::unique_ptr<Node>> children;
			for (pugi::xml_node& child = node.first_child(); child; child = child.next_sibling())
			{
				auto maybe_node = parseChild(child, STARTING_INDEX + children.size());
				if (!maybe_node.has_value())
				{
					return tl::unexpected(fmt::format(R"({}<-{}:["{}",{}])", maybe_node.error(), std::string(node.name()), name, index));
				}
				children.push_back(std::move(maybe_node.value()));
			}
			if (children.size() <= 0)
			{
				return tl::unexpected(fmt::format(R"(Composite node must have at least one child | {}:["{}",{}])", std::string(node.name()), name, index));
			}
			switch (composite_type)
			{
			case CompositeType::Sequence:
				return std::make_unique<Sequence>(Sequence(name, std::move(children)));
			case CompositeType::Selector:
				return std::make_unique<Selector>(Selector(name, std::move(children)));
			}
		}

		tl::expected<std::unique_ptr<Condition>, std::string> parseCondition(pugi::xml_node& node, const int& index)
		{
			const std::string name = node.attribute("name").as_string();
			switch (hash(node.attribute("type").as_string()))
			{
			case hash("NearbyPoints"):
				return std::make_unique<NearbyPoints>(
					NearbyPoints(
						name,
						node.attribute("min_angle").as_int(),
						node.attribute("max_angle").as_int(),
						node.attribute("avg_distance").as_int()
					)
				);

			default:
				return tl::unexpected(fmt::format(R"(Invalid condition type: {} | Condition:["{}",{}])", std::string(node.attribute("type").as_string()), name, index));
			}
		}

		tl::expected<std::unique_ptr<Task>, std::string> parseTask(pugi::xml_node& node, const int& index)
		{
			std::vector<std::unique_ptr<Action>> actions;
			for (pugi::xml_node& child = node.child("Action"); child; child = child.next_sibling("Action"))
			{
				auto maybe_action = action_parser->parseAction(child);
				if (!maybe_action.has_value())
				{
					return tl::unexpected(maybe_action.error());
				}
				actions.push_back(std::move(maybe_action.value()));
			}
			const std::string name = node.attribute("name").as_string();
			if (actions.size() <= 0)
			{
				return tl::unexpected(fmt::format(R"(Task node must have at least one action | Task:["{}",{}])", name, index));
			}
			return std::make_unique<Task>(Task(name, std::move(actions)));
		}
	};
}

#endif