#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateMsgBox.h"

#include <QFont>
#include <QFontDatabase>
#include <iostream>
int screen_resolution_x = 0;
int screen_resolution_y = 0;
float blank_height_ratio = 0.58;
float canvas_panel_width_ratio = 0.40;
float canvas_panel_height_ratio = 0.60;
float gcode_panel_width_ratio = 0.58;
float gcode_panel_height_ratio = 0.50;
float move_height_ratio = 0.1625;
float taskpanel_width_ratio = 0.315;
float taskpanel_height_ratio = 0.74;
float todoCanvas_width_ratio = 0.053;
float todoCanvas_height_ratio;
float fixed_canvas_aspect;
int canvasAnchorX = 0;
int canvasAnchorY = 0;

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
