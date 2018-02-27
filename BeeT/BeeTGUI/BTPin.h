#ifndef __BTPIN_H__
#define __BTPIN_H__

#include <vector>
#include "ThirdParty/NodeEditor/Include/NodeEditor.h"

class BTNode;
class BTLink;

class BTPin
{
public:
	BTPin();
	BTPin(int id, ax::NodeEditor::PinKind kind, BTNode* node);
	~BTPin();

	bool IsLinkAvailable()const;

public:
	int id = -1;
	BTNode* node = nullptr;
	ax::NodeEditor::PinKind kind;
	std::vector<BTLink*> links;
};
#endif // !__BTPIN_H__