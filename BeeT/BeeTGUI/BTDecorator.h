#ifndef __BTDECORATOR_H__
#define __BTDECORATOR_H__

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/NodeEditor/Source/Shared/Math2D.h"
#include "Data.h"
#include <boost/any.hpp>

#define DEC_TICK_ONCE_COLOR ImVec4(192, 160, 239, 200)
#define DEC_TICK_ALWAYS_COLOR ImVec4(79, 134, 242, 200)
#define DEC_PASS_COLOR ImVec4(30, 104, 52, 200)
#define DEC_NO_PASS_COLOR ImVec4(104, 30, 30, 200)

// TODO: Different types of decorators. For now only Blackboard comparisons
class BTNode;
class Blackboard;
struct BBVar;

enum DecBoolOpt
{
	IS_FALSE = 0,
	IS_TRUE = 1
};

enum DecNumberOpt
{
	IS_EQUAL = 0,
	IS_NOT_EQUAL = 1,
	IS_GREATER = 2,
	IS_LESS = 3,
	IS_EQUAL_OR_GREATER = 4,
	IS_EQUAL_OR_LESS = 5
};

enum DecStringOpt
{
	S_IS_EQUAL = 0,
	S_IS_NOT_EQUAL = 1,
	CONTAINS = 2,
	NOT_CONTAINS = 3
};

class BTDecorator
{
public:
	BTDecorator(BTNode* node, Blackboard* bb, BBVar* var);
	BTDecorator(BTNode* node, Blackboard* bb, Data& data);
	~BTDecorator();

	void PrepareToDraw();
	void InspectorInfo();

	void Save(Data& file);

	void Remove(bool isBBVarRemoved = false);
	void CleanUp();

	int GetUID()const;
	bool IsCheckedEveryFrame()const;

private:
	void PrintType()const;

	// Prepare to draw by type
	void PrepDrawBool();
	void PrepDrawInt();
	void PrepDrawFloat();
	void PrepDrawString();
	void PrepDrawVector2();
	void PrepDrawVector3();

	// Type Options
	void TypeBoolOptions();
	void TypeNumberOptions(int numType); // 0-int 1-float
	void TypeStringOptions();
	void TypeVector2Options();
	void TypeVector3Options();

public:
	ax::rect contentRect;
	ImVec4 bgColor;
private:
	int uid;
	BTNode* nodeAttachedTo = nullptr;
	Blackboard* bb = nullptr;
	BBVar* var = nullptr;
	
	boost::any var2;

	int option = 0;
	bool checkEveryFrame = false;
};
#endif // !__BTDECORATOR_H__
