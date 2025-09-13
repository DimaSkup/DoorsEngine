#include "MainMenuBar.h"
#include <imgui.h>



namespace UI
{

// =================================================================================
//                              public methods
// =================================================================================

void MainMenuBar::RenderBar()
{
    // create a window called "Main menu bar" with a main menu 
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open..", "Ctrl+O"))
            {
                // TODO: make opening of the engine project
            }
            else if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                // TODO:: make saving of the engine project
            }
            else if (ImGui::MenuItem("Exit"))
            {
                // TODO: execute exit from the engine
            }

            ImGui::EndMenu();
        }

        // --------------------------------------------

        if (ImGui::BeginMenu("View"))
        {
            // menu items to open particular windows
            ImGui::MenuItem("Textures browser",  NULL, &g_GuiStates.showWndTexturesBrowser);
            ImGui::MenuItem("Materials browser", NULL, &g_GuiStates.showWndMaterialsBrowser);
            ImGui::MenuItem("Models browser",    NULL, &g_GuiStates.showWndModelsBrowser);
            ImGui::EndMenu();
        }

        // --------------------------------------------

        if (ImGui::BeginMenu("Create"))
        {
            // create a modal window for entities creation
            ImGui::MenuItem("Entity", NULL, &g_GuiStates.showWndEnttCreation);
            ImGui::EndMenu();
        }

        // --------------------------------------------

        // setup color for the button as like it is a usual menu bar elements
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));

        if (ImGui::Button("Options"))
        {
            // create a window for control common engine options
            g_GuiStates.showWndEngineOptions = !g_GuiStates.showWndEngineOptions;
        }

        ImGui::PopStyleColor();


        ImGui::EndMainMenuBar();
    }
}

///////////////////////////////////////////////////////////

