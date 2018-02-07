#include "BeeTGui.h"
#include "Application.h"

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_sdl_gl3.h"

namespace ne = ax::NodeEditor;

BeeTGui::BeeTGui(const char* name) : Module(name)
{}

BeeTGui::~BeeTGui()
{}

bool BeeTGui::Init()
{
	editorContext = ne::CreateEditor();

	g_app->AddModuleUpdate(this);

	// Testing

	// Init texture

	return true;
}

bool BeeTGui::CleanUp()
{
	ne::DestroyEditor(editorContext);
	return true;
}

bool BeeTGui::Update()
{
	ImGui::Begin("White Room");
	
	ne::SetCurrentEditor(editorContext);

	ne::Begin("Node editor");

	TreeNode();

	ne::End(); // Node editor

	ImGui::End();
	return true;
}

void BeeTGui::CompleteNode()
{
	ImGui::Begin("MyWindow");

	ne::SetCurrentEditor(editorContext);
	ne::Begin("Test Editor");
	//------------------------------
	int id = 4;
	int input_pin_id = 5;

	//Creation
	HeaderRect = ax::rect();
	ne::PushStyleVar(ne::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));

	ne::BeginNode(id);
	ImGui::PushID(id);

	//Begin
	ImGui::BeginVertical("node");

	//Input
	//StageContent
	ImGui::Spring(0);
	ImGui::BeginHorizontal("content");
	ImGui::Spring(0, 0);
	//StageInput
	ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);
	ne::PushStyleVar(ne::StyleVar_PivotAlignment, ImVec2(0, 0.5f));
	ne::PushStyleVar(ne::StyleVar_PivotSize, ImVec2(0, 0));
	ImGui::Spring(1, 0); //If doesn't have header
						 //Input
	ne::BeginPin(input_pin_id, ne::PinKind::Target);
	ImGui::BeginHorizontal(id);
	//----
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha); //This is probably to show the node disabled when a connection can't be made
	ImGui::Text("ic");//Replace for DrawIcon
	ImGui::Spring(0);
	ImGui::TextUnformatted("input_name");
	ImGui::Spring(0);
	ImGui::PopStyleVar();
	//end input
	ImGui::EndHorizontal();
	ne::EndPin();

	//Simpler node
	//middle
	ne::PopStyleVar(2); //end of input
	ImGui::Spring(1, 0);
	ImGui::EndVertical();

	ImGui::Spring(1);
	ImGui::BeginVertical("middle", ImVec2(0, 0), 1.0f);

	ImGui::Spring(1, 0);
	ImGui::TextUnformatted("Node name");
	ImGui::Spring(1, 0);

	//Let's assume this node has not output connections (for simplicity)
	ImGui::EndVertical(); //end of middle
						  //stage:end
	ImGui::EndHorizontal();
	ContentRect2 = ImGui_GetItemRect();
	ImGui::EndVertical(); //end of node
	NodeRect = ImGui_GetItemRect();

	ne::EndNode();

	if (ImGui::IsItemVisible())
	{
		ItemIsVisible();
	}

	ImGui::PopID();
	ne::PopStyleVar();
	//-----------------------
	ne::End();
	ImGui::End();
}

void BeeTGui::ItemIsVisible()
{
	int alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);
	ImDrawList* drawList = ne::GetNodeBackgroundDrawList(4); //hardcoded node id
	const float halfBorderWidth = ne::GetStyle().NodeBorderWidth * 0.5f;
	unsigned int headerColor = IM_COL32(0, 0, 0, alpha) | (headerColor & IM_COL32(255, 255, 255, 0));
	

	
	auto headerSeparatorRect = ax::rect(HeaderRect.bottom_left(), ContentRect2.top_right());
	auto footerSeparatorRect = ax::rect(ContentRect2.bottom_left(), NodeRect.bottom_right());
	drawList->AddLine(
		to_imvec(headerSeparatorRect.top_left()) + ImVec2(-(8 - halfBorderWidth), -0.5f),
		to_imvec(headerSeparatorRect.top_right()) + ImVec2((8 - halfBorderWidth), -0.5f),
		ImColor(255, 255, 255, 96 * alpha / (3 * 255)), 1.0f);
}

