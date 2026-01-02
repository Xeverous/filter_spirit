#include <fs/gui/settings/network_settings.hpp>
#include <fs/gui/auxiliary/widgets.hpp>

#include <imgui.h>

namespace fs::gui {

void network_settings::draw_interface()
{
	ImGui::TextWrapped(
		"These options are only relevant for spirit filter templates that "
		"are using market data downloaded from external sites.");

	// use a temporary int variable because Dear ImGui wants int, not long
	int timeout = _download_settings.timeout_milliseconds;
	ImGui::DragInt("Timeout (use 0 to never timeout)", &timeout, 10.0f, 0, INT_MAX, "%d milliseconds", ImGuiSliderFlags_AlwaysClamp);
	_download_settings.timeout_milliseconds = timeout;

	ImGui::DragInt("Max age for market data", &_max_market_data_age_min, 0.25f, 60, INT_MAX, "%d minutes", ImGuiSliderFlags_AlwaysClamp);
	aux::on_hover_text_tooltip(
		"Upon spirit filter regeneration, market data older than specified time "
		"will automatically be redownloaded.");

	ImGui::Checkbox("SSL - verify peer", &_download_settings.ssl_verify_peer);
	aux::on_hover_text_tooltip("Verify the authenticity of the peer's certificate.");

	ImGui::Checkbox("SSL - verify host", &_download_settings.ssl_verify_host);
	aux::on_hover_text_tooltip("Verify certificate's name against host name.");

	aux::input_string("Certificate Authority bundle path", _download_settings.ca_info_path);
	aux::on_hover_text_tooltip(network::strings::ca_bundle_desc);

	aux::input_string("Proxy", _download_settings.proxy);
	aux::on_hover_text_tooltip(network::strings::proxy_desc);
}

}
