#include "UI/SketchInformation.h"
#include <QGraphicsDropShadowEffect>
#include <qgroupbox.h>

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
    int padding = 10 + 10; // padding-left + padding-right

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

void SketchInfoPanel::updateStats(int entities, int contours, double totalLen, double idleLen,const QSize& size)
{
    static QList<QLabel*> label = { labelEntities,labelContours,labelTotalPath ,labelIdlePath,labelDimension };
    labelEntities->setText(QString::number(entities));
    labelContours->setText(QString::number(contours));
    labelTotalPath->setText(QString::asprintf("%.2f", totalLen));
    labelIdlePath->setText(QString::asprintf("%.2f", idleLen));
    labelDimension->setText(QString::asprintf("%d x %d", size.width(),size.height()));
    double ratio = (totalLen > 0) ? (idleLen / totalLen * 100.0) : 0.0;
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
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valueLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        layout->addWidget(tLabel, row, 0);
        layout->addWidget(valueLabel, row, 1);

        if (!unit.isEmpty()) {
            auto* uLabel = new QLabel(unit, this);
            uLabel->setObjectName("unitLabel");
            uLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            layout->addWidget(uLabel, row, 2);
        }
        layout->setColumnStretch(0, 0); // 标题列：不拉伸，按内容大小
        layout->setColumnStretch(1, 1); // 数值列：自动拉伸，填充剩余空间
        layout->setColumnStretch(2, 0); // 单位列：不拉伸
    };

    addInfoRow(QStringLiteral("尺寸"), labelDimension, "mm", 0);
    addInfoRow(QStringLiteral("图元数量"), labelEntities, "个", 1);
    addInfoRow(QStringLiteral("轮廓数量"), labelContours, "个", 2);
    addInfoRow(QStringLiteral("路径总度"), labelTotalPath, "mm", 3);
    addInfoRow(QStringLiteral("空走长度"), labelIdlePath, "mm", 4);
    addInfoRow(QStringLiteral("空走比例"), labelIdleRatio, "%", 5);

    QHBoxLayout* layout2 = new QHBoxLayout(this);
    layout2->addWidget(groupBox);
    layout2->setContentsMargins(0, 10, 0, 10);
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
