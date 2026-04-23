#pragma once

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QPushButton>
#include <Graphics/Text.h>

class FontAttributePanel : public QWidget
{
	Q_OBJECT
public:
	static FontAttributePanel* GetInstance();
	FontConfig* GetConfig();
	void SetAttachedText(Text* text);
private:
	FontAttributePanel();
	~FontAttributePanel();
	
public:
	static FontAttributePanel* s_instance;
	QPushButton* applyBtn{ nullptr };
	QPushButton* closeBtn{ nullptr };
	QComboBox *fontTypeCombo{ nullptr };
	QTextEdit* textEidt{ nullptr };
	QLineEdit* Xposition{ nullptr };
	QLineEdit* Yposition{ nullptr };
	QLineEdit* Xdimension{ nullptr };
	QLineEdit* Ydimension{ nullptr };
	QLineEdit* Fontsapcing{ nullptr };
	QLineEdit* RowSpacing{ nullptr };

	Text* attachedText{ nullptr };
};