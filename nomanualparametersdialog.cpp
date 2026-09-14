#include "nomanualparametersdialog.h"
#include "ui_nomanualparametersdialog.h"

#include <QTimer>

noManualParametersDialog::noManualParametersDialog(int *nMSort, int *nMSortDir, int *nMListStart, int *nMListEnd, int nMListSize,
                                                   QWidget *parent)
    : QDialog(parent)
    , noManualSort(nMSort)
    , noManualSortDir(nMSortDir)
    , noManualListStart(nMListStart)
    , noManualListEnd(nMListEnd)
    , noManualListSize(nMListSize)
    , ui(new Ui::noManualParametersDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(qtTrId("nomanualparametersdialog.title.noManualParametersDialog"));

    ui->gBSortType->setTitle(qtTrId("nomanualparametersdialog.groupBox.gBSortType"));
    ui->gBSortDir->setTitle(qtTrId("nomanualparametersdialog.groupBox.gBSortDir"));

    ui->rbAdded->setText(qtTrId("nomanualparametersdialog.radioButton.rbAdded"));
    ui->rbAlpha->setText(qtTrId("nomanualparametersdialog.radioButton.rbAlpha"));
    ui->rbSize->setText(qtTrId("nomanualparametersdialog.radioButton.rbSize"));
    ui->rbLastMod->setText(qtTrId("nomanualparametersdialog.radioButton.rbLastMod"));
    ui->rbImageSize->setText(qtTrId("nomanualparametersdialog.radioButton.rbImageSize"));
    ui->rbAsc->setText(qtTrId("nomanualparametersdialog.radioButton.rbAsc"));
    ui->rbDesc->setText(qtTrId("nomanualparametersdialog.radioButton.rbDesc"));

    ui->lblListFrom->setText(qtTrId("nomanualparametersdialog.label.lblListFrom"));
    ui->lblListTo->setText(qtTrId("nomanualparametersdialog.label.lblListTo"));


    m_start           = *noManualListStart + 1;
    m_end             = *noManualListEnd   + 1;
    m_noManualSort    = *noManualSort;
    m_noManualSortDir = *noManualSortDir;

    m_start = std::min(std::max(0, m_start), noManualListSize);
    m_end   = std::min(std::max(m_start, m_end), std::min(m_start + maxRange - 1, noManualListSize));

    switch (*noManualSort)
    {
    case 1: ui->rbAlpha->setChecked(true); break;
    case 2: ui->rbSize->setChecked(true); break;
    case 3: ui->rbLastMod->setChecked(true); break;
    case 4: ui->rbImageSize->setChecked(true); break;
    default: ui->rbAdded->setChecked(true); break;
    }

    if (*noManualSortDir == 1)
        ui->rbDesc->setChecked(true);
    else
        ui->rbAsc->setChecked(true);


    ui->lblListFrom->setAlignment(Qt::AlignBottom);
    ui->lblListTo->setAlignment(Qt::AlignBottom);
    ui->lblMax->setAlignment(Qt::AlignBottom);
    ui->lblMin->setAlignment(Qt::AlignBottom);

    ui->lblMax->setText(QString("%1").arg(nMListSize));

    ui->containerMin->installEventFilter(this);
    ui->containerMax->installEventFilter(this);

    auto minLayout = new QVBoxLayout(ui->containerMin);
    minLayout->setContentsMargins(0, 20, 0, 0);
    minLayout->setSpacing(0);

    auto maxLayout = new QVBoxLayout(ui->containerMax);
    maxLayout->setContentsMargins(0, 20, 0, 0);
    maxLayout->setSpacing(0);

    slFrom = new QSlider(Qt::Horizontal);
    slFrom->setRange(1, nMListSize);
    slFrom->setValue(*nMListStart);
    // slFrom->setAttribute(Qt::WA_StyledBackground, true);
    minLayout->addWidget(slFrom);
    connect(slFrom, &QSlider::valueChanged,
            this, [=](int value) {
                on_slFrom_valueChanged(value);
            });

    slTo = new QSlider(Qt::Horizontal);
    slTo->setRange(1, nMListSize);
    slTo->setValue(*nMListStart);
    // slTo->setAttribute(Qt::WA_StyledBackground, true);
    maxLayout->addWidget(slTo);
    connect(slTo, &QSlider::valueChanged,
            this, [=](int value) {
                on_slTo_valueChanged(value);
            });

    lblMinCurrent = new QLabel(qtTrId("nomanualparametersdialog.label.min"), ui->containerMin);
    lblMinCurrent->setFixedSize(50, 20);
    lblMinCurrent->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblMinCurrent->setAlignment(Qt::AlignCenter);

    lblMinCurrent->raise();

    lblMaxCurrent = new QLabel(qtTrId("nomanualparametersdialog.label.max"), ui->containerMax);
    lblMaxCurrent->setFixedSize(50, 20);
    lblMaxCurrent->setAlignment(Qt::AlignCenter);
    lblMaxCurrent->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblMaxCurrent->raise();

    lblMinCurrent->setText(QString::number(m_start));
    lblMaxCurrent->setText(QString::number(m_end));

    QTimer::singleShot(0, this, [=]() {
        slFrom->setRange(1, nMListSize);
        slFrom->setValue(m_start);     // triggert reposition
        slTo->setRange(1, nMListSize);
        slTo->setValue(m_end);         // triggert reposition
        updateLabelMin();
        updateLabelMax();
    });

    this->resize(this->width(), this->height());
    initPhase = false;
}

// ----------------------------------------------------------------------------------------------------------
noManualParametersDialog::~noManualParametersDialog()
{
    delete ui;
}

// ----------------------------------------------------------------------------------------------------------
void noManualParametersDialog::accept()
{
    *noManualListStart = slFrom->value() - 1;
    *noManualListEnd   = slTo->value()   - 1;

    *noManualSort    = m_noManualSort;
    *noManualSortDir = m_noManualSortDir;

    QDialog::accept();
}

// ----------------------------------------------------------------------------------------------------------
void noManualParametersDialog::on_slFrom_valueChanged(int value)
{
    int currentTo = slTo->value();
    if (value > currentTo && !initPhase)
    {
        slFrom->setValue(currentTo);
    }
    else if (value <= (currentTo - maxRange))
    {
        slFrom->setValue(currentTo - maxRange + 1);
    }
/*
    else
    {
        slFrom->setValue(slFrom->value());
    }
*/
    updateLabelMin();
}

// ----------------------------------------------------------------------------------------------------------
void noManualParametersDialog::on_slTo_valueChanged(int value)
{
    int currentfrom = slFrom->value();
    if (value < currentfrom && !initPhase)
    {
        slTo->setValue(currentfrom);
    }
    else if (value >= (currentfrom + maxRange))
    {
        slTo->setValue(currentfrom + maxRange - 1);
    }
/*
    else
    {
        slTo->setValue(slTo->value());
    }
*/
    updateLabelMax();
}

// ----------------------------------------------------------------------------------------------------------
void noManualParametersDialog::updateLabelMin()
{
    int value = slFrom->value();
    lblMinCurrent->setText(QString::number(value));

    int sliderWidth = slFrom->width();
    int min         = slFrom->minimum();
    int max         = slFrom->maximum();

    int lblLeft = lblMinCurrent->x();
    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    lblMinCurrent->move(x - lblMinCurrent->width()/2, 0);

    lblLeft = lblMinCurrent->x();
}

// ----------------------------------------------------------------------------------------------------------
void noManualParametersDialog::updateLabelMax()
{
    int value = slTo->value();
    lblMaxCurrent->setText(QString::number(value));

    int sliderWidth = slTo->width();
    int min         = slTo->minimum();
    int max         = slTo->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    lblMaxCurrent->move(x - lblMaxCurrent->width()/2, 0);

}

// --------------------------------------------------------------------------------------------------------------
bool noManualParametersDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == slFrom && event->type() == QEvent::Resize)
    {
        updateLabelMin();
        return false;
    }
    else if (obj == slTo && event->type() == QEvent::Resize)
    {
        updateLabelMax();
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

// --------------------------------------------------------------------------------------------------------------
void noManualParametersDialog::on_rbAdded_clicked()
{
    m_noManualSort = 0;
}


void noManualParametersDialog::on_rbAlpha_clicked()
{
    m_noManualSort = 1;
}


void noManualParametersDialog::on_rbSize_clicked()
{
    m_noManualSort = 2;
}


void noManualParametersDialog::on_rbLastMod_clicked()
{
    m_noManualSort = 3;
}


void noManualParametersDialog::on_rbImageSize_clicked()
{
    m_noManualSort = 4;
}


void noManualParametersDialog::on_rbAsc_clicked()
{
    m_noManualSortDir = 0;
}


void noManualParametersDialog::on_rbDesc_clicked()
{
    m_noManualSortDir = 1;
}

// --------------------------------------------------------------------------------------------------------------
