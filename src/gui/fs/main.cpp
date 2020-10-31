#include <fs/gui/application.hpp>

int main(int argc, char** argv)
{
	fs::gui::application app({argc, argv});
	return app.exec();
}
