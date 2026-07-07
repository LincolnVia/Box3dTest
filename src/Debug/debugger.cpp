#include "debugger.hpp"
#include <imgui.h>


void Debugger::Init()
{
     rlImGuiSetup(true); 	
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
   
}
Debugger::~Debugger()
{
    rlImGuiShutdown();
}

void Debugger::Render()
{
    rlImGuiBegin();			
    // Create a window called "My First Tool", with a menu bar.
ImGui::Begin("My First Tool");


// Display contents in a scrolling region
ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
ImGui::BeginChild("Scrolling");
for (int n = 0; n < 50; n++)
    ImGui::Text("%04d: Some text", n);
ImGui::EndChild();
ImGui::End();
rlImGuiEnd();
}
void Debugger::Update()
{}