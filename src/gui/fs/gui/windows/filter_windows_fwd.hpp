#pragma once

#include <fs/gui/imgui_window.hpp>

#include <memory>
#include <string>

namespace fs::gui {

constexpr auto str_real_filter_from_text_input = "Real filter - from text input";
constexpr auto str_spirit_filter_from_text_input = "Spirit filter - from text input";

class application;

#ifndef __EMSCRIPTEN__
std::unique_ptr<imgui_window> real_filter_window_from_file(application& app, std::string path);
std::unique_ptr<imgui_window> spirit_filter_window_from_file(application& app, std::string path);
#endif

std::unique_ptr<imgui_window> real_filter_window_from_text_input(application& app);
std::unique_ptr<imgui_window> spirit_filter_window_from_text_input(application& app);
std::unique_ptr<imgui_window> real_filter_window_from_source(application& app, std::string name, std::string source);
std::unique_ptr<imgui_window> spirit_filter_window_from_source(application& app, std::string name, std::string source);

}
