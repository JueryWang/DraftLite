#pragma once

#include "UI/GLWidget.h"
#include <QLabel>
#include <QListView>
#include <QGridLayout>
#include <Graphics/Canvas.h>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QProxyStyle>
#include <QStyleOptionTab>
#include <vector>
#include <QStylePainter>
#include <Qsci/qsciscintilla.h>
#include "UI/MenuLayerTop.h"
#include "UI/GCodeEditor.h"
#include "UI/SketchInformation.h"
using namespace CNCSYS;

class PreviewItem : public QWidget
{
public:
	PreviewItem(GLWidget* preview,int id = 0);
	~PreviewItem();
	GLWidget* canvas = nullptr;
	GCodeEditor* editor = nullptr;
	SketchInfoPanel* infoPanel = nullptr;
};

class MainLayer;

class StationSwitchTab : public QTabBar
{
    Q_OBJECT
public:
	StationSwitchTab(MainLayer* parent);
	~StationSwitchTab();

public slots:
    void SwitchStation(int index);

protected:
    // 重写绘制事件：强制文字横向绘制
    void paintEvent(QPaintEvent* event) override {
        QStylePainter painter(this);
        QStyleOptionTab opt;

        for (int i = 0; i < count(); i++) {
            initStyleOption(&opt, i);
            // 关键1：取消默认的文字旋转/竖排
            opt.shape = QTabBar::RoundedNorth; // 强制使用顶部标签的绘制逻辑
            opt.direction = Qt::LeftToRight; // 文字方向：从左到右

            // 绘制标签（边框+背景）
            painter.drawControl(QStyle::CE_TabBarTabShape, opt);
            // 绘制文字（强制横向）
            painter.drawControl(QStyle::CE_TabBarTabLabel, opt);
        }
    }

    // 重写尺寸计算：保证标签宽度足够容纳文字
    QSize tabSizeHint(int index) const override {
        QSize size = QTabBar::tabSizeHint(index);
        // 强制标签宽度（根据文字长度调整，比如“工位10”需要80px）
        size.setWidth(80);
        // 标签高度（单行文字高度）
        size.setHeight(50);
        return size;
    }
public:
    MainLayer* holder = nullptr;
};

