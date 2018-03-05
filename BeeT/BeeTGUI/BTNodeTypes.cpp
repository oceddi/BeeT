#include "BTNodeTypes.h"
#include "Log.h"

using namespace std;

NodeType::NodeType()
{}

NodeType::NodeType(int typeId, std::string name, int maxOutputs) : typeId(typeId), name(name), maxOutputs(maxOutputs)
{}

BTNodeTypes::BTNodeTypes()
{
}

BTNodeTypes::~BTNodeTypes()
{}

void BTNodeTypes::Init()
{
	// Default node types
	NodeType root(0, "Root", 1);
	NodeType sequence(1, "Sequence", -1);
	NodeType action(2, "Action", 0);
	NodeType condition(3, "Condition", 1);

	typesList.push_back(root);
	typesList.push_back(sequence);
	typesList.push_back(action);
	typesList.push_back(condition);
}

NodeType * BTNodeTypes::GetTypeById(int id)
{
	if ((unsigned int)id >= typesList.size() || id < 0)
	{
		LOGW("Couldn't find node type with id: %", id);
		return nullptr;
	}
	
	return &typesList[id];
}

std::vector<NodeType> BTNodeTypes::GetTypeList() const
{
	return typesList;
}


