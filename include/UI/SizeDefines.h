#pragma once
#include "Components/HmiInterfaceDefines.h"

#define BLANK_HEIGHT_RATIO 0.58
#define DrawPanelWidth_Ratio 0.48
#define DrawPanelHeight_Ratio 0.50
#define GCodePanelWidth_Ratio (0.90 - DrawPanelWidth_Ratio)
#define GCodePanelWidth_Ratio (0.90 - DrawPanelWidth_Ratio)
#define SPACERITEM_HEIGHT_RATIO 0.060
#define GCodePanelHeight_Ratio DrawPanelHeight_Ratio
#define MotionControlPanelWith_Ratio 1.0
#define MotionControlPanelHeight_Ratio 0.32
#define MotionNormalButtonWidth_Ratio 0.10
#define MotionNormalButtonHeight_Ratio (MotionControlPanelHeight_Ratio * 0.1333)
#define MotionNormalLabelWidth (MotionControlPanelWith_Ratio * 0.0781)
#define MotionNoarmalLabelHeight (MotionControlPanelHeight_Ratio * 0.0642)
#define MarchingPressButtonWidth_Ratio 0.15
#define MarchingPressButtonHeight_Ratio 0.1
#define TaskListPanelWidth_Ratio 0.25
#define MotionPanelWith_Ratio 1.0
#define MotionPanelHeight_Ratio 0.42
#define TaskWindowPanelWidth_Ratio 0.315
#define TaskWindowPanelHeight_Ratio 0.74
#define ICON_SIZE_RATIO 0.037

#define ScreenSizeHintX(hintX) screen_resolution_x * hintX
#define ScreenSizeHintY(hintY) screen_resolution_y * BLANK_HEIGHT_RATIO * hintY
#define IConSizeHint(hint) min(screen_resolution_x,screen_resolution_y) * hint