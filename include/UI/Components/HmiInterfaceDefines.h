#pragma once

#include <QColor>
#include <QString>
#include <QMap>

#define VariantChangeWarning QColor(246,230,65) 
#define ICOPATH(x) "Resources/icon/"#x
#define FONTPATH(x) "Resources/font/"#x
#define EDITOR_LANG_CONFIG_PATH(x) "Resources/editor/intellisense/"#x

extern int screen_resolution_x;
extern int screen_resolution_y;
extern int canvasAnchorX;
extern int canvasAnchorY;
extern float blank_height_ratio;
extern float canvas_panel_width_ratio;
extern float canvas_panel_height_ratio;
extern float gcode_panel_width_ratio;
extern float gcode_panel_height_ratio;
extern float move_height_ratio;
extern float taskpanel_width_ratio;
extern float taskpanel_height_ratio;
extern float todoCanvas_width_ratio;
extern float todoCanvas_height_ratio;
extern float fixed_canvas_aspect;
extern QMap<QString, QString> global_font_mp;

constexpr float msgbox_width_refactor = 0.243;                         //470
constexpr float msgbox_height_refactor = 0.108;                        //120
constexpr float msgbox_icon_width_refactor = 0.036;                    //65
constexpr float msgbox_icon_height_refactor = 0.060;                   //65

void InitUIEnvironment();

static QString GroupBoxStyle = R"(
	.QGroupBox{
		border: 1px solid rgba(80,96,110,155);
		margin-top: 15px;
	}
	.QGroupBox::title {
		top: -6px;
		left: 10px;
	}
)";

static QString ButtonReleasingStyle = R"(
	.QPushButton#hmiTemplate
	{
		background: #6a7583;
		color: whilte;
		border-radius: 20%;
	}
	.QPushButton#hmiTemplate:hover,
	.QPushButton#hmiTemplate:selected
	{
		background-color: #cacfd4;
	}
	.QPushButton#hmiTemplate:pressed
	{
		background-color: #c2db53;
	}
)";

static QString ButtonPressingStyle = R"(
	QPushButton#hmiTemplate
	{
		background: #d4ff16;
		color: whilte;
		border-radius: 20%;
	}
	QPushButton#hmiTemplate:hover,
	QPushButton#hmiTemplate:selected
	{
		background-color: #cacfd4;
	}
	QPushButton#hmiTemplate:pressed
	{
		background-color: #6a7583;
	}
)";