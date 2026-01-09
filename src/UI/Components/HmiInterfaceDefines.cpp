#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateMsgBox.h"

#include <QFont>
#include <QFontDatabase>
#include <iostream>
int screen_resolution_x = 0;
int screen_resolution_y = 0;
float canvas_panel_width_ratio = 0.48;
float canvas_panel_height_ratio = 0.70;
float gcode_panel_width_ratio = 0.45;
float gcode_panel_height_ratio = 0.63;
float move_height_ratio = 0.1625;

QMap<QString, QString> global_font_mp;

void InitUIEnvironment()
{
	int fontId = QFontDatabase::addApplicationFont(FONTPATH(Cascadia.ttf));
	QStringList font_list = QFontDatabase::applicationFontFamilies(fontId);
	global_font_mp["Cascadia"] = font_list[0];

	fontId = QFontDatabase::addApplicationFont(FONTPATH(Caviar_Dreams_Bold.ttf));
	font_list = QFontDatabase::applicationFontFamilies(fontId);
	global_font_mp["Caviar_Dreams_Bold"] = font_list[0];

	fontId = QFontDatabase::addApplicationFont("C:/Windows/Fonts/comic.ttf");
	font_list = QFontDatabase::applicationFontFamilies(fontId);
	global_font_mp["Comic"] = font_list[0];
}
