#pragma once

#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QCheckBox>
#include "UI/SpinEditInput.h"

namespace CNCSYS
{
	enum OffsetType
	{
		OFFSET,
		SPACING
	};

	enum RowDirection{
		UP,
		DOWN
	};
	enum ColDirection {
		LEFT,
		RIGHT
	};

	struct RectArrayParam
	{
		int rowCount = 2;
		int colCount = 2;
		float offsetRow = 10;
		float offsetCol = 10;
		RowDirection rowDir = RowDirection::UP;
		ColDirection colDir = ColDirection::LEFT;
	};

	class RectArrayGenDlg : public QDialog
	{
		Q_OBJECT
	public:
		static RectArrayGenDlg* GetInstance();

	signals:
		void ConfirmData(RectArrayParam param);

	private:
		RectArrayGenDlg(QWidget* parent = nullptr);
		~RectArrayGenDlg();
	
	public:
		static RectArrayGenDlg* instance;
		int rowCount;
		int colCount;
		int rowOffset;
		int colOffset;

		SpinEditInput* rowCountEdit = nullptr;
		SpinEditInput* colCountEdit = nullptr;
		QRadioButton* checkOffset = nullptr;
		QRadioButton* checkSpacing = nullptr;
		SpinEditInput* rowOffsetBox = nullptr;
		SpinEditInput* colOffsetBox = nullptr;
		QRadioButton* lineDirUp = nullptr;
		QRadioButton* lineDirDown = nullptr;
		QRadioButton* lineDirLeft = nullptr;
		QRadioButton* lineDirRight = nullptr;

		QPushButton* btnOK = nullptr;
		QPushButton* btnCancel = nullptr;
	};

	enum RingArrayType
	{
		BaseOnSpacing,
		BaseOnRange
	};

	struct RingArrayParam
	{
		int itemCount = 6;
		int angleSpacing = 30;
		int angleRange = 180;
		int radius = 20;
		int startupAngle = 0;
		bool setCenterParam = true;
		RingArrayType RingType = RingArrayType::BaseOnSpacing;
	};

	class RingArrayGenDlg : public QDialog
	{
		Q_OBJECT
	public:
		static RingArrayGenDlg* GetInstance();

	signals:
		void ConfirmData(RingArrayParam param);
	private:
		RingArrayGenDlg(QWidget* parent = nullptr);
		~RingArrayGenDlg();
	private:
		static RingArrayGenDlg* instance;

		int itemCount;
		int angleSpacing;
		int angleRange;
		int radius;
		int startupAngle;

		bool setCenterParam = true;

		SpinEditInput* itemCountEdit = nullptr;
		SpinEditInput* angleSpacingEdit = nullptr;
		SpinEditInput* angleRangeEdit = nullptr;
		SpinEditInput* radiusEdit = nullptr;
		SpinEditInput* startupAngleEdit = nullptr;
		QCheckBox* setCenterCbx = nullptr;
		QRadioButton* checkBySpacingBtn = nullptr;
		QRadioButton* checkByRangeBtn = nullptr;
	};
}