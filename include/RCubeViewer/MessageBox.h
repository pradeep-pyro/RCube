#pragma once

#include "imgui.h"
#include <string>

namespace rcube
{
namespace viewer
{

void messageBoxError(const std::string &title, const std::string &msg)
{
    ImGui::OpenPopup(title.c_str());
    bool open = true;
    if (ImGui::BeginPopupModal(title.c_str(), &open))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        ImGui::Text(msg.c_str());
        ImGui::PopStyleColor();
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void messageBoxWarning(const std::string &title, const std::string &msg)
{
    ImGui::OpenPopup(title.c_str());
    bool open = true;
    if (ImGui::BeginPopupModal(title.c_str()))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.65f, 0, 1));
        ImGui::Text(msg.c_str());
        ImGui::PopStyleColor();
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void messageBoxInfo(const std::string &title, const std::string &msg)
{
    ImGui::OpenPopup(title.c_str());
    bool open = true;
    if (ImGui::BeginPopupModal(title.c_str()))
    {
        ImGui::Text(msg.c_str());
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void messageBoxOKCancel(const std::string &title, const std::string &msg, bool &ok)
{
    ImGui::OpenPopup(title.c_str());
    bool open = true;
    if (ImGui::BeginPopupModal(title.c_str()))
    {
        ImGui::Text(msg.c_str());
        if (ImGui::Button("OK"))
        {
            ok = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel"))
        {
            ok = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace viewer
} // namespace rcube