#include "UI/SketchInformation.h"
#include "Graphics/Sketch.h"
#include <QGraphicsDropShadowEffect>
#include <qgroupbox.h>
#include "UI/TaskFlowGuide.h"
#include "UI/Configer/FontAttributePanel.h"
#include "UI/SizeDefines.h"
#include "Controls/ScadaScheduler.h"

int calculateRequiredWidth(const QString& maxTitle, const QString& maxValue, const QString& maxUnit, const QFont& font) {
    QFontMetrics fm(font);

    // 1. 计算每一列的最大宽度
    int col0_width = fm.horizontalAdvance(maxTitle); // 标题列
    int col1_width = fm.horizontalAdvance(maxValue); // 数值列
    int col2_width = fm.horizontalAdvance(maxUnit);  // 单位列

    // 2. 加上布局的间距 (Margins & Spacing)
    // 根据你的代码：layout->setContentsMargins(10, 8, 10, 8);
    // layout->setSpacing(6); 假设列间距为 6
    int margins = 10 + 10; // 左右边距
    int spacings = 6 * 2;  // 两处列间距

    // 3. 加上 QGroupBox 自身的 padding (来自 QSS)
    int padding = 10; // padding-left + padding-right

    return col0_width + col1_width + col2_width + margins + spacings + padding + 10; // +10 作为安全余量
}

SketchInfoPanel::SketchInfoPanel(QWidget* parent)
{
    setupUI();
    applyStyle();
}

SketchInfoPanel::~SketchInfoPanel()
{

}

void SketchInfoPanel::updateStats(CNCSYS::SketchGPU* sketch)
{
    static QList<QLabel*> label = { labelEntities,labelContours,labelTotalPath ,labelIdlePath,labelDimension };
    labelEntities->setText(QString::number(sketch->keyparams.entitySize));
    labelContours->setText(QString::number(sketch->keyparams.contourSize));
    labelTotalPath->setText(QString::asprintf("%.2f", sketch->keyparams.pathLength));
    labelIdlePath->setText(QString::asprintf("%.2f", sketch->keyparams.idleLength));
    labelDimension->setText(QString::asprintf("%d x %d", sketch->keyparams.dimensionWidth, sketch->keyparams.dimensionHeight));
    double ratio = (sketch->keyparams.pathLength > 0) ? (sketch->keyparams.idleLength / sketch->keyparams.pathLength * 100.0) : 0.0;
    labelIdleRatio->setText(QString::asprintf("%.1f", ratio));
    
    QFont font = groupBox->font();
    // 如果 QSS 中设置了更大的字号，需要手动调整 font
    font.setPointSize(12);
    font.setBold(true);

    QString longestTitle = "图元数量";
    QString longestValue;
    QString longestUnit = "mm";
    for (QLabel* lb : label)
    {
        if (lb->text().size() > longestValue.size())
        {
            longestValue = lb->text();
        }
    }
    labelSource->setText(QString::fromLocal8Bit(sketch->source.c_str()));
    int totalWidth = calculateRequiredWidth(longestTitle, longestValue, longestUnit, font);
    this->setFixedWidth(totalWidth);
}

void SketchInfoPanel::setupUI()
{
    groupBox = new QGroupBox("图纸信息",this);
    groupBox->setStyleSheet(R"(
        QGroupBox {
            border: 2px solid #2E97FF;
            border-radius: 8px;
            /* 增加顶部间距，确保内部布局不挤压标题 */
            margin-top: 12px; 
            padding-top: 15px;
            padding-left: 10px;
            padding-right: 10px;
            padding-bottom: 10px;
            font-size: 14px;
            font-weight: bold;
            color: #333333;
        }

        QGroupBox::title {
            subcontrol-origin: margin; /* 改为 margin 更好控制 */
            subcontrol-position: top left;
            left: 15px;
            top: 0px; /* 配合 margin-top 使用 */
            background-color: #f5f5f5; /* 确保与界面背景色一致 */
            padding: 0 5px 0 5px;
            color: #2E97FF;
        }
    )");

    auto* layout = new QGridLayout(groupBox);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setVerticalSpacing(6);

    auto addInfoRow = [&](const QString& title, QLabel*& valueLabel, const QString& unit, int row) {
        auto* tLabel = new QLabel(title, this);
        tLabel->setObjectName("titleLabel");
        tLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        valueLabel = new QLabel("0", this);
        valueLabel->setObjectName("valueLabel");
        valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        valueLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        valueLabel->setWordWrap(true); // 全局开启换行

        // 来源栏合并列，充分利用宽度
        if (title == QStringLiteral("来源")) {
            layout->addWidget(tLabel, row, 0, 1, 1);
            layout->addWidget(valueLabel, row, 1, 1, 2); // 占2列
            // 给来源标签单独加右内边距
        }
        else {
            layout->addWidget(tLabel, row, 0);
            layout->addWidget(valueLabel, row, 1);
            if (!unit.isEmpty()) {
                auto* uLabel = new QLabel(unit, this);
                uLabel->setObjectName("unitLabel");
                uLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
                layout->addWidget(uLabel, row, 2);
            }
        }
    };

    addInfoRow(QStringLiteral("尺寸"), labelDimension, "mm", 0);
    addInfoRow(QStringLiteral("图元数量"), labelEntities, "个", 1);
    addInfoRow(QStringLiteral("轮廓数量"), labelContours, "个", 2);
    addInfoRow(QStringLiteral("路径总长"), labelTotalPath, "mm", 3);
    addInfoRow(QStringLiteral("空走长度"), labelIdlePath, "mm", 4);
    addInfoRow(QStringLiteral("空走占比"), labelIdleRatio, "%", 5);
    addInfoRow(QStringLiteral("来源"),labelSource,"", 6);
   
    QVBoxLayout* layout2 = new QVBoxLayout(this);
    layout2->addWidget(groupBox);
    if (g_settings->value("Settings/DebugAssist").toInt())
    {
        btnOpenComm = new QPushButton("断开通信连接");
        connect(btnOpenComm, &QPushButton::clicked, this, &SketchInfoPanel::CommDisconnect);
        layout2->addWidget(btnOpenComm);
    }
    //layout2->addWidget(statusInfo);
    TaskFlowGuide* guide = new TaskFlowGuide(this);
    layout2->addWidget(guide,Qt::AlignBottom);
    layout2->setContentsMargins(0, 10, 0, 10);

    FontAttributePanel* fontPanel = FontAttributePanel::GetInstance();
    fontPanel->setFixedHeight(screen_resolution_y * 0.36);
    layout2->addWidget(fontPanel);

    this->setLayout(layout2);
}

void SketchInfoPanel::applyStyle()
{
    this->setStyleSheet(R"(
    InfoPanel {
        background-color: #FFFFFF;
        border: 1px solid #D1D1D1;
        border-radius: 2px;
    }

    QLabel#titleLabel {
        color: #444444; 
        font-family: "Segoe UI", "Microsoft YaHei";
        font-size: 12px;
        padding-left: 5px;
    }

    QLabel#valueLabel {
        color: #005FB8; 
        font-family: "Consolas", "Segoe UI Semibold";
        font-weight: 600;
        font-size: 14px;
        padding-right: 5px;
    }

    QLabel#unitLabel {
        color: #888888;
        font-size: 11px;
    }
)");
}

void SketchInfoPanel::CommDisconnect()
{
    ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::RUNNING, false);
    btnOpenComm->disconnect();
    btnOpenComm->setText("连接通信");
    connect(btnOpenComm,&QPushButton::clicked,this,&SketchInfoPanel::CommConnect);
}
void SketchInfoPanel::CommConnect()
{
    ScadaScheduler::GetInstance()->SetStatus(DISPACTH_FLAG_BIT::RUNNING, true);
    btnOpenComm->disconnect();
    btnOpenComm->setText("断开通信连接");
    connect(btnOpenComm, &QPushButton::clicked, this, &SketchInfoPanel::CommDisconnect);
}