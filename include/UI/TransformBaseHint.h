#pragma once

#include "glm/glm.hpp"
#include <QWidget>
#include <QLabel>
#include <QKeyEvent>
#include <QDoubleSpinBox>

namespace CNCSYS
{
	class TransformBaseHint;
	enum class EditSpinType
	{
		X,
		Y
	};

	class MouseHoverEditSpin : public QDoubleSpinBox
	{
		Q_OBJECT
	public:
		MouseHoverEditSpin(TransformBaseHint* parent, EditSpinType type);

	signals:
		void ConfirmData(const glm::vec3& p);

	protected:
		void keyPressEvent(QKeyEvent* event);
		void focusInEvent(QFocusEvent* event) override;
		void focusOutEvent(QFocusEvent* event) override;

		TransformBaseHint* baseHint;
		EditSpinType type;
	};

	class TransformBaseHint : public QWidget
	{
		Q_OBJECT
	public:
		TransformBaseHint(QWidget* parent = nullptr);
		~TransformBaseHint();

		void ResetToRotate();
		void ResetToScale();
		void ResetToMirror();

	public:
		QLabel* hintLabel = nullptr;
		MouseHoverEditSpin* mouseSpinX = nullptr;
		MouseHoverEditSpin* mouseSpinY = nullptr;
	};
}