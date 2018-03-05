#include "BehaviorTree.h"
#include "BTNode.h"
#include "BTNodeTypes.h"
#include "BTLink.h"
#include "BTPin.h"
#include "Log.h"
#include "Data.h"
#include "Random.h"

#include "ThirdParty/NodeEditor/Include/NodeEditor.h"
#include "ThirdParty/NodeEditor/Source/Shared/Interop.h"

namespace ne = ax::NodeEditor;
using namespace std;

BehaviorTree::BehaviorTree()
{
	uid = g_rnd->RandomInt();
	AddNode(0, 0, 0); // Root node
	rootNode = nodesList[0];
}

BehaviorTree::BehaviorTree(Data & data)
{
	map<int, BTPin*> pinsList;
	// Load Nodes
	int numNodes = data.GetArraySize("nodes");
	for (int i = 0; i < numNodes; ++i)
	{
		Data nodeData = data.GetArray("nodes", i);
		AddNode(nodeData, pinsList);
	}
	// Load Links
	int numLinks = data.GetArraySize("links");
	for (int i = 0; i < numLinks; ++i)
	{
		Data linkData = data.GetArray("links", i);
		BTLink* link = new BTLink(linkData, pinsList);
		linksList.insert(pair<int, BTLink*>(link->id, link));
	}

	uid = data.GetInt("uid");
	nextId = data.GetInt("nextId");
	int rootNodeId = data.GetInt("rootNodeId");
	map<int, BTNode*>::iterator found = nodesList.find(rootNodeId);
	rootNode = (found != nodesList.end()) ? found->second : nullptr;
}

BehaviorTree::~BehaviorTree()
{
	for (auto link : linksList)
	{
		delete link.second;
	}
	for (auto node : nodesList)
	{
		delete node.second;
	}
}

void BehaviorTree::AddNode(float posX, float posY, int typeId)
{
	int nodeId = GetNextId();
	int inputId = GetNextId();
	int outputId = GetNextId();
	BTNode* node = new BTNode(nodeId, outputId, inputId, typeId, this);

	nodesList.insert(pair<int, BTNode*>(nodeId, node));
	
	ne::SetNodePosition(nodeId, ImVec2(posX, posY));
	LOGI("Node %i created", nodeId);
}

void BehaviorTree::AddLink(BTPin* startPinId, BTPin* endPinId)
{
	BTLink* link = new BTLink(GetNextId(), startPinId, endPinId);
	startPinId->links.push_back(link);
	endPinId->links.push_back(link);
	linksList.insert(pair<int, BTLink*>(link->id, link));
}

void BehaviorTree::RemoveNode(int id)
{
	vector<BTLink*> linksToRemove;
	map<int, BTNode*>::iterator found = nodesList.find(id);
	if (found != nodesList.end())
	{
		linksToRemove = found->second->GetAllLinks();
		for (auto link : linksToRemove)
		{
			link->CleanUp();
			linksList.erase(link->id);
			delete link;
		}
		delete found->second;
		nodesList.erase(found);
	}
}

void BehaviorTree::RemoveLink(int id)
{
	map<int, BTLink*>::iterator found = linksList.find(id);
	if (found != linksList.end())
	{
		found->second->CleanUp();

		delete found->second;
		linksList.erase(found);
	}
}

int BehaviorTree::Serialize(char** buffer) const
{
	Data data;
	data.AppendArray("nodes");
	// Serialize BTNodes
	bool allNodesSerialized = false;
	
	if (rootNode)
		rootNode->Save(data);

	while (allNodesSerialized == false)
	{
		allNodesSerialized = true;
		BTNode* nodeNotSerialized = nullptr;
		for (auto node : nodesList)
		{
			if (node.second->saveFlag == false)
			{
				nodeNotSerialized = node.second;
				allNodesSerialized = false;
				break;
			}
		}
		if (allNodesSerialized == false)
		{
			// Find rootNode of this sub-tree. Serialize from that node
			while (nodeNotSerialized->GetParent() != nullptr)
			{
				nodeNotSerialized = nodeNotSerialized->GetParent();
			}
			nodeNotSerialized->Save(data);
		}
	}
	// Reset saveFlag of all nodes
	for (auto node : nodesList)
	{
		node.second->saveFlag = false;
	}

	// Serialize BTLinks
	data.AppendArray("links");
	for (auto link : linksList)
	{
		link.second->Save(data);
	}

	data.AppendInt("uid", uid);
	data.AppendInt("nextId", nextId);
	if (rootNode)
		data.AppendInt("rootNodeId", rootNode->GetId());
	else
		data.AppendInt("rootNodeId", -1);

	return data.Serialize(buffer);
}

void BehaviorTree::Draw()
{
	// Style Colors
	ne::PushStyleColor(ne::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
	ne::PushStyleColor(ne::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
	ne::PushStyleColor(ne::StyleColor_PinRect, ImColor(60, 180, 255, 150));
	ne::PushStyleColor(ne::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

	// Style Vars
	ne::PushStyleVar(ne::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ne::PushStyleVar(ne::StyleVar_NodeRounding, rounding);
	ne::PushStyleVar(ne::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
	ne::PushStyleVar(ne::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
	ne::PushStyleVar(ne::StyleVar_LinkStrength, 0.0f);
	ne::PushStyleVar(ne::StyleVar_PinBorderWidth, 1.0f);
	ne::PushStyleVar(ne::StyleVar_PinRadius, 5.0f);

	// Prepare the nodes to draw
	for (auto node : nodesList)
	{
		node.second->PrepareToDraw();
	}

	ne::PopStyleVar(7);
	ne::PopStyleColor(4);

	// Actually draw the nodes
	int inputAlpha = 200;
	const ImVec4 pinBackground = ne::GetStyle().Colors[ne::StyleColor_NodeBg];
	int outputAlpha = 200;

	for (auto node : nodesList)
	{
		ImDrawList* drawList = ne::GetNodeBackgroundDrawList(node.second->GetId());

		// Draw Input
		drawList->AddRectFilled(to_imvec(node.second->inputsRect.top_left()) + ImVec2(0, 1), to_imvec(node.second->inputsRect.bottom_right()), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);

		ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(to_imvec(node.second->inputsRect.top_left()) + ImVec2(0, 1), to_imvec(node.second->inputsRect.bottom_right()), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
		ImGui::PopStyleVar();

		// Draw Output
		drawList->AddRectFilled(to_imvec(node.second->outputsRect.top_left()), to_imvec(node.second->outputsRect.bottom_right()) - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);

		ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(to_imvec(node.second->outputsRect.top_left()), to_imvec(node.second->outputsRect.bottom_right()) - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
		ImGui::PopStyleVar();

		// Draw Content
		drawList->AddRectFilled(to_imvec(node.second->contentRect.top_left()), to_imvec(node.second->contentRect.bottom_right()), IM_COL32(24, 64, 128, 200), 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(to_imvec(node.second->contentRect.top_left()), to_imvec(node.second->contentRect.bottom_right()), IM_COL32(48, 128, 255, 100), 0.0f);
		ImGui::PopStyleVar();
	}

	// Draw Links
	for (auto link : linksList)
	{
		ne::Link(link.second->id, link.second->sourcePin->id, link.second->targetPin->id, link.second->color, 2.0f);
	}
}

BTNode * BehaviorTree::FindNode(int id) const
{
	map<int, BTNode*>::const_iterator node = nodesList.find(id);
	if (node == nodesList.end())
	{
		LOGW("Could not find node %i", id);
		return nullptr;
	}
	
	return node->second;
}

BTLink * BehaviorTree::FindLink(int id) const
{
	map<int, BTLink*>::const_iterator link = linksList.find(id);
	if (link == linksList.end())
	{
		return nullptr;
	}
	return link->second;
}

BTPin * BehaviorTree::FindPin(int id) const
{
	for (auto node : nodesList)
	{
		if (node.second->inputPin->id == id)
			return node.second->inputPin;
		if (node.second->outputPin->id == id)
			return node.second->outputPin;
	}
	return nullptr;
}

int BehaviorTree::GetNextId()
{
	return nextId++;
}

void BehaviorTree::AddNode(Data & data, map<int, BTPin*>& pinsList)
{
	BTNode* node = new BTNode(this, data);
	nodesList.insert(pair<int, BTNode*>(node->GetId(), node));
	pinsList.insert(pair<int, BTPin*>(node->inputPin->id, node->inputPin));
	pinsList.insert(pair<int, BTPin*>(node->outputPin->id, node->outputPin));
	int numChilds = data.GetArraySize("childs");
	for (int i = 0; i < numChilds; ++i)
	{
		AddNode(data.GetArray("childs", i), pinsList);
	}
	ne::SetNodePosition(node->GetId(), ImVec2(data.GetFloat("positionX"), data.GetFloat("positionY")));
}