void MainMenuBar::RenderWndEngineOptions(bool* pOpen)
{
    // open a separate window with the engine options

    if (ImGui::Begin("Engine Options", pOpen))
    {
        if (ImGui::BeginTabBar("##TabBarEngineOptions"))
        {
            if (ImGui::BeginTabItem("GUI"))
            {
                ShowOptionsGui();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("OptionsTab2"))  // for testing
            {
                ImGui::Text("Test tab 2");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("OptionsTab3"))  // for testing
            {
                ImGui::Text("Test tab 3");
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}




// =================================================================================
//                              private methods
// =================================================================================

void MainMenuBar::ShowOptionsGui()
{
    ImVec4* colors = ImGui::GetStyle().Colors;

    ImGui::ColorEdit3("Text",                   &colors[ImGuiCol_Text].x);

    // background color selector
    ImGui::ColorEdit3("Window",                 &colors[ImGuiCol_WindowBg].x);              // Background of normal windows
    ImGui::ColorEdit3("Child",                  &colors[ImGuiCol_ChildBg].x);               // Background of child windows
    ImGui::ColorEdit3("Popup",                  &colors[ImGuiCol_PopupBg].x);               // Background of popups, menus, tooltips windows

    // border color selector (for the main window, child windows, etc.)
    ImGui::ColorEdit3("Border",                 &colors[ImGuiCol_Border].x);
    ImGuiCol_BorderShadow,

    ImGui::ColorEdit3("Frame",                  &colors[ImGuiCol_FrameBg].x);               // Background of checkbox, radio button, plot, slider, text input
    ImGui::ColorEdit3("Frame hovered",          &colors[ImGuiCol_FrameBgHovered].x);
    ImGui::ColorEdit3("Frame bg active",        &colors[ImGuiCol_FrameBgActive].x);

    ImGui::ColorEdit3("Title",                  &colors[ImGuiCol_TitleBg].x);               // Title bar
    ImGui::ColorEdit3("Title bg active",        &colors[ImGuiCol_TitleBgActive].x);         // Title bar when focused
    ImGui::ColorEdit3("Title bg collapsed",     &colors[ImGuiCol_TitleBgCollapsed].x);      // Title bar when collapsed

    ImGui::ColorEdit3("Menu bar bg",            &colors[ImGuiCol_MenuBarBg].x);
    ImGui::ColorEdit3("Scrollbar bg",           &colors[ImGuiCol_ScrollbarBg].x);
    ImGui::ColorEdit3("Scrollbar grab",         &colors[ImGuiCol_ScrollbarGrab].x);

    ImGui::ColorEdit3("ScrollbarGrabHovered",   &colors[ImGuiCol_ScrollbarGrabHovered].x);
    ImGui::ColorEdit3("ScrollbarGrabActive",    &colors[ImGuiCol_ScrollbarGrabActive].x);
    ImGui::ColorEdit3("CheckMark",              &colors[ImGuiCol_CheckMark].x);             // Checkbox tick and RadioButton circle

    ImGui::ColorEdit3("SliderGrab",             &colors[ImGuiCol_SliderGrab].x);
    ImGui::ColorEdit3("SliderGrabActive",       &colors[ImGuiCol_SliderGrabActive].x);
    ImGui::ColorEdit3("Button",                 &colors[ImGuiCol_Button].x);

    ImGui::ColorEdit3("ButtonHovered",          &colors[ImGuiCol_ButtonHovered].x);
    ImGui::ColorEdit3("ButtonActive",           &colors[ImGuiCol_ButtonActive].x);
    ImGui::ColorEdit3("Header",                 &colors[ImGuiCol_Header].x);                // Header colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem

    ImGui::ColorEdit3("HeaderHovered",          &colors[ImGuiCol_HeaderHovered].x);
    ImGui::ColorEdit3("HeaderActive",           &colors[ImGuiCol_HeaderActive].x);
    ImGui::ColorEdit3("Separator",              &colors[ImGuiCol_Separator].x);
    ImGui::ColorEdit3("SeparatorHovered",       &colors[ImGuiCol_SeparatorHovered].x);

    ImGui::ColorEdit3("SeparatorActive",        &colors[ImGuiCol_SeparatorActive].x);
    ImGui::ColorEdit3("ResizeGrip",             &colors[ImGuiCol_ResizeGrip].x);            // Resize grip in lower-right and lower-left corners of windows.
    ImGui::ColorEdit3("ResizeGripHovered",      &colors[ImGuiCol_ResizeGripHovered].x);
    ImGui::ColorEdit3("ResizeGripActive",       &colors[ImGuiCol_ResizeGripActive].x);

    // tab stuff
    ImGui::ColorEdit3("TabHovered",             &colors[ImGuiCol_TabHovered].x);            // Tab background, when hovered
    ImGui::ColorEdit3("Tab",                    &colors[ImGuiCol_Tab].x);                   // Tab background, when tab-bar is focused & tab is unselected
    ImGui::ColorEdit3("TabSelected",            &colors[ImGuiCol_TabSelected].x);           // Tab background, when tab-bar is focused & tab is selected
    ImGui::ColorEdit3("TabSelectedOverline",    &colors[ImGuiCol_TabSelectedOverline].x);   // Tab horizontal overline, when tab-bar is focused & tab is selected
    ImGui::ColorEdit3("TabDimmed",              &colors[ImGuiCol_TabDimmed].x);             // Tab background, when tab-bar is unfocused & tab is unselected
    ImGui::ColorEdit3("TabDimmedSelected",      &colors[ImGuiCol_TabDimmedSelected].x);     // Tab background, when tab-bar is unfocused & tab is selected

    ImGui::ColorEdit3("TabDimmedSelectedOverline", &colors[ImGuiCol_TabDimmedSelectedOverline].x); //..horizontal overline, when tab-bar is unfocused & tab is selected
    ImGui::ColorEdit3("DockingPreview",         &colors[ImGuiCol_DockingPreview].x);        // Preview overlay color when about to docking something
    ImGui::ColorEdit3("DockingEmptyBg",         &colors[ImGuiCol_DockingEmptyBg].x);        // Background color for empty node (e.g. CentralNode with no window docked into it)
    ImGui::ColorEdit3("PlotLines",              &colors[ImGuiCol_PlotLines].x);
    ImGui::ColorEdit3("PlotLinesHovered",       &colors[ImGuiCol_PlotLinesHovered].x);
    ImGui::ColorEdit3("PlotHistogram",          &colors[ImGuiCol_PlotHistogram].x);
    ImGui::ColorEdit3("PlotHistogramHovered",   &colors[ImGuiCol_PlotHistogramHovered].x);

    ImGui::ColorEdit3("TableHeaderBg",          &colors[ImGuiCol_TableHeaderBg].x);         // Table header background
    ImGui::ColorEdit3("TableBorderStrong",      &colors[ImGuiCol_TableBorderStrong].x);     // Table outer and header borders (prefer using Alpha=1.0 here)
    ImGui::ColorEdit3("TableBorderLight",       &colors[ImGuiCol_TableBorderLight].x);      // Table inner borders (prefer using Alpha=1.0 here)

    ImGui::ColorEdit3("TableRowBg",             &colors[ImGuiCol_TableRowBg].x);            // Table row background (even rows)
    ImGui::ColorEdit3("TableRowBgAlt",          &colors[ImGuiCol_TableRowBgAlt].x);         // Table row background (odd rows)
    ImGui::ColorEdit3("TextLink",               &colors[ImGuiCol_TextLink].x);              // Hyperlink color
    ImGui::ColorEdit3("TextSelectedBg",         &colors[ImGuiCol_TextSelectedBg].x);

    ImGui::ColorEdit3("DragDropTarget",         &colors[ImGuiCol_DragDropTarget].x);        // Rectangle highlighting a drop target
    ImGui::ColorEdit3("NavCursor",              &colors[ImGuiCol_NavCursor].x);             // Color of keyboard/gamepad navigation cursor/rectangle, when visible
    ImGui::ColorEdit3("NavWindowingHighlight",  &colors[ImGuiCol_NavWindowingHighlight].x); // Highlight window when using CTRL+TAB
    ImGui::ColorEdit3("NavWindowingDimBg",      &colors[ImGuiCol_NavWindowingDimBg].x);     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    ImGui::ColorEdit3("ModalWindowDimBg",       &colors[ImGuiCol_ModalWindowDimBg].x);      // Darken/colorize entire screen behind a modal window, when one is active

    // font selector
    ShowFontSelector();
}

///////////////////////////////////////////////////////////

void MainMenuBar::ShowFontSelector()
{
    // change GUI (ImGui) font
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font_current = ImGui::GetFont();
    if (ImGui::BeginCombo("Fonts", font_current->GetDebugName()))
    {
        for (ImFont* font : io.Fonts->Fonts)
        {
            ImGui::PushID((void*)font);
            if (ImGui::Selectable(font->GetDebugName(), font == font_current))
                io.FontDefault = font;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
}

} // namespace UI
