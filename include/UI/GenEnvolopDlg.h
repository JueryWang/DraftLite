#pragma once

#include <QWidget>

namespace CNCSYS
{
	class GenEnvolopDialog : public QWidget
	{
	public:
		GenEnvolopDialog(QWidget* parent = nullptr);
		~GenEnvolopDialog();

	private:
		float partInterval = 20.0;
		bool isValidForall;
		bool isClearCurrent;

		float expand = 0.5f;
		float smooth = 1.0f;
		bool isSmooth;
		float smoothPercision = 0.1;
	};
}