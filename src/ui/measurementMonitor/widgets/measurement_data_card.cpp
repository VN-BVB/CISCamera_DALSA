#include "measurement_data_card.h"

#include <QBoxLayout>
#include "../qss_loader.h"

MeasurementDataCard::MeasurementDataCard(const QString &title, const QString &unit, QWidget *parent)
    : QFrame(parent)
    , m_title(title)
    , m_unit(unit)
    , m_value(0.0)
    , m_isValid(false)
    , m_titleLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_unitLabel(nullptr)
    , m_statusIcon(nullptr)
{
    setupUi();
}

void MeasurementDataCard::setupUi() {
    setFixedHeight(80);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(10);

    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);

    m_titleLabel = new QLabel(m_title, leftWidget);
    m_titleLabel->setObjectName("titleLabel");
    leftLayout->addWidget(m_titleLabel);

    QWidget* valueWidget = new QWidget(leftWidget);
    QHBoxLayout* valueLayout = new QHBoxLayout(valueWidget);
    valueLayout->setContentsMargins(0, 0, 0, 0);
    valueLayout->setSpacing(5);

    m_valueLabel = new QLabel("--", valueWidget);
    m_valueLabel->setObjectName("valueLabel");
    m_valueLabel->setProperty("state", "invalid");
    valueLayout->addWidget(m_valueLabel);

    m_unitLabel = new QLabel(m_unit, valueWidget);
    m_unitLabel->setObjectName("unitLabel");
    valueLayout->addWidget(m_unitLabel);
    valueLayout->addStretch();

    leftLayout->addWidget(valueWidget);
    mainLayout->addWidget(leftWidget, 1);

    m_statusIcon = new QLabel(this);
    m_statusIcon->setObjectName("statusIcon");
    m_statusIcon->setFixedSize(24, 24);
    m_statusIcon->setText(QString::fromUtf8("○"));
    m_statusIcon->setProperty("state", "invalid");
    m_statusIcon->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusIcon);
}

void MeasurementDataCard::setValue(double value, bool valid) {
    m_value = value;
    m_isValid = valid;

    if (valid) {
        m_valueLabel->setText(QString::number(value, 'f', 3));
        qss_loader::setState(m_valueLabel, "state", "valid");
        m_statusIcon->setText(QString::fromUtf8("✓"));
        qss_loader::setState(m_statusIcon, "state", "valid");
    } else {
        m_valueLabel->setText("--");
        qss_loader::setState(m_valueLabel, "state", "invalid");
        m_statusIcon->setText(QString::fromUtf8("○"));
        qss_loader::setState(m_statusIcon, "state", "invalid");
    }
}

void MeasurementDataCard::setTitle(const QString &title) {
    m_title = title;
    m_titleLabel->setText(title);
}