void BeeTGui::TreeNode()
{
	// Hardcoded
	int nodeId = 20;

	const float rounding = 5.0f;
	const float padding = 12.0f;

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

	// Node code
	ne::BeginNode(nodeId);
	TreeNodeContent(nodeId, padding);
	ne::EndNode();

	ne::PopStyleVar(7);
	ne::PopStyleColor(4);

	DrawTreeNode();
}

void BeeTGui::TreeNodeContent(int nodeId, const float padding)
{
	ImGui::BeginVertical(nodeId);

	// Input ---------------------------------
	ImGui::BeginHorizontal("inputs");
	ImGui::Spring(0, padding * 2);

	if (bool nodeHasInput = true)
		TreeNodeContentInput(padding);
	else
		ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal(); // 'inputs'
	// -----------------------------------------

	// Content ---------------------------------
	ImGui::BeginHorizontal("content_frame");
	ImGui::Spring(1, padding);
	
	ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
	ImGui::Dummy(ImVec2(160, 0));

	ImGui::Spring(1);
	ImGui::TextUnformatted("Node's Name");
	ImGui::Spring(1);

	ImGui::EndVertical(); // 'content'

	contentRect = ImGui_GetItemRect();
	ImGui::Spring(1, padding);
	ImGui::EndHorizontal(); // 'content_frame'
	// -------------------------------------------

	// Output ------------------------------------
	ImGui::BeginHorizontal("outputs");
	ImGui::Spring(0, padding * 2);


	// For simplicity, this node has no outputs. Same code as inputs
	ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal(); // 'outputs'
	// -------------------------------------------
	
	ImGui::EndVertical(); // nodeId
}

void BeeTGui::TreeNodeContentInput(const float padding)
{
	// Hardcoded variables
	int pinId = 21;

	// Something about pins
	ImGui::Dummy(ImVec2(0, padding));
	ImGui::Spring(1, 0);
	inputsRect = ImGui_GetItemRect();

	ne::PushStyleVar(ne::StyleVar_PinArrowSize, 10.0f);
	ne::PushStyleVar(ne::StyleVar_PinArrowWidth, 10.0f);
	ne::PushStyleVar(ne::StyleVar_PinCorners, 12);
	ne::BeginPin(pinId, ne::PinKind::Target);
	ne::PinPivotRect(to_imvec(inputsRect.top_left()), to_imvec(inputsRect.bottom_right()));
	ne::PinRect(to_imvec(inputsRect.top_left()), to_imvec(inputsRect.bottom_right()));
	ne::EndPin();
	ne::PopStyleVar(3);
}

void BeeTGui::DrawTreeNode()
{
	int nodeId = 20; // Hardcoded before
	int inputAlpha = 200;
	const ImVec4 pinBackground = ne::GetStyle().Colors[ne::StyleColor_NodeBg];
	int outputAlpha = 200;

	ImDrawList* drawList = ne::GetNodeBackgroundDrawList(nodeId);

	// Draw Input
	drawList->AddRectFilled(to_imvec(inputsRect.top_left()) + ImVec2(0, 1), to_imvec(inputsRect.bottom_right()), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
	
	ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(to_imvec(inputsRect.top_left()) + ImVec2(0, 1), to_imvec(inputsRect.bottom_right()), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
	ImGui::PopStyleVar();
	
	// Draw Output
	drawList->AddRectFilled(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()) - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
	
	ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(to_imvec(outputsRect.top_left()), to_imvec(outputsRect.bottom_right()) - ImVec2(0, 1), IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
	ImGui::PopStyleVar();
	
	// Draw Content
	drawList->AddRectFilled(to_imvec(contentRect.top_left()), to_imvec(contentRect.bottom_right()), IM_COL32(24, 64, 128, 200), 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(to_imvec(contentRect.top_left()), to_imvec(contentRect.bottom_right()), IM_COL32(48, 128, 255, 100), 0.0f);
	ImGui::PopStyleVar();
}