#include "mainwindow.h"
#include "nomanualparametersdialog.h"
// #include "./ui_mainwindow.h"

// #include <algorithm>
#include <QApplication>
// #include <QCryptographicHash>
// #include <QDebug>
// #include <QDir>
// #include <QDirIterator>
// #include <QElapsedTimer>
// #include <QEvent>
// #include <QFile>
#include <QFileDialog>
#include <QFileInfo>
// #include <QFileInfoList>
#include <QImageReader>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
// #include <QSettings>       // <- mainwindow.h
#include <QStandardPaths>
#include <QStyleFactory>
#include <QtConcurrent>
// #include <QThread>
// #include <QThreadPool>
#include <QTimer>
// #include <QVBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QSqlRecord>



// Vom header hierhin:
#include <QStandardItemModel>
#include <QCloseEvent>

#include <QSqlQuery>
#include <QSqlError>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QCheckBox>
#include <QTreeView>

#include <QColorSpace>
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QString v, QWidget *parent)
    : QMainWindow(parent),
    progressTimer(nullptr)
{

    qInfo() << "Qt version:" << qVersion();
    qInfo() << "Image formats:" << QImageReader::supportedImageFormats();

    version = v;

    // set Language
    QSettings settings("DuplicateFinder", "DuplicateFinder");
    QString currentLanguage = settings.value("General/Language", QLocale::system().name()).toString();
    setLanguage(currentLanguage);

    // Window Size + Load first part of settings
    resize(1000, 700);
    setMinimumSize(1000, 400);
    loadSettingsPart1();

    auto *central = new QWidget;
    setCentralWidget(central);

    folderTreeView = new QTreeView;
    folderModel    = new QStandardItemModel(this);

    folderModel->setHorizontalHeaderLabels({qtTrId("mainwindow.table.colFolder"), qtTrId("mainwindow.table.colImages"),
                                            qtTrId("mainwindow.table.colStatus"), qtTrId("mainwindow.table.colExclDir")});
    folderModel->setHeaderData(1, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    folderModel->setHeaderData(2, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    folderModel->setHeaderData(3, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    folderTreeView->setModel(folderModel);
    // folderTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // folderTreeView->setSelectionBehavior(QAbstractItemView::SelectColumns);
    folderTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    folderTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    folderTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    folderTreeView->header()->setStretchLastSection(false);
    folderTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    folderTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    folderTreeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    folderTreeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    folderTreeView->header()->setMinimumSectionSize(120);

    setCombo     = new QComboBox;
    btnNewSet    = new QPushButton(qtTrId("mainwindow.button.newSet"));   btnNewSet->setStyleSheet("QPushButton { padding: 3px 10px; }" );
    btnCloneSet  = new QPushButton(qtTrId("mainwindow.button.duplSet"));  btnCloneSet->setStyleSheet("QPushButton { padding: 3px 10px; }" );
    btnDeleteSet = new QPushButton(qtTrId("mainwindow.button.delSet"));   btnDeleteSet->setStyleSheet("QPushButton { padding: 3px 10px; }" );

    btnAddFolder    = new QPushButton(qtTrId("mainwindow.button.addFolder"));
    btnRemoveFolder = new QPushButton(qtTrId("mainwindow.button.remFolder"));

    progressLabel = new QLabel(qtTrId("mainwindow.label.progressTask.Idle"));
    progressLabel->setFixedWidth(170);
    progressLabel->setAlignment(Qt::AlignCenter);
    progressBar   = new QProgressBar(this);
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setRange(0, 1);
    progressBar->setValue(0);     // Startwert
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: gray; }");
    volumeLabel = new QLabel(qtTrId("mainwindow.label.progressNoMax"));
    volumeLabel->setFixedWidth(125);
    volumeLabel->setAlignment(Qt::AlignCenter);

    topLayout = new QHBoxLayout;

    // LINKS (Sets) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *leftLayout = new QHBoxLayout;
    leftLayout->addWidget(setCombo);
    leftLayout->addWidget(btnNewSet);
    leftLayout->addWidget(btnCloneSet);
    leftLayout->addWidget(btnDeleteSet);

    // MITTE Rechts (Button) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    btnFindDuplicates = new QPushButton(qtTrId("mainwindow.button.findDupl"));

    btnFindDuplicates->setStyleSheet("QPushButton { padding: 3px 10px; }" );

    // Rechts daneben: version label
    auto lblVersion = new QLabel(version);

    // Farben für Sliders - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    QColor c = palette().color(QPalette::Highlight);
    hexApplied = c.name();
    QColor hsv  = c.toHsv();
    hsv.setHsv(hsv.hue(), std::max(0, hsv.saturation() - 120), std::min(255, hsv.value() + 0));
    hexChanged = hsv.name();

    // Unten links (Slider für ORB Limit)  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    SliderLbl1 = new QLabel(qtTrId("mainwindow.slider.ORB.Title"));  // <center>(3) ORB Score Limit<br>higher Result =&gt; Duplicate</center>
    auto *minValue1  = new QLabel("0.05");
    auto *maxValue1  = new QLabel("1.0");
    sliderScoreLimit = new QSlider(Qt::Horizontal);
    sliderScoreLimit->setRange(5, 100);
    sliderScoreLimit->setValue((int)(scoreLimit * 100));
    sliderScoreLimit->setAttribute(Qt::WA_StyledBackground, true);

    QString currentStyle = sliderScoreLimit->styleSheet();
    sliderScoreLimit->setStyle(QStyleFactory::create("fusion"));
    currentStyle = sliderScoreLimit->styleSheet();

    sliderContainer1 = new QWidget;
    sliderContainer1->setFixedHeight(60);
    sliderContainer1->installEventFilter(this);

    auto *containerLayout1 = new QVBoxLayout(sliderContainer1);
    containerLayout1->setContentsMargins(0, 20, 0, 0);
    containerLayout1->setSpacing(0);

    // Slider normal ins Layout
    containerLayout1->addWidget(sliderScoreLimit);

    // Label als Overlay (frei beweglich)
    value1Label = new QLabel("0.5", sliderContainer1);
    value1Label->setFixedWidth(50);
    value1Label->setAlignment(Qt::AlignCenter);
    value1Label->raise();

    connect(sliderScoreLimit, &QSlider::valueChanged,
            this, [=](int value) {

            updateLabel1Pos();

            if (!forcedApply) {
                btnApply->setEnabled(true);

                QPalette p = sliderScoreLimit->palette();
                p.setColor(QPalette::Highlight, QColor(hexChanged));
                sliderScoreLimit->setPalette(p);
            }
            });

    // Unten rechts 1 (Slider für HashDistance Lower Limit) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    SliderLbl2 = new QLabel(qtTrId("mainwindow.slider.pHashLower.Title"));  // <center>(1a) pHash Lower Limit<br>lower Result =&gt; always Duplicate</center>
    auto *minValue2  = new QLabel("1");
    auto *maxValue2  = new QLabel("50");
    sliderDistLower   = new QSlider(Qt::Horizontal);
    sliderDistLower->setRange(1, 50);
    sliderDistLower->setValue(distanceLowerLimit);
    sliderDistLower->setAttribute(Qt::WA_StyledBackground, true);

    sliderDistLower->setStyle(QStyleFactory::create("fusion"));
    currentStyle = sliderDistLower->styleSheet();

    sliderContainer2 = new QWidget;
    sliderContainer2->setFixedHeight(60);
    sliderContainer2->installEventFilter(this);

    auto *containerLayout2 = new QVBoxLayout(sliderContainer2);
    containerLayout2->setContentsMargins(0, 20, 0, 0);
    containerLayout2->setSpacing(0);

    // Slider normal ins Layout
    containerLayout2->addWidget(sliderDistLower);

    // Label als Overlay (frei beweglich)
    value2Label = new QLabel("10", sliderContainer2);
    value2Label->setFixedWidth(50);
    value2Label->setAlignment(Qt::AlignCenter);
    value2Label->raise();

    connect(sliderDistLower, &QSlider::valueChanged,
            this, [=](int value) {

        if (!forcedApply) {
            if (value > sliderDistUpper->value()) {
                        sliderDistLower->setValue(sliderDistUpper->value());
                } else {
                    btnApply->setEnabled(true);

                    QPalette p = sliderDistLower->palette();
                    p.setColor(QPalette::Highlight, QColor(hexChanged));
                    sliderDistLower->setPalette(p);
                }
        }

        updateLabel2Pos();
    });

    // Unten rechts 2 (Slider für HashDistance Upper Limit) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    SliderLbl3 = new QLabel(qtTrId("mainwindow.slider.pHashHigher.Title"));  // <center>(1b) pHash Upper Limit<br>higher Result =&gt; always NO Duplicate</center>
    auto *minValue3  = new QLabel("1");
    auto *maxValue3  = new QLabel("50");
    sliderDistUpper = new QSlider(Qt::Horizontal);
    sliderDistUpper->setRange(1, 50);
    sliderDistUpper->setValue(distanceUpperLimit);
    sliderDistUpper->setAttribute(Qt::WA_StyledBackground, true);

    QString currentStyle3 = sliderDistUpper->styleSheet();
    sliderDistUpper->setStyle(QStyleFactory::create("fusion"));
    currentStyle = sliderDistUpper->styleSheet();

    sliderContainer3 = new QWidget;
    sliderContainer3->setFixedHeight(60);
    sliderContainer3->installEventFilter(this);

    auto *containerLayout3 = new QVBoxLayout(sliderContainer3);
    containerLayout3->setContentsMargins(0, 20, 0, 0);
    containerLayout3->setSpacing(0);

    // Slider normal ins Layout
    containerLayout3->addWidget(sliderDistUpper);

    // Label als Overlay (frei beweglich)
    value3Label = new QLabel("15", sliderContainer3);
    value3Label->setFixedWidth(50);
    value3Label->setAlignment(Qt::AlignCenter);
    value3Label->raise();

    connect(sliderDistUpper, &QSlider::valueChanged,
            this, [=](int value) {

        if (!forcedApply) {
            if (value < sliderDistLower->value()) {
                sliderDistUpper->setValue(sliderDistLower->value());
            } else {
                btnApply->setEnabled(true);

                QPalette p = sliderDistUpper->palette();
                p.setColor(QPalette::Highlight, QColor(hexChanged));
                sliderDistUpper->setPalette(p);
            }
        }

        updateLabel3Pos();
    });


    // Unten rechts 1 (Slider für Canny Lower Limit) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    SliderLbl4 = new QLabel(qtTrId("mainwindow.slider.cannyLower.Title"));  // <center>(2a) Canny Lower Limit<br>lower Result =&gt; always Duplicate</center>
    auto *minValue4  = new QLabel("0.01");
    auto *maxValue4  = new QLabel("0.2");
    sliderCannyLower   = new QSlider(Qt::Horizontal);
    sliderCannyLower->setRange(1, 20);
    sliderCannyLower->setValue((int)(cannyDiffLowerLimit * 100));
    sliderCannyLower->setAttribute(Qt::WA_StyledBackground, true);

    sliderCannyLower->setStyle(QStyleFactory::create("fusion"));
    currentStyle = sliderCannyLower->styleSheet();

    sliderContainer4 = new QWidget;
    sliderContainer4->setFixedHeight(60);
    sliderContainer4->installEventFilter(this);

    auto *containerLayout4 = new QVBoxLayout(sliderContainer4);
    containerLayout4->setContentsMargins(0, 20, 0, 0);
    containerLayout4->setSpacing(0);

    // Slider normal ins Layout
    containerLayout4->addWidget(sliderCannyLower);

    // Label als Overlay (frei beweglich)
    value4Label = new QLabel("0.03", sliderContainer4);
    value4Label->setFixedWidth(50);
    value4Label->setAlignment(Qt::AlignCenter);
    value4Label->raise();

    connect(sliderCannyLower, &QSlider::valueChanged,
            this, [=](int value) {

                if (!forcedApply) {
                    if (value > sliderCannyUpper->value()) {
                        sliderCannyLower->setValue(sliderCannyUpper->value());
                    } else {
                        btnApply->setEnabled(true);

                        QPalette p = sliderCannyLower->palette();
                        p.setColor(QPalette::Highlight, QColor(hexChanged));
                        sliderCannyLower->setPalette(p);
                    }
                }

                updateLabel4Pos();
            });

    // Unten rechts 2 (Slider für Canny Upper Limit) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    SliderLbl5 = new QLabel(qtTrId("mainwindow.slider.cannyHigher.Title"));  // <center>(2b) Canny Upper Limit<br>higher Result =&gt; always NO Duplicate</center>
    auto *minValue5  = new QLabel("0.01");
    auto *maxValue5  = new QLabel("0.2");
    sliderCannyUpper = new QSlider(Qt::Horizontal);
    sliderCannyUpper->setRange(1, 20);
    sliderCannyUpper->setValue((int)(cannyDiffUpperLimit * 100));
    sliderCannyUpper->setAttribute(Qt::WA_StyledBackground, true);

    QString currentStyle5 = sliderCannyUpper->styleSheet();
    sliderCannyUpper->setStyle(QStyleFactory::create("fusion"));
    currentStyle = sliderCannyUpper->styleSheet();

    sliderContainer5 = new QWidget;
    sliderContainer5->setFixedHeight(60);
    sliderContainer5->installEventFilter(this);

    auto *containerLayout5 = new QVBoxLayout(sliderContainer5);
    containerLayout5->setContentsMargins(0, 20, 0, 0);
    containerLayout5->setSpacing(0);

    // Slider normal ins Layout
    containerLayout5->addWidget(sliderCannyUpper);

    // Label als Overlay (frei beweglich)
    value5Label = new QLabel("0.1", sliderContainer5);
    value5Label->setFixedWidth(50);
    value5Label->setAlignment(Qt::AlignCenter);
    value5Label->raise();

    connect(sliderCannyUpper, &QSlider::valueChanged,
            this, [=](int value) {

                if (!forcedApply) {
                    if (value < sliderCannyLower->value()) {
                        sliderCannyUpper->setValue(sliderCannyLower->value());
                    } else {
                        btnApply->setEnabled(true);

                        QPalette p = sliderCannyUpper->palette();
                        p.setColor(QPalette::Highlight, QColor(hexChanged));
                        sliderCannyUpper->setPalette(p);
                    }
                }

                updateLabel5Pos();
            });

    QTimer::singleShot(0, this, [=]() {
        sliderScoreLimit->setValue(sliderScoreLimit->value());  // triggert reposition
        sliderDistLower->setValue(sliderDistLower->value());    // triggert reposition
        sliderDistUpper->setValue(sliderDistUpper->value());    // triggert reposition
        sliderCannyLower->setValue(sliderCannyLower->value());  // triggert reposition
        sliderCannyUpper->setValue(sliderCannyUpper->value());  // triggert reposition
    });


    // Unten rechts (Exclude Same Dir CheckBox) - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // cbExcludeSameDir = new QCheckBox("Exclude Same Dir");
    lblSameDir = new QLabel(qtTrId("mainwindow.combobox.lblMngSameDir"));
    lblSameDir->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    cbHandleSameDir = new QComboBox();
    cbHandleSameDir->addItem(qtTrId("mainwindow.combobox.valOff"));
    cbHandleSameDir->addItem(qtTrId("mainwindow.combobox.valExcl"));
    cbHandleSameDir->addItem(qtTrId("mainwindow.combobox.valOnly"));
    cbHandleSameDir->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QFontMetrics fm(cbHandleSameDir->font());
    int cbWidth = 0;
    for (int i = 0; i < cbHandleSameDir->count(); ++i)
        cbWidth = qMax(cbWidth, fm.horizontalAdvance(cbHandleSameDir->itemText(i)));
    cbHandleSameDir->setMinimumWidth(cbWidth + 30);
    cbHandleSameDir->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(cbHandleSameDir, &QComboBox::currentIndexChanged,
            this, [=](int index) {
        commonSameDir = index;
    });

    // lblSameDir->setStyleSheet("background-color: red;");
    // cbHandleSameDir->setStyleSheet("background-color: blue;");

    auto *handleDirLayout = new QHBoxLayout;
    handleDirLayout->setSpacing(10);
    handleDirLayout->addStretch(1);
    handleDirLayout->addWidget(lblSameDir);
    handleDirLayout->addWidget(cbHandleSameDir);
    handleDirLayout->addStretch(1);
//    handleDirLayout->addSpacing(15);


    // Unten rechts (Apply Button)  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    btnApply = new QPushButton(qtTrId("mainwindow.button.apply"));
    btnApply->setEnabled(false);

    btnReset = new QPushButton(qtTrId("mainwindow.button.reset"));
    btnReset->setEnabled(true);

    btnManual         = new QPushButton(qtTrId("mainwindow.button.showManual"));
    btnManual->setStyleSheet("QPushButton { padding: 3px 10px; }" );

    btnCleanDB        = new QPushButton(qtTrId("mainwindow.button.cleanDB"));
    btnCleanDB->setStyleSheet("QPushButton { padding: 3px 10px; }" );

    btnLanguage = new QPushButton(qtTrId("mainwindow.button.language"));
    btnLanguage->setStyleSheet("QPushButton { padding: 3px 10px; }" );

    auto *lineH8 = new QFrame;
    lineH8->setFrameShape(QFrame::HLine); lineH8->setFrameShadow(QFrame::Sunken);

    auto *btnBox = new QVBoxLayout();
    btnBox->addWidget(btnApply);
    btnBox->addWidget(btnReset);
    btnBox->addWidget(lineH8);
    btnBox->addWidget(btnManual);
    btnBox->addWidget(btnCleanDB);
    btnBox->addWidget(btnLanguage);


    // Mitte - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    sliderContainer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sliderContainer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sliderContainer3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sliderContainer4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sliderContainer5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *slider1H = new QHBoxLayout();
    slider1H->addWidget(minValue1);
    slider1H->addWidget(sliderContainer1, 4);
    slider1H->addWidget(maxValue1);
    auto *slider2H = new QHBoxLayout();
    slider2H->addWidget(minValue2);
    slider2H->addWidget(sliderContainer2, 4);
    slider2H->addWidget(maxValue2);
    auto *slider3H = new QHBoxLayout();
    slider3H->addWidget(minValue3);
    slider3H->addWidget(sliderContainer3, 4);
    slider3H->addWidget(maxValue3);
    auto *slider4H = new QHBoxLayout();
    slider4H->addWidget(minValue4);
    slider4H->addWidget(sliderContainer4, 4);
    slider4H->addWidget(maxValue4);
    auto *slider5H = new QHBoxLayout();
    slider5H->addWidget(minValue5);
    slider5H->addWidget(sliderContainer5, 4);
    slider5H->addWidget(maxValue5);

    auto *lineH1 = new QFrame; lineH1->setFrameShape(QFrame::HLine); lineH1->setFrameShadow(QFrame::Sunken);
    auto *slider1V = new QVBoxLayout();  slider1V->setSpacing(0);
    slider1V->addWidget(SliderLbl1, 0, Qt::AlignCenter);
    slider1V->addSpacing(4);
    slider1V->addWidget(lineH1);
    slider1V->addLayout(slider1H);

    auto *lineH2 = new QFrame; lineH2->setFrameShape(QFrame::HLine); lineH2->setFrameShadow(QFrame::Sunken);
    auto *slider2V = new QVBoxLayout();  slider2V->setSpacing(0);
    slider2V->addWidget(SliderLbl2, 0, Qt::AlignCenter);
    slider2V->addSpacing(4);
    slider2V->addWidget(lineH2);
    slider2V->addLayout(slider2H);

    auto *lineH3 = new QFrame; lineH3->setFrameShape(QFrame::HLine); lineH3->setFrameShadow(QFrame::Sunken);
    auto *slider3V = new QVBoxLayout();  slider3V->setSpacing(0);
    slider3V->addWidget(SliderLbl3, 0, Qt::AlignCenter);
    slider3V->addSpacing(4);
    slider3V->addWidget(lineH3);
    slider3V->addLayout(slider3H);

    auto *lineH4 = new QFrame; lineH4->setFrameShape(QFrame::HLine); lineH4->setFrameShadow(QFrame::Sunken);
    auto *slider4V = new QVBoxLayout();  slider4V->setSpacing(0);
    slider4V->addWidget(SliderLbl4, 0, Qt::AlignCenter);
    slider4V->addSpacing(4);
    slider4V->addWidget(lineH4);
    slider4V->addLayout(slider4H);

    auto *lineH5 = new QFrame; lineH5->setFrameShape(QFrame::HLine); lineH5->setFrameShadow(QFrame::Sunken);
    auto *slider5V = new QVBoxLayout();  slider5V->setSpacing(0);
    slider5V->addWidget(SliderLbl5, 0, Qt::AlignCenter);
    slider5V->addSpacing(4);
    slider5V->addWidget(lineH5);
    slider5V->addLayout(slider5H);

    // RECHTS (Ordner)  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *rightLayout = new QHBoxLayout;
    rightLayout->addStretch();
    rightLayout->addWidget(btnAddFolder);
    rightLayout->addWidget(btnRemoveFolder);

    // Obere Zeile zusammensetzen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    topLayout->addLayout(leftLayout);
    topLayout->addStretch(4);
    topLayout->addWidget(btnFindDuplicates);
    topLayout->addStretch(4);
    topLayout->addWidget(lblVersion);
    topLayout->addStretch(1);
    topLayout->addLayout(rightLayout);



    // Grid Layout  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *lineV1 = new QFrame;
    lineV1->setFrameShape(QFrame::VLine);
    lineV1->setFrameShadow(QFrame::Sunken);
    auto *lineV2 = new QFrame;
    lineV2->setFrameShape(QFrame::VLine);
    lineV2->setFrameShadow(QFrame::Sunken);

    auto *lineH7 = new QFrame;
    lineH7->setFrameShape(QFrame::HLine);
    lineH7->setFrameShadow(QFrame::Sunken);

    auto *gridL = new QGridLayout();
    gridL->setContentsMargins(15, 0, 15, 0);
    gridL->setHorizontalSpacing(30);

    gridL->addLayout(slider2V, 0, 0);
    gridL->addLayout(slider3V, 0, 1);
    gridL->addWidget(lineV1, 0, 2);
    gridL->addLayout(slider1V, 0, 3);

    gridL->addWidget(lineH7, 1, 0, 1, 4);

    gridL->addLayout(slider4V, 2, 0);
    gridL->addLayout(slider5V, 2, 1);
    gridL->addWidget(lineV2, 2, 2);
    gridL->addLayout(handleDirLayout, 2, 3);  gridL->setColumnStretch(2, 3);

    /*
    // Footer 1a  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *footLayout1a = new QHBoxLayout;
    footLayout1a->addLayout(slider2V);
    footLayout1a->addSpacing(30);
    footLayout1a->addLayout(slider3V);
    footLayout1a->addSpacing(30);
    footLayout1a->addWidget(lineV2);
    footLayout1a->addSpacing(30);
    footLayout1a->addLayout(handleDirLayout);
    footLayout1a->addSpacing(30);

    // Footer 1b  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    auto *footLayout1b = new QHBoxLayout;
    footLayout1b->addLayout(slider4V);
    footLayout1b->addSpacing(30);
    footLayout1b->addLayout(slider5V);
    footLayout1b->addSpacing(30);
    footLayout1b->addWidget(lineV2);
    footLayout1b->addSpacing(30);
    footLayout1b->addWidget(lblSameDir);
    footLayout1b->addWidget(cbHandleSameDir);
    footLayout1b->addSpacing(30);

    // Footer 1ab - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *footLayout1ab = new QVBoxLayout;
    footLayout1ab->addLayout(footLayout1a);
    footLayout1ab->addWidget(lineH6);
    footLayout1ab->addLayout(footLayout1b);
*/

    // Footer 1 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *lineV3 = new QFrame;
    lineV3->setFrameShape(QFrame::VLine);
    lineV3->setFrameShadow(QFrame::Sunken);

    auto *footLayout1 = new QHBoxLayout;
    footLayout1->addLayout(gridL);
    // footLayout1->addLayout(footLayout1ab);
    footLayout1->addWidget(lineV3);
    footLayout1->addSpacing(15);
    // footLayout1->addWidget(btnApply);
    footLayout1->addLayout(btnBox);
    footLayout1->addSpacing(15);

    // Footer 3 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *footLayout2 = new QHBoxLayout;
    footLayout2->addWidget(progressLabel);
    footLayout2->addWidget(progressBar);
    footLayout2->addWidget(volumeLabel);



    // Alles zusammen setzen  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto *lineH6 = new QFrame;
    lineH6->setFrameShape(QFrame::HLine);
    lineH6->setFrameShadow(QFrame::Sunken);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(folderTreeView);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(footLayout1);
    mainLayout->addWidget(lineH6);
    mainLayout->addLayout(footLayout2);

    // Listen Handling + Größe
    setCombo->setEditable(true);
    setCombo->setInsertPolicy(QComboBox::NoInsert);
    setCombo->setMinimumWidth(200);
    setCombo->setMaximumWidth(400);

    connect(setCombo->lineEdit(), &QLineEdit::editingFinished, this, [this]() {

        QString newName = setCombo->currentText().trimmed();

        if (newName.isEmpty())
        {   reloadSetNames();
            return;
        }

        // aktuelles Set ausnehmen!
        for (int i = 0; i < m_sets.size(); ++i)
        {   if (i != m_currentSetIndex && m_sets[i].name == newName)
            {   QMessageBox::warning(this, qtTrId("mainwindow.output.error"), qtTrId("mainwindow.output.error.setExists").arg(newName));
                reloadSetNames();
                return;
            }
        }
        renameCurrentSet(newName);
    });

    connect(setCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::onSetChanged);

    connect(btnNewSet, &QPushButton::clicked,
            this, &MainWindow::createNewSet);

    connect(btnCloneSet, &QPushButton::clicked,
            this, &MainWindow::cloneSet);

    connect(btnDeleteSet, &QPushButton::clicked,
            this, &MainWindow::deleteSet);

    connect(btnFindDuplicates, &QPushButton::clicked,
            this, &MainWindow::startStopScanAndFind);

    connect(btnCleanDB, &QPushButton::clicked,
            this, &MainWindow::cleanDB);

    connect(btnLanguage, &QPushButton::clicked,
            this, &MainWindow::showLanguageDialog);

    connect(btnManual, &QPushButton::clicked,
            this, &MainWindow::FetchManualDuplicateList);

    connect(btnAddFolder, &QPushButton::clicked,
            this, &MainWindow::addFolder);

    connect(btnRemoveFolder, &QPushButton::clicked,
            this, &MainWindow::removeFolder);

    connect(btnApply, &QPushButton::clicked,
            this, &MainWindow::onApply);

    connect(btnReset, &QPushButton::clicked,
            this, &MainWindow::onReset);

    connect(folderTreeView, &QTreeView::clicked,
            this, &MainWindow::handleItemClicked);

    connect(this, &MainWindow::updateProgress, this, &MainWindow::onUpdateProgress, Qt::QueuedConnection);
    connect(this, &MainWindow::updateProgressForThreads, this, &MainWindow::onUpdateProgress, Qt::QueuedConnection);


    loadSettingsPart2();
    if (!initializePictureDB()) {
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        return;
    };

}

// -------------------------------------------------------------------------------------------------------------
/*
MainWindow::~MainWindow()
{
    //    delete ui;
}
*/

// -------------------------------------------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

    if (m_duplicateView)
    {
        m_duplicateView->close();
        m_duplicateView = nullptr;
    }

    QMainWindow::closeEvent(event);
}

// --------------------------------------------------------------------------------------------------------------
void MainWindow::prepareProgress(QString text, qint64 nbr, QString colorText, QString colorBar)
{
    progressLabel->setText(text);
    progressLabel->setStyleSheet(QString("color: %1;").arg(colorText));
    progressBar->setRange(0, std::max(qint64(1), nbr));
    progressBar->setValue(0);
    progressBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(colorBar));
    if (nbr >= 0)
        volumeLabel->setText(QLocale().toString(nbr));
    else
        volumeLabel->setText(qtTrId("mainwindow.label.progressNoMax"));
    QCoreApplication::processEvents();
}

// --------------------------------------------------------------------------------------------------------------
void MainWindow::onReset()
{
    sliderScoreLimit->setValue((int)(0.5 * 100));
    sliderDistLower->setValue(14);
    sliderDistUpper->setValue(20);
    sliderCannyLower->setValue((int)(0.03 * 100));
    sliderCannyUpper->setValue((int)(0.10 * 100));

    btnApply->setEnabled(true);
}
// --------------------------------------------------------------------------------------------------------------
void MainWindow::onApply()
{   bool changes = false;

    btnApply->setEnabled(false);

    QPalette p = sliderScoreLimit->palette();
    p.setColor(QPalette::Highlight, QColor(hexApplied));
    sliderScoreLimit->setPalette(p);
    sliderDistLower->setPalette(p);
    sliderDistUpper->setPalette(p);

    changes  = scoreLimit          != sliderScoreLimit->value() / 100.0f;
    changes |= distanceLowerLimit  != sliderDistLower->value();
    changes |= distanceUpperLimit  != sliderDistUpper->value();
    changes |= cannyDiffLowerLimit != sliderCannyLower->value() / 100.0f;
    changes |= cannyDiffUpperLimit != sliderCannyUpper->value() / 100.0f;

    if (changes) {
        scoreLimit          = sliderScoreLimit->value() / 100.0f;
        distanceLowerLimit  = sliderDistLower->value();
        distanceUpperLimit  = sliderDistUpper->value();
        cannyDiffLowerLimit = sliderCannyLower->value() / 100.0f;
        cannyDiffUpperLimit = sliderCannyUpper->value() / 100.0f;

        QSettings settings("DuplicateFinder", "DuplicateFinder");
        settings.setValue("Program/scoreLimit"         , (int)(scoreLimit * 100) );

        settings.setValue("Program/distanceLowerLimit" , distanceLowerLimit );
        settings.setValue("Program/distanceUpperLimit" , distanceUpperLimit );

        settings.setValue("Program/cannyDiffLowerLimit", cannyDiffLowerLimit);
        settings.setValue("Program/cannyDiffUpperLimit", cannyDiffUpperLimit);

        if (m_duplicateView)
            CalculateDuplicateList(false);  // run on existing data, which might have deleted pairs
    }
}

// --------------------------------------------------------------------------------------------------------------
void MainWindow::updateLabel1Pos()
{
    int value = sliderScoreLimit->value();

    value1Label->setText(QString::number(value / 100.0, 'f', 2));

    int sliderWidth = sliderScoreLimit->width();
    int min = sliderScoreLimit->minimum();
    int max = sliderScoreLimit->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    value1Label->move(x - value1Label->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
void MainWindow::updateLabel2Pos()
{
    int value = sliderDistLower->value();

    value2Label->setText(QString::number(value));

    int sliderWidth = sliderDistLower->width();
    int min = sliderDistLower->minimum();
    int max = sliderDistLower->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    value2Label->move(x - value2Label->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
void MainWindow::updateLabel3Pos()
{
    int value = sliderDistUpper->value();

    value3Label->setText(QString::number(value));

    int sliderWidth = sliderDistUpper->width();
    int min = sliderDistUpper->minimum();
    int max = sliderDistUpper->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    value3Label->move(x - value3Label->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
void MainWindow::updateLabel4Pos()
{
    int value = sliderCannyLower->value();

    value4Label->setText(QString::number(value / 100.0, 'f', 2));

    int sliderWidth = sliderCannyLower->width();
    int min = sliderCannyLower->minimum();
    int max = sliderCannyLower->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    value4Label->move(x - value4Label->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
void MainWindow::updateLabel5Pos()
{
    int value = sliderCannyUpper->value();

    value5Label->setText(QString::number(value / 100.0, 'f', 2));

    int sliderWidth = sliderCannyUpper->width();
    int min = sliderCannyUpper->minimum();
    int max = sliderCannyUpper->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    value5Label->move(x - value5Label->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sliderContainer1 && event->type() == QEvent::Resize) {
        updateLabel1Pos();
        return false;
    } else     if (obj == sliderContainer2 && event->type() == QEvent::Resize) {
        updateLabel2Pos();
        return false;
    } else     if (obj == sliderContainer3 && event->type() == QEvent::Resize) {
        updateLabel3Pos();
        return false;
    } else     if (obj == sliderContainer4 && event->type() == QEvent::Resize) {
        updateLabel4Pos();
        return false;
    } else     if (obj == sliderContainer5 && event->type() == QEvent::Resize) {
        updateLabel5Pos();
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::loadSettingsPart1()
{
    m_updating = true;

    QSettings settings("DuplicateFinder", "DuplicateFinder");

    if (settings.contains("MainWindow/geometry"))
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    lastDir = settings.value("Program/lastdir", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    currentSetting = settings.value("Program/lastSet", "").toString();

    scoreLimit          = settings.value("Program/scoreLimit"        , scoreLimit           * 100).toInt() / 100.0;

    distanceLowerLimit  = settings.value("Program/distanceLowerLimit", distanceLowerLimit         ).toInt();
    distanceUpperLimit  = settings.value("Program/distanceUpperLimit", distanceUpperLimit         ).toInt();

    cannyDiffLowerLimit = settings.value("Program/cannyDiffLowerLimit", cannyDiffLowerLimit).toFloat();
    cannyDiffUpperLimit = settings.value("Program/cannyDiffUpperLimit", cannyDiffUpperLimit).toFloat();
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::loadSettingsPart2()
{
    QSettings settings("DuplicateFinder", "DuplicateFinder");

    settings.beginGroup("sets");

    QStringList groups = settings.childGroups();

    std::sort(groups.begin(), groups.end(), [](const QString &a, const QString &b){
        return a.toInt() < b.toInt();
    });

    for (const QString &g : std::as_const(groups))
    {   settings.beginGroup(g);

        FolderSet set;
        set.name = settings.value("name").toString();

        settings.beginGroup("roots");

        QStringList rootGroups = settings.childGroups();

        for (const QString &rootGroup : std::as_const(rootGroups))
        {   set.roots.append(loadNode(settings, rootGroup));
        }

        settings.endGroup(); // roots

        m_sets.append(set);

        settings.endGroup(); // g
    }
    settings.endGroup();  // sets

    syncWithFilesystem(m_sets);

    // 👉 FALLBACK: Default Set erzeugen
    if (m_sets.isEmpty())
    {
        FolderSet defaultSet;
        defaultSet.name = qtTrId("mainwindow.default.setName");
        m_sets.append(defaultSet);
    }

    // m_sets sortieren
    std::sort(m_sets.begin(), m_sets.end(),
              [](const FolderSet &a, const FolderSet &b)
              {
                  return a.name.toLower() < b.name.toLower();
              });

    // Finde den aktuellen Index:
    m_currentSetIndex = 0;
    for (int i=0; i < m_sets.size(); i++) {
        if (m_sets[i].name == currentSetting) {
            m_currentSetIndex = i;
            break;
        }
    }

    // ComboBox füllen
    reloadSetNames();

    setCombo->blockSignals(true);
    setCombo->setCurrentIndex(0);
    setCombo->blockSignals(false);

    rebuildTree();

    QString tmp = qtTrId("mainwindow.table.colImages.withNumber").arg(QLocale().toString(countAllPictures()));
    folderModel->setHeaderData(1, Qt::Horizontal, tmp);

    m_updating = false;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::syncWithFilesystem(QVector<FolderSet> &sets)
{
    for (FolderSet &set : sets)
    {
        QVector<FolderNode> updated;

        for (FolderNode &root : set.roots)
        {
            if (!QDir(root.path).exists())
                continue; // ❌ löschen inkl. Subtree

            syncNode(root);

            updated.append(root);
        }

        set.roots = updated;
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::syncNode(FolderNode &node)
{
    QDir dir(node.path);
    if (!dir.exists())
        return;

    // Find the number of picture files
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff" << "*.webp" << "*.avif";   // "*.gif" <<
    const QList<QByteArray> supported = QImageReader::supportedImageFormats();
    filters.removeIf([&supported](const QString &filter) {
        const QString extension = filter.mid(2);   // "*.xxx" -> "xxx"
        return !supported.contains(extension.toUtf8());
    });

    node.nbrPictures = dir.entryList(
                              filters,  // {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tiff", "*.tif", "*.webp", "*.avif"},
                              QDir::Files | QDir::NoDotAndDotDot
                              ).size();

    QFileInfoList subdirs =
        dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 1. echte vorhandene Pfade sammeln
    QSet<QString> actual;
    for (const QFileInfo &info : std::as_const(subdirs))
        actual.insert(info.filePath());

    // 2. neue children hinzufügen
    QSet<QString> existing;
    for (const FolderNode &c : std::as_const(node.children))
        existing.insert(c.path);

    for (const QFileInfo &info : std::as_const(subdirs))
    {
        if (!existing.contains(info.filePath()))
        {
            FolderNode newNode;
            newNode.path = info.filePath();
            newNode.nbrPictures = 0;
            newNode.state = node.state; // Vererbung
            node.children.append(newNode);
        }
    }

    // 3. REKURSIV syncen + ungültige entfernen
    QVector<FolderNode> filtered;

    for (FolderNode &child : node.children)
    {
        if (!actual.contains(child.path))
        {
            continue; // ❌ entfernt verschwundene Ordner
        }

        syncNode(child);
        filtered.append(child);
    }

    node.children = filtered;

    std::sort(node.children.begin(), node.children.end(),
              [](const FolderNode &a, const FolderNode &b)
              {
                  return QString::compare(a.path, b.path, Qt::CaseInsensitive) < 0;
              });
}

// -------------------------------------------------------------------------------------------------------------

FolderNode MainWindow::loadNode(QSettings &settings, const QString &groupName)
{
    FolderNode node;

    settings.beginGroup(groupName);

    node.path              = settings.value("path").toString();
    node.state             = static_cast<FolderState>(settings.value("state").toInt());
    node.excludeSameFolder = static_cast<FolderState>(settings.value("excludeSameFolder", false).toBool());

    settings.beginGroup("children");

    QStringList childGroups = settings.childGroups();

    for (const QString &childGroup : std::as_const(childGroups))
    {
        node.children.append(loadNode(settings, childGroup));
    }

    settings.endGroup(); // children
    settings.endGroup(); // groupName

    return node;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::saveNode(QSettings &settings, const FolderNode &node, const QString &groupName)
{
    settings.beginGroup(groupName);

    settings.setValue("path", node.path);
    settings.setValue("state", static_cast<int>(node.state));
    settings.setValue("excludeSameFolder", static_cast<bool>(node.excludeSameFolder));

    settings.beginGroup("children");

    for (int i = 0; i < node.children.size(); ++i)
    {
        QString childGroup = QString("node_%1").arg(i);
        saveNode(settings, node.children[i], childGroup);
    }

    settings.endGroup(); // children
    settings.endGroup(); // groupName
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::saveSettings()
{
    if (m_updating)
        return;

    // saveCurrentSet();

    QSettings settings("DuplicateFinder", "DuplicateFinder");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup(); // MainWindow

    settings.beginGroup("Program");
    settings.setValue("lastdir", lastDir);
    settings.setValue("lastSet", currentSetting);
    settings.setValue("scoreLimit",  (int)(scoreLimit * 100) );
    settings.endGroup(); // Program

    settings.beginGroup("sets");
    settings.remove("");

    for (int i = 0; i < m_sets.size(); ++i)
    {
        settings.beginGroup(QString::number(i));
        settings.setValue("name", m_sets[i].name);

        settings.beginGroup("roots");
        for (int r = 0; r < m_sets[i].roots.size(); ++r)
        {
            QString groupName = QString("node_%1").arg(r);
            saveNode(settings, m_sets[i].roots[r], groupName);
        }
        settings.endGroup(); // roots

        settings.endGroup(); // set index
    }
    settings.endGroup(); // sets
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::addFolder()
{
    FolderState existingState;

    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size() || scanActive)
        return;


    if (lastDir.isEmpty() || !QDir(lastDir).exists()) lastDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    QString dir = QFileDialog::getExistingDirectory(this, qtTrId("mainwindow.dialog.selectFolder.title"), lastDir);
    if (dir.isEmpty())
        return;

    FolderSet &set = m_sets[m_currentSetIndex];

    // 🔥 DUPLIKAT CHECK
    if (folderExistsInModel(dir, nullptr))
    {
        QMessageBox::information(this, qtTrId("mainwindow.output.hint"),
                                 QString("mainwindow.output.hint.folderExists").arg(dir));
        return;
    }

    lastDir = dir;

    // 🔥 Datenmodell aufbauen
    FolderNode rootNode;
    rootNode.path = dir;
    rootNode.state = Included;


    rootsToBeRemoved.clear();

    buildDirectoryTree(rootNode, dir);   // rekursiv

    // Delete not needed root nodes - not needed as they are in the new root node
    std::sort(rootsToBeRemoved.begin(), rootsToBeRemoved.end(), std::greater<int>());

    for (auto &i : rootsToBeRemoved)
        m_sets[m_currentSetIndex].roots.removeAt(i);


    // - - - - - - - - - - - - - - - - - -
    set.roots.append(rootNode);

    syncNode(set.roots[set.roots.size()-1]);

    // 👉 optional sortieren (Datenmodell, NICHT UI!)
    std::sort(set.roots.begin(), set.roots.end(), [](const FolderNode &a, const FolderNode &b){
        return a.path < b.path;
    });


    QList<QStandardItem*> row = createRow(dir);
    folderModel->appendRow(row);

    rebuildTree();


    QString tmp = qtTrId("mainwindow.table.colImages.withNumber").arg(QLocale().toString(countAllPictures()));
    folderModel->setHeaderData(1, Qt::Horizontal, tmp);

    saveSettings();
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::folderExistsInModel(const QString &path, QStandardItem *parent)
{
    int rows = parent ? parent->rowCount() : folderModel->rowCount();

    for (int i = 0; i < rows; ++i)
    {
        QStandardItem *item = parent
                                  ? parent->child(i, 0)
                                  : folderModel->item(i, 0);

        if (!item)
            continue;

        if (item->text() == path) {
            return true;
        }

        if (item->hasChildren())
        {
            if (folderExistsInModel(path, item))
                return true;
        }
    }

    return false;
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::folderExistsInSet(const QString &path, QVector<FolderNode> *parent, FolderState &existingState, int &setNbr)
{
    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size())
        return false;

    QVector<FolderNode> set;

    if (parent == nullptr)
        set = m_sets[m_currentSetIndex].roots;
    else
        set = *parent;

    for (int i = 0; i < set.size(); ++i) {
        FolderNode s = set[i];
        if (s.path == path) {
            existingState = s.state;
            setNbr = i;
            return true;
        } else {
            bool found = folderExistsInSet(path, &s.children, existingState, setNbr);
            if (found) {
                setNbr = i;
                return found;
            }
        }
    }

    return false;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::removeFolder()
{
    if (scanActive) return;

    QModelIndex index = folderTreeView->currentIndex();
    if (!index.isValid())
        return;

    QModelIndex nameIndex = index.sibling(index.row(), 0);

    // 🔥 echten Top-Level Index im MODEL finden
    QModelIndex rootIndex = nameIndex;

    while (rootIndex.parent().isValid())
        rootIndex = rootIndex.parent();

    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size())
        return;

    int rootRow = rootIndex.row();

    FolderSet &set = m_sets[m_currentSetIndex];

    if (rootRow < 0 || rootRow >= set.roots.size())
        return;

    QString path = set.roots[rootRow].path;

    auto reply = QMessageBox::question(this, qtTrId("mainwindow.output.confirm"),
                                       qtTrId("mainwindow.output.confirm.delFolder").arg(path));

    if (reply == QMessageBox::Yes)
    {
        set.roots.removeAt(rootRow);

        rebuildTree(false);
        saveSettings();
    }

}

// -------------------------------------------------------------------------------------------------------------
FolderNode* MainWindow::getNodeFromIndex(const QModelIndex &index)
{
    QModelIndex nameIndex = index.sibling(index.row(), 0);

    QStandardItem *item = folderModel->itemFromIndex(nameIndex);
    if (!item)
        return nullptr;

    QVariant v = item->data(Qt::UserRole + 1);

    if (!v.isValid())
        return nullptr;

    int flatIndex = v.toInt();   // statt Pointer!

    return &m_sets[m_currentSetIndex].roots[flatIndex];
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::buildDirectoryTree(FolderNode &parent, const QString &path)
{
    QDir dir(path);
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &info : std::as_const(subdirs))
    {
        FolderState existingState = Included;
        int rootNbr = -1;
        FolderNode  child;
        child.path = info.filePath();

        // 👉 Status vom Parent übernehmen
        if (folderExistsInSet(child.path, nullptr, existingState, rootNbr)) {
            child.state = existingState;
            if (!rootsToBeRemoved.contains(rootNbr))
                rootsToBeRemoved.append(rootNbr);
        } else
            child.state = parent.state;

        buildDirectoryTree(child, info.filePath());

        parent.children.append(child);
    }
}

// -------------------------------------------------------------------------------------------------------------
QList<QStandardItem*> MainWindow::createRow(const QString &path)
{
    QStandardItem *nameItem     = new QStandardItem(path);
    QStandardItem *stateItem    = new QStandardItem;
    QStandardItem *eqFolderItem = new QStandardItem;
    nameItem->setEditable(false);
    stateItem->setEditable(false);
    eqFolderItem->setEditable(false);

//    setState(stateItem, Included);
    applyState(nameItem    , Included, true);
    applyState(stateItem   , Included, false);
    applyState(eqFolderItem, Included, true);

    return {nameItem, stateItem, eqFolderItem};
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::handleItemClicked(const QModelIndex &index)
{
    if (index.column() < 2 || scanActive)
        return;

    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size())
        return;

    FolderNode *node = findNodeByIndex(index);
    if (!node)
        return;

    bool recursive = QApplication::keyboardModifiers() & Qt::ShiftModifier;

    switch (index.column())
    {case 2:
    {
        FolderState state = node->state;
        state = static_cast<FolderState>((state + 1) % 3);

        if (recursive)
            setNodeStateRecursive(*node, state);
        else
            node->state = state;
        break;
    }

    case 3:
    {
        bool exclDir = node->excludeSameFolder;

        if (recursive)
            setNodeExclDirRecursive(*node, !exclDir);
        else
            node->excludeSameFolder = !exclDir;
        break;
    }
    }
    rebuildTree(false);
    saveSettings();

    QString tmp = qtTrId("mainwindow.table.colImages.withNumber").arg(QLocale().toString(countAllPictures()));
    folderModel->setHeaderData(1, Qt::Horizontal, tmp);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::applyState(QStandardItem *item, FolderState state, bool nameItem)
{
    if (!nameItem) item->setData(state, Qt::UserRole);

    switch (state)
    {
    case Excluded:
        if (!nameItem) item->setText(qtTrId("mainwindow.table.colStatus.valExcl"));
        item->setBackground(Qt::lightGray);
        break;

    case Included:
        if (!nameItem) item->setText(qtTrId("mainwindow.table.colStatus.valIncl"));
        item->setBackground(Qt::green);
        break;

    case Reference:
        if (!nameItem) item->setText(qtTrId("mainwindow.table.colStatus.valRef"));
        item->setBackground(Qt::cyan);
        break;
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setState(QStandardItem *stateItem, FolderState state)
{
    stateItem->setData(state, Qt::UserRole);

    switch (state)
    {
    case Excluded:
        stateItem->setText(qtTrId("mainwindow.table.colStatus.valExcl"));
        stateItem->setBackground(Qt::lightGray);
        break;

    case Included:
        stateItem->setText(qtTrId("mainwindow.table.colStatus.valIncl"));
        stateItem->setBackground(Qt::green);
        break;

    case Reference:
        stateItem->setText(qtTrId("mainwindow.table.colStatus.valRef"));
        stateItem->setBackground(Qt::cyan);
        break;
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setChildrenState(QStandardItem *parentNameItem, FolderState state)
{
    folderModel->blockSignals(true);

    for (int i = 0; i < parentNameItem->rowCount(); ++i)
    {
        QStandardItem *childState = parentNameItem->child(i, 1);
        QStandardItem *childName = parentNameItem->child(i, 0);

        setState(childState, state);
        setChildrenState(childName, state);
    }

    folderModel->blockSignals(false);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::createNewSet()
{
    if (scanActive) return;

    QString base = qtTrId("mainwindow.default.newSetName");
    QString name = base;
    int counter = 1;

    while (setCombo->findText(name) != -1)
        name = qtTrId("mainwindow.default.newSetName.Count").arg(QString::number(counter++));  //  base + " " + QString::number(counter++);

    FolderSet set;
    set.name = name;

    m_sets.append(set);

    setCombo->addItem(name);
    setCombo->setCurrentIndex(setCombo->count() - 1);

    currentSetting = name;
    saveSettings();

    btnAddFolder->setEnabled(true);
    btnRemoveFolder->setEnabled(true);
    btnCloneSet->setEnabled(true);
    btnDeleteSet->setEnabled(true);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::cloneSet()
{
    if (scanActive) return;

    int index = setCombo->currentIndex();
    if (index < 0 || index >= m_sets.size())
        return;

    saveSettings();

    const QString srcName = m_sets[index].name;

    // 1. Basisnamen bereinigen
    QString baseName = srcName;

    QString regExpr = qtTrId("mainwindow.default.cloneSet.RegularExpr");
    baseName.remove(QRegularExpression(regExpr));  // "\\sKopie(\\s\\d+)?$"));

    // 2. neue Namen generieren
    QString newName = qtTrId("mainwindow.default.cloneSet.firstCopy").arg(baseName); //  baseName + " Kopie";

    auto exists = [&](const QString &name)
    {
        for (const FolderSet &s : std::as_const(m_sets))
            if (s.name == name)
                return true;
        return false;
    };

    if (exists(newName))
    {
        int counter = 2;
        do {
            newName = qtTrId("mainwindow.default.cloneSet.copyCount").arg(baseName, QString::number(counter++)); //  baseName + " Kopie " + QString::number(counter++);
        } while (exists(newName));
    }

    // 3. kopieren
    FolderSet copy = m_sets[index];
    copy.name = newName;

    m_sets.append(copy);

    setCombo->addItem(newName);
    setCombo->setCurrentIndex(setCombo->count() - 1);

    currentSetting = newName;
    saveSettings();
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::deleteSet()
{
    if (scanActive) return;

    int index = setCombo->currentIndex();
    if (index < 0)
        return;

    auto reply = QMessageBox::question(this, qtTrId("mainwindow.output.delete"),
                                             qtTrId("mainwindow.output.delete.delSet").arg(m_sets[index].name));

    if (reply != QMessageBox::Yes)
        return;

    setCombo->blockSignals(true);
    m_sets.removeAt(index);
    setCombo->removeItem(index);

    if (m_sets.isEmpty())
        m_currentSetIndex = -1;
    else {
        m_currentSetIndex--;
        if (m_currentSetIndex >= 0)
            setCombo->setCurrentIndex(m_currentSetIndex);
    }
    setCombo->blockSignals(false);

    rebuildTree();

    if (m_currentSetIndex >= 0) {
        currentSetting = m_sets[m_currentSetIndex].name;
    } else {
        currentSetting = "";
        btnAddFolder->setEnabled(false);
        btnRemoveFolder->setEnabled(false);
        btnCloneSet->setEnabled(false);
        btnDeleteSet->setEnabled(false);
    }
    saveSettings();
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::setNameExists(const QString &name)
{
    for (int i = 0; i < setCombo->count(); ++i)
    {
        if (setCombo->itemText(i) == name)
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------------------
QList<QStandardItem*> MainWindow::createRowFromNode(const FolderNode &node, int flatIndex)
{
    QStandardItem *nameItem = new QStandardItem(node.path);
    nameItem->setEditable(false);

    QStandardItem *nbrFilesItem = new QStandardItem;
    nbrFilesItem->setEditable(false);

    QStandardItem *stateItem = new QStandardItem;
    stateItem->setEditable(false);

    QStandardItem *eqDirItem = new QStandardItem;
    eqDirItem->setEditable(false);

    // stateItem->setData(node.state, Qt::UserRole);
    // nameItem->setData(QVariant::fromValue((void*)&node), Qt::UserRole + 1);
    nameItem->setData(flatIndex, Qt::UserRole + 1);

    nbrFilesItem->setText(QLocale().toString(node.nbrPictures));

    switch (node.state)
    {
    case Excluded : stateItem->setText(qtTrId("mainwindow.table.colStatus.valExcl"));  break;
    case Included : stateItem->setText(qtTrId("mainwindow.table.colStatus.valIncl"));  break;
    case Reference: stateItem->setText(qtTrId("mainwindow.table.colStatus.valRef")); break;
    }

    if (node.excludeSameFolder)
        eqDirItem->setText(qtTrId("mainwindow.table.colExclDir.Excl"));
    else
        eqDirItem->setText("");

    applyState(nameItem     , node.state, true);
    applyState(nbrFilesItem , node.state, true);
    applyState(stateItem    , node.state, false);
    applyState(eqDirItem    , node.state, true);
    return {nameItem, nbrFilesItem, stateItem, eqDirItem};
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::rebuildTree(bool expand)
{
    // 🔥 Zustand merken
    QStringList expanded = getExpandedPaths();

    folderModel->clear();
    folderModel->setHorizontalHeaderLabels({qtTrId("mainwindow.table.colFolder"), qtTrId("mainwindow.table.colImages"),
                                            qtTrId("mainwindow.table.colStatus"), qtTrId("mainwindow.table.colExclDir")});
    folderModel->setHeaderData(1, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    folderModel->setHeaderData(2, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);
    folderModel->setHeaderData(3, Qt::Horizontal, Qt::AlignCenter, Qt::TextAlignmentRole);

    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size())
        return;

    const FolderSet &set = m_sets[m_currentSetIndex];

    std::function<void(const FolderNode&, QStandardItem*, int)> addNode;
    addNode = [&](const FolderNode &node, QStandardItem *parent, int flatIndex)
    {
        QList<QStandardItem*> row = createRowFromNode(node, flatIndex);
        QStandardItem *nameItem = row[0];

        if (parent) {
            parent->appendRow(row);
        } else {
            folderModel->appendRow(row);
        }

        QModelIndex idx = folderModel->indexFromItem(row[1]);
        folderModel->setData(idx, Qt::AlignCenter, Qt::TextAlignmentRole);
        idx = folderModel->indexFromItem(row[2]);
        folderModel->setData(idx, Qt::AlignCenter, Qt::TextAlignmentRole);
        idx = folderModel->indexFromItem(row[3]);
        folderModel->setData(idx, Qt::AlignCenter, Qt::TextAlignmentRole);

        // for (const FolderNode &child : node.children)
        for (int i = 0; i < node.children.size(); ++i)
        {
            // addNode(child, row[0]);
            addNode(node.children[i], nameItem, i);
        }
    };

    // for (const FolderNode &root : set.roots)
    for (int i = 0; i < set.roots.size(); ++i)
    {
        // addNode(root, nullptr);
        addNode(set.roots[i], nullptr, i);
    }

    // 🔥 Zustand wiederherstellen
    restoreExpandedPaths(expanded);

    folderTreeView->header()->resizeSections(QHeaderView::ResizeToContents);
    folderTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    if (expand) folderTreeView->expandAll();

    QString tmp = qtTrId("mainwindow.table.colImages.withNumber").arg(QLocale().toString(countAllPictures()));
    folderModel->setHeaderData(1, Qt::Horizontal, tmp);

}

// -------------------------------------------------------------------------------------------------------------
QStringList MainWindow::getExpandedPaths()
{
    QStringList expanded;

    std::function<void(const QModelIndex&)> collect;
    collect = [&](const QModelIndex &index)
    {
        if (!index.isValid())
            return;

        if (folderTreeView->isExpanded(index))
        {
            QString path = index.sibling(index.row(), 0).data().toString();
            expanded << path;
        }

        for (int i = 0; i < folderModel->rowCount(index); ++i)
        {
            collect(folderModel->index(i, 0, index));
        }
    };

    for (int i = 0; i < folderModel->rowCount(); ++i)
    {
        collect(folderModel->index(i, 0));
    }

    return expanded;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::restoreExpandedPaths(const QStringList &expanded)
{
    std::function<void(const QModelIndex&)> restore;

    restore = [&](const QModelIndex &index)
    {
        if (!index.isValid())
            return;

        QString path = index.sibling(index.row(), 0).data().toString();

        if (expanded.contains(path))
        {
            folderTreeView->expand(index);
        }

        for (int i = 0; i < folderModel->rowCount(index); ++i)
        {
            restore(folderModel->index(i, 0, index));
        }
    };

    for (int i = 0; i < folderModel->rowCount(); ++i)
    {
        restore(folderModel->index(i, 0));
    }
}

// -------------------------------------------------------------------------------------------------------------
/*
void MainWindow::saveCurrentSet()
{
    if (m_updating)
        return;

    if (m_currentSetIndex < 0)
        return;

    FolderSet &set = m_sets[m_currentSetIndex];
    set.roots.clear();

    for (int i = 0; i < folderModel->rowCount(); ++i)
    {
        QStandardItem *nameItem = folderModel->item(i, 0);
        QStandardItem *stateItem = folderModel->item(i, 1);

        set.roots.append(buildNode(nameItem, stateItem));
    }
}
*/
// -------------------------------------------------------------------------------------------------------------
FolderNode MainWindow::buildNode(QStandardItem *nameItem, QStandardItem *stateItem)
{
    FolderNode node;

    node.path = nameItem->text();

    node.state = static_cast<FolderState>(
        stateItem->data(Qt::UserRole).toInt()
        );

    for (int i = 0; i < nameItem->rowCount(); ++i)
    {
        node.children.append(
            buildNode(nameItem->child(i, 0),
                      nameItem->child(i, 1))
            );
    }

    return node;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::onSetChanged(int index)
{
    if (index < 0 || index >= m_sets.size())
        return;

    m_currentSetIndex = index;
    rebuildTree();           // neue Daten anzeigen

    currentSetting = m_sets[m_currentSetIndex].name;
    saveSettings();
}



// -------------------------------------------------------------------------------------------------------------
void MainWindow::reloadSetNames()
{
    setCombo->blockSignals(true);
    setCombo->clear();

    for (const FolderSet &set : std::as_const(m_sets))
    {   setCombo->addItem(set.name);
    }

    setCombo->blockSignals(false);

    // 🔥 entscheidend: verzögert setzen
    QTimer::singleShot(0, this, [this]()
                       {
                           setCombo->setCurrentIndex(m_currentSetIndex);
                       });
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::renameCurrentSet(const QString &newName)
{
    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size())
        return;

    m_sets[m_currentSetIndex].name = newName;

    setCombo->blockSignals(true);
    setCombo->setItemText(m_currentSetIndex, newName);
    setCombo->blockSignals(false);

    currentSetting = newName;
    saveSettings();
}

// -------------------------------------------------------------------------------------------------------------
FolderNode* MainWindow::findNodeByIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return nullptr;

    // Pfad aus UI holen
    QString path = index.sibling(index.row(), 0).data().toString();

    FolderSet &set = m_sets[m_currentSetIndex];

    std::function<FolderNode*(QVector<FolderNode>&)> find;
    find = [&](QVector<FolderNode> &nodes) -> FolderNode*
    {
        for (FolderNode &node : nodes)
        {
            if (node.path == path)
                return &node;

            if (auto child = find(node.children))
                return child;
        }
        return nullptr;
    };

    return find(set.roots);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setNodeStateRecursive(FolderNode &node, FolderState state)
{
    node.state = state;

    for (FolderNode &child : node.children)
    {
        setNodeStateRecursive(child, state);
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setNodeExclDirRecursive(FolderNode &node, bool exclDir)
{
    node.excludeSameFolder = exclDir;

    for (FolderNode &child : node.children)
    {
        setNodeExclDirRecursive(child, exclDir);
    }
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::initializePictureDB()
{
    // Basispfad ~/.config/PictureDB
    // QString basePath = QDir::homePath() + "/.config/PictureDB";
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/PictureDB";
    QDir dir(basePath);

    qDebug() << "Picutre DB location: " << basePath;

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qDebug() << "Could not create folder for Picture DB:" << basePath;
            return false;
        }
    }

    // DB öffnen / erstellen
    QString dbPath = dir.filePath("picturedb.sqlite");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "PictureDBConnection");
    db.setDatabaseName(dbPath);

    bool dbExists = QFile::exists(dbPath);  // ← VORHER prüfen

    if (!db.open()) {
        qDebug() << "DB Fehler:" << db.lastError().text();
        return false;
    }

    QSqlQuery pragma(db);
    if (!pragma.exec("PRAGMA foreign_keys = ON;")) {
        qDebug() << "PRAGMA Fehler:" << pragma.lastError().text();
    }


    if (!dbExists) {
        QSqlQuery query(db);

        // Tabelle 1: Files
        QString createFilesTable = R"(
            CREATE TABLE IF NOT EXISTS Files (
                id INTEGER PRIMARY KEY,
                OrigFilepath TEXT,
                OrigFilename TEXT,
                OrigFilesize INTEGER,
                OrigLastModified INTEGER,
                Filepath TEXT,
                Filename TEXT,
                FilenameStripped TEXT,
                Filesize INTEGER,
                LastModified INTEGER,
                FirstKnownValidFolder TEXT
            );
        )";

        if (!query.exec(createFilesTable)) {
            qDebug() << "Fehler Files Tabelle:" << query.lastError().text();
            return false;
        }

        // Tabelle 2: Features
        QString createFeaturesTable = R"(
            CREATE TABLE IF NOT EXISTS Features (
                file_id     INTEGER PRIMARY KEY,
                KeyPoints   BLOB,
                Descriptors BLOB,
                pHash       INTEGER,
                FOREIGN KEY(file_id) REFERENCES Files(id) ON DELETE CASCADE
            );
        )";

        if (!query.exec(createFeaturesTable)) {
            qDebug() << "Fehler Features Tabelle:" << query.lastError().text();
            return false;
        }


        // Tabelle 3: Version
        QString createVersionTable = R"(
            CREATE TABLE IF NOT EXISTS Version (
                id INTEGER PRIMARY KEY,
                Version INT
            );
        )";

        if (!query.exec(createVersionTable)) {
            qDebug() << "Fehler Version Tabelle:" << query.lastError().text();
            return false;
        }

        query.prepare(R"(
            INSERT OR REPLACE INTO Version
            (id, Version)
            VALUES (1, ?)
        )");
        query.addBindValue(6);    //  <<=== aktuelle Version !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        query.exec();


        // Tabelle 4: Scores
        QString createScoresTable = R"(
            CREATE TABLE IF NOT EXISTS Scores (
                file1_id INTEGER,
                file2_id INTEGER,
                Score REAL,
                manualNoDuplicate INTEGER,
                manNoDuplLastModified INTEGER,
                PRIMARY KEY (file1_id, file2_id),
                FOREIGN KEY(file1_id) REFERENCES Files(id) ON DELETE CASCADE,
                FOREIGN KEY(file2_id) REFERENCES Files(id) ON DELETE CASCADE
            );
        )";

        if (!query.exec(createScoresTable)) {
            qDebug() << "Fehler Scores Tabelle:" << query.lastError().text();
            return false;
        }

        // Index hinzufügen
        query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_files_filename
            ON Files(Filename);
            )");

        query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_files_stripped
            ON Files(FilenameStripped);
            )");


        query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_scores_file1 ON Scores(file1_id);
            )");

        query.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_scores_file2 ON Scores(file2_id);
            )");

    } // of "if db does not exist"
    else {
        QSqlQuery query(db);

        // Version check
        query.prepare(R"(
            SELECT id, Version
            FROM Version
        )");

        if (query.exec() ){
            if (query.next()) {
                int version = query.value(1).toInt();

                if (version < 4) {
                    QMessageBox::warning(this, qtTrId("mainwindow.output.DBwarning"),
                                               qtTrId("mainwindow.output.DBwarning.tooOld"));
                    return false;
                }

                if (version == 4) {

                    QSqlQuery check(db);
                    check.exec("PRAGMA table_info(Features)");
                    bool hasCanny = false;
                    while (check.next()) {
                        if (check.value(1).toString().toLower() == "canny") {
                            hasCanny = true;
                            break;
                        }
                    }

                    if (!hasCanny) {
                        // Add canny Column to
                        QString addCanny = R"(
                            ALTER TABLE Features
                            ADD COLUMN canny BLOB;
                        )";

                        if (!query.exec(addCanny)) {
                            qDebug() << "Fehler Features Tabelle (add canny):" << query.lastError().text();
                            return false;
                        }
                    }

                    QSqlQuery update(db);
                    update.exec("UPDATE Version SET Version = 5;");
                    version = 5;
                }

                if (version == 5) {
                    QSqlQuery check(db);
                    check.exec("PRAGMA table_info(Scores)");
                    bool hasManNoDuplLastModified = false;
                    while (check.next()) {
                        if (check.value(1).toString().toLower() == "manNoDuplLastModified") {
                            hasManNoDuplLastModified = true;
                            break;
                        }
                    }

                    if (!hasManNoDuplLastModified) {
                        // Add hasManNoDuplLastModified Column to Scores
                        QString addManNoDuplLastModified = R"(
                            ALTER TABLE Scores
                            ADD COLUMN manNoDuplLastModified INTEGER;
                        )";

                        if (!query.exec(addManNoDuplLastModified)) {
                            qDebug() << "Fehler Scores Tabelle (add manNoDuplLastModified):" << query.lastError().text();
                            return false;
                        }
                    }

                    QSqlQuery update(db);
                    update.exec("UPDATE Version SET Version = 6;");
                    version = 6;
                }

                if (version == 6) {
/*   DAS IST DIE AKTUELLSTE VERSION
 *   ===============================
 */
                }

                if (version > 6) {
                    QMessageBox::warning(this, qtTrId("mainwindow.output.DBwarning"),
                                               qtTrId("mainwindow.output.DBwarning.tooNew"));
                    return false;
                }

            }
        }
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::startStopScanAndFind()
{
    if (scanActive) {
        stopScan = true;
    } else {
        btnFindDuplicates->setText(qtTrId("mainwindow.button.stopScan"));
        scanAndFindDuplicates();
    }

}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::stopCurrentScan()
{
    setButtonState(true, false);
    scanActive = false;
    stopScan   = false;
    btnFindDuplicates->setText(qtTrId("mainwindow.button.findDupl"));
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::collectedExcludedSameFolders(const QVector<FolderNode> &fn)
{
    for (const auto &f : fn)
    {
        if (f.excludeSameFolder && f.state != Excluded)
            sameFolders.append(f.path);

        if (!f.children.isEmpty())
            collectedExcludedSameFolders(f.children);
    }
}
// -------------------------------------------------------------------------------------------------------------
void MainWindow::scanAndFindDuplicates()
    {
    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size() || scanActive)
        return;

    // Absichern gegen weitere UI Aktionen ------------------------------------------------------------------------------------------------------------
    scanActive = true;
    stopScan   = false;
    setButtonState(false, true);

    // Ordnerliste holen ------------------------------------------------------------------------------------------------------------------------------
    const FolderSet& set = m_sets[m_currentSetIndex];
    allFiles.clear();

    // Excluded Folders sammeln -----------------------------------------------------------------------------------------------------------------------
    sameFolders.clear();
    collectedExcludedSameFolders(set.roots);

    // Dateien sammeln --------------------------------------------------------------------------------------------------------------------------------
    for (const FolderNode& root : set.roots) {
        collectFromFolderNode(root, allFiles);
    }
    qint64 nbrFiles = allFiles.size();
    qInfo() << "Gefundene Dateien:" << nbrFiles;

    if (nbrFiles == 0) {
        QMessageBox::information(this, qtTrId("mainwindow.output.info"),
                                       qtTrId("mainwindow.output.info.noPictures"));

        stopCurrentScan();
        return;
    }

    // Existierende Daten aus DB holen ----------------------------------------------------------------------------------------------------------------
    prepareProgress(qtTrId("mainwindow.label.progressTask.readFileInfoFromDB"), nbrFiles, "black", "gray");
/*
    progressLabel->setText("Read File Info from DB: ");
    progressLabel->setStyleSheet("color: black;");
    progressBar->setRange(0, nbrFiles);
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: white; }");
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();
*/
    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        return;
    }

    qint64 count = 0;
    db.transaction();
    for (auto& f : allFiles) {
        count++;
        if ((count % 100) == 0) {
            progressBar->setValue(count);
            QCoreApplication::processEvents();
        }
        tryMatchInDB(db, f);
    }
    db.commit();

    if (stopScan) {
        stopCurrentScan();
        return;
    }


    // Dateien sammeln, für die ORB / pHash /canny berechnet werden muss ------------------------------------------------------------------------------
    idealCount = QThread::idealThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(idealCount);

    filesToProcess.clear();
    // for (auto& f : allFiles) {
    for (int i = 0; i < allFiles.size(); ++i) {
        if (!(allFiles[i].foundInDB)
             || (allFiles[i].pHash == 0) || (allFiles[i].pHash == 93825012413472)
             || (allFiles[i].canny.isEmpty())
             || (allFiles[i].descriptors.isEmpty()) || (allFiles[i].keyPoints.isEmpty()))
        {
            filesToProcess.append(i);
        }
    }

    // ORB / pHash / canny berechnen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    nbrFiles = filesToProcess.size();
    qInfo() << "Berechnungen für neue ORB / pHash / canny - Start with: " << nbrFiles;


    // Watcher einrichten - Das muss unbedingt gemacht werden, es könnte sein, dass die Score Berechnung noch laufen muss
    if (watcher == nullptr) {
        watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished,
                this, &MainWindow::scanAndFindDuplicates_stepX);
    }

    if (nbrFiles == 0) {
        scanAndFindDuplicates_step2();
        return;
    }

    // ORB/pHash/canny-Berechnungen -------------------------------------------------------------------------------------------------------------------
    prepareProgress(qtTrId("mainwindow.label.progressTask.calculateORBetc"), nbrFiles, "blue", "lightblue");
/*
    progressLabel->setText("Calculate ORB/pHash/Canny");
    progressLabel->setStyleSheet("color: blue;");
    progressBar->setRange(0, nbrFiles);
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: lightblue; }");
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();
*/
    current_step      = 1;
    currentchunkStart = 0;
    chunkSize         = idealCount * 20;

    chunkOfFilesToProcess = filesToProcess.mid(currentchunkStart, chunkSize);

    timer.start(); lastTimeElapsedMs = 0;

    runcalculateORBandPHash();
}


// -------------------------------------------------------------------------------------------------------------
void MainWindow::runcalculateORBandPHash()
{
    watcher->setFuture(
        QtConcurrent::map(chunkOfFilesToProcess, [this](int i) {
//            if (i > 168 && i < 180)
//                qInfo() << "runcalculateORBandPHash - i:" << i << " | " << allFiles[i].path;
            this->calculateORBandPHash(allFiles[i]);
        })
    );

    qInfo() << "scanAndFindDuplicates - watcher läuft (" << currentchunkStart / chunkSize << ")";
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::calculateORBandPHash(FileEntry &entry)
{
    // qInfo() << "calculateORBandPHash - Start: " << entry.path;

    QImage qimage(entry.path);

    if (qimage.isNull()) {
        // Bild konnte nicht geladen werden
        qDebug() << "calculateORBandPHash: QImage hat das Bild nicht laden können: " << entry.path;
        return false;
    }

    // Workaround for Qt6.11.1 Crash of direct conversion to Grayscale8 for some jpg pictures
    // Therefore ".convertToFormat(QImage::Format_RGB888)" is added.
    qimage = qimage.convertToFormat(QImage::Format_RGB888).convertToFormat(QImage::Format_Grayscale8);

//    if (entry.path.contains(".avif"))
//        qInfo() << "calculateORBandPHash - convertToFormat:" << entry.path << " <- ERFOLGREICH";

    cv::Mat image(
        qimage.height(),
        qimage.width(),
        CV_8UC1,
        qimage.bits(),
        qimage.bytesPerLine()
        );

    // cv::Mat image = cv::imread(entry.path.toStdString(), cv::IMREAD_GRAYSCALE);

    if (image.empty()) {
        qDebug() << "calculateORBandPHash: image leer: " << entry.path;
        return false;
    }

    // qInfo() << "calculateORBandPHash - image geladen: " << entry.filename;

    if (!entry.foundInDB || entry.pHash == 0 || entry.pHash == 93825012413472)
    {
        entry.pHash = computePHash(image);
        entry.entryUpdated = true;
    }

    if (entry.canny.isEmpty())
    {
        entry.canny = computeCanny(image);
        if (entry.canny.isEmpty())
            qDebug() << "Canny konnte nicht berechnet werden für: " << entry.path;
        else
            entry.entryUpdated = true;
    }

    // qInfo() << "calculateORBandPHash - pHash: " << entry.pHash << " | " << entry.filename;

    if (!entry.foundInDB || entry.descriptors.isEmpty() || entry.keyPoints.isEmpty())
    {
        entry.entryUpdated = true;

        thread_local cv::Ptr<cv::ORB> orb = cv::ORB::create();

        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        orb->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

        // serialize keypoints
        QByteArray kpData;
        QDataStream kpStream(&kpData, QIODevice::WriteOnly);
        kpStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

        for (const auto &kp : keypoints) {
            kpStream << kp.pt.x << kp.pt.y
                     << kp.size << kp.angle
                     << kp.response << kp.octave
                     << kp.class_id;
        }

        entry.keyPoints   = kpData;
        entry.descriptors = descriptorsToByteArray(descriptors);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------
QByteArray MainWindow::descriptorsToByteArray(const cv::Mat &mat)
{
    if (mat.empty())
        return QByteArray();

    cv::Mat continuous = mat.isContinuous() ? mat : mat.clone();

    return QByteArray(reinterpret_cast<const char*>(continuous.data),
                      static_cast<int>(continuous.total() * continuous.elemSize()));
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::estimatedTime(qint64 done, qint64 volume)
{
    qint64 lastElapsedMs;
    qint64 lastDone;

    qint64 elapsedMs   = timer.elapsed();

    if (lastTimeElapsedMs > 0) {
        lastElapsedMs = elapsedMs - lastTimeElapsedMs;
        lastDone      = done      - lastTimeDone;
    } else {
        lastElapsedMs = elapsedMs;
        lastDone      = done;
    }

    double msPerStep   = (double)lastElapsedMs / lastDone;
    int remainingSteps = volume - done;
    qint64 remainingMs = msPerStep * remainingSteps;
    qint64 totalMs     = elapsedMs + msPerStep * (volume - done);

    int secondsRemain = remainingMs / 1000;
    int minutesRemain = secondsRemain / 60;
    int hoursRemain   = minutesRemain / 60;
    minutesRemain %= 60;
    secondsRemain %= 60;
    QString eta;
    if (hoursRemain > 0)
        eta = QString("%1:%2:%3")
                  .arg(hoursRemain  , 2, 10, QChar('0'))
                  .arg(minutesRemain, 2, 10, QChar('0'))
                  .arg(secondsRemain, 2, 10, QChar('0'));
    else
        eta = QString("%1:%2")
                  .arg(minutesRemain, 2, 10, QChar('0'))
                  .arg(secondsRemain, 2, 10, QChar('0'));


    int secondsTotal = totalMs / 1000;
    int minutesTotal = secondsTotal / 60;
    int hoursTotal   = minutesTotal / 60;
    minutesTotal %= 60;
    secondsTotal %= 60;
    QString total;
    if (hoursTotal > 0)
        total = QString("%1:%2:%3")
                  .arg(hoursTotal  , 2, 10, QChar('0'))
                  .arg(minutesTotal, 2, 10, QChar('0'))
                  .arg(secondsTotal, 2, 10, QChar('0'));
    else
        total = QString("%1:%2")
                  .arg(minutesTotal, 2, 10, QChar('0'))
                  .arg(secondsTotal, 2, 10, QChar('0'));

    //volumeLabel->setText(QLocale().toString(volume) + "\n" + eta  + "(r) | " + total + "(t)");
    volumeLabel->setText(qtTrId("mainwindow.label.progress.eta").arg(QLocale().toString(volume), eta, total));
    QCoreApplication::processEvents();

    lastTimeElapsedMs = elapsedMs;
    lastTimeDone      = done;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::scanAndFindDuplicates_stepX()
{
    if (stopScan) {
        stopCurrentScan();
        return;
    }

    currentchunkStart += chunkSize;
    if (currentchunkStart < filesToProcess.size()) {
        chunkOfFilesToProcess = filesToProcess.mid(currentchunkStart, chunkSize);
        progressBar->setValue(currentchunkStart);
        estimatedTime(currentchunkStart, filesToProcess.size());
    } else {
        volumeLabel->setText(QLocale().toString(filesToProcess.size()));
        progressBar->setValue(filesToProcess.size());
    }
    QCoreApplication::processEvents();


    switch (current_step) {
    case 1:
        if (currentchunkStart < filesToProcess.size()) {
            runcalculateORBandPHash();
        } else
            scanAndFindDuplicates_step2();
        break;

    case 2:
        if (currentchunkStart < filesToProcess.size()) {
            runScanAndFindDuplicates();
        } else
            scanAndFindDuplicates_step3();
        break;

    default: { }
    }
}

// -------------------------------------------------------------------------------------------------------------
QString makeKey(const qint64 & u1, const qint64 & u2)
{
    return QString("%1").arg(u1) + "|" + QString("%2").arg(u2);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::scanAndFindDuplicates_step2()
{
    qint64 nbrFiles = filesToProcess.size();

    // Check, ob Scores zu speichern sind -------------------------------------------------------------------------------------------------------------
    qInfo() << "Check, ob Scores zu speichern sind - Start with: " << nbrFiles;
    bool anythingToStore = false;
    for (auto &i : filesToProcess) {
        anythingToStore |= !allFiles[i].foundInDB || allFiles[i].entryUpdated;
        if (anythingToStore) break;
    }

    // ORB & pHash in DB speichern --------------------------------------------------------------------------------------------------------------------
    if (anythingToStore) {
        qInfo() << "Store ORBs and pHash in DB - Start with: " << nbrFiles;

        QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
        if (!db.isOpen()) {
            qDebug() << "DB is not open!";
            return;
        }

        prepareProgress(qtTrId("mainwindow.label.progressTask.storeORBetc"), nbrFiles, "orange", "orange");
/*
        progressLabel->setText("Store ORB/pHash in DB: ");
        progressLabel->setStyleSheet("color: orange;");
        progressBar->setRange(0, std::max(qint64(1), nbrFiles));
        progressBar->setValue(0);
        progressBar->setStyleSheet("QProgressBar::chunk { background-color: orange; }");
        volumeLabel->setText(QLocale().toString(nbrFiles));
        QCoreApplication::processEvents();
*/
        timer.start(); lastTimeElapsedMs = 0;
        qint64 count = 0;
        for (auto &i : filesToProcess) {
            count++;
            if ((count % 100) == 0) {
                progressBar->setValue(count);
                estimatedTime(count, filesToProcess.size());
                QCoreApplication::processEvents();
            }
            writeOrbAndPHashToDB(db, allFiles[i]);
        }
        progressBar->setValue(nbrFiles);
        volumeLabel->setText(QLocale().toString(nbrFiles));
        QCoreApplication::processEvents();

        if (stopScan) {
            stopCurrentScan();
            return;
        }
    }

    // Pairing List Creation --------------------------------------------------------------------------------------------------------------------------
    nbrFiles = allFiles.size() * (allFiles.size() - 1) / 2;

    qInfo() << "Fetch Scores/Distances from DB - Start with: " << nbrFiles;

    pairs.clear();
    pairs.reserve(nbrFiles);

    pairIndex.clear();

    prepareProgress(qtTrId("mainwindow.label.progressTask.preparePairing"), nbrFiles , "brown", "yellow");
    qsizetype count     = 0;
    qsizetype realCount = 0;
    qsizetype step  = std::max((qsizetype)1, nbrFiles / 100);
    timer.start(); lastTimeElapsedMs = 0;
    for (int i = 0; i < allFiles.size() - 1; ++i) {
        for (int j = i + 1; j < allFiles.size(); ++j)
        {
            count++;
            if ((count % step) == 0) {
                if (stopScan) {
                    stopCurrentScan();
                    return;
                }

                progressBar->setValue(count);
                estimatedTime(count, nbrFiles);
                QCoreApplication::processEvents();
            }

            QString a1 = allFiles[i].path.left(allFiles[i].path.size() - allFiles[i].filename.size() - 1);
            QString a2 = allFiles[j].path.left(allFiles[j].path.size() - allFiles[j].filename.size() - 1);

            bool compareAllowed = true;
            switch (commonSameDir)
            {
            case 0:  // Use individual config per folder
                compareAllowed = !(sameFolders.contains(a1) && sameFolders.contains(a2) && (a1 == a2));
                break;
            case 1:  // Always exclude same folders
                compareAllowed = (a1 != a2);
                break;
            case 2:  // Only allow same folders
                compareAllowed =  (a1 == a2);
                break;
            default:
                break;
            }

            if (compareAllowed)
            {
                realCount++;

                qint64   uuid1 = allFiles[i].uuid;
                qint64   uuid2 = allFiles[j].uuid;
                if (uuid1 > uuid2) std::swap(uuid1, uuid2);

                pairs.push_back({i, j, uuid1, uuid2, 0.0, 0, 0.0,                      // i, j, uuid1, uuid2, score, pHashDistance, cannyDiff
                                 false, orbStatus::Duplicate, false, false, false});   // foundInDB, orbNeeded, deleted, manualNoDuplicate, calculationDone

//                 QString key = makeKey(uuid1, uuid2);
                PairKey key { uuid1, uuid2 };
                int index = pairs.size() - 1;
//                pairIndex[key].append(index);
                pairIndex.insert(key, index);
            }
        }
    }
    // pairs.resize(realCount);

    if (stopScan) {
        stopCurrentScan();
        return;
    }

    // Scores / Distances aus DB holen ----------------------------------------------------------------------------------------------------------------
    prepareProgress(qtTrId("mainwindow.label.progressTask.readScores"), nbrFiles, "red", "#ff8080");   // hell-rot
/*
    progressLabel->setText("Read Scores/pHash from DB: ");
    progressLabel->setStyleSheet("color: red;");
    progressBar->setRange(0, std::max(qint64(1), nbrFiles));
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: #ff8080; }");  // hell-rot
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();
*/

    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        return;
    }


    timer.start(); lastTimeElapsedMs = 0;

    readAllScoresFromDB(db);

    count = 0;
    int foundCount = 0;
    step  = std::max((qsizetype)1, pairs.size() / 100);
    for (auto &f : pairs) {
        count++;
        if ((count % step) == 0) {
            if (stopScan) {
                stopCurrentScan();
                return;
            }

            progressBar->setValue(count);
            estimatedTime(count, pairs.size());
            QCoreApplication::processEvents();
        }

        if (!allFiles[f.i].foundInDB || !allFiles[f.j].foundInDB) {  // if one of the files were not found in DB, then there cannot be a score
/*  Temporary Logging:
            if (!allFiles[f.i].foundInDB) {
                qInfo() << "Read Scores/pHash from DB - Not Found: " << f.i;
                qInfo() << "Read Scores/pHash from DB - Not Found: " << allFiles[f.i].filename;
            }
            if (!allFiles[f.j].foundInDB) {
                qInfo() << "Read Scores/pHash from DB - Not Found: " << f.j;
                qInfo() << "Read Scores/pHash from DB - Not Found: " << allFiles[f.j].filename;
            }
*/
            continue;
        }

        qint64 uuid1 = allFiles[f.i].uuid;
        qint64 uuid2 = allFiles[f.j].uuid;

        if (uuid1 == uuid2) {
            if (allFiles[f.i].size     == allFiles[f.j].size    &&
                allFiles[f.i].filename == allFiles[f.j].filename   )
            {   f.score = 1;
                f.manualNoDuplicate = true;   // To force that it is not shown in duplicate list as identical files
                f.calculationDone   = true;
                f.foundInDB         = true;

            } else {
                qDebug() << "Identical UUIDs: " << uuid1;
                qDebug() << "Identical UUIDs: " << f.i;
                qDebug() << "Identical UUIDs: " << allFiles[f.i].filename;
                qDebug() << "Identical UUIDs: " << f.j;
                qDebug() << "Identical UUIDs: " << allFiles[f.j].filename;
            }

        } else {
            if (uuid1 >  uuid2) std::swap(uuid1, uuid2);

            //    QString key = makeKey(uuid1, uuid2);
            PairKey key { uuid1, uuid2 };

            if (allScoresInDB.contains(key))
            {
                f.score             = allScoresInDB[key].score;
                f.manualNoDuplicate = allScoresInDB[key].manualNoDuplicate;
                f.calculationDone   = true;
                f.foundInDB         = true;
                ++foundCount;
            }
        }
    }

    progressBar->setValue(nbrFiles);
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();

     qInfo() << "Anzahl in DB gefundener Paare: " << foundCount;

    if (stopScan) {
        stopCurrentScan();
        return;
    }

    // pHash-Distances berechnen ----------------------------------------------------------------------------------------------------------------------
    qInfo() << "pHash-Distances berechnen - Start with: " << nbrFiles;
    int maxDistance = 0;
    int minDistance = 30000;
    for (auto &p : pairs) {
        if (p.manualNoDuplicate) {
            p.orbNeeded = orbStatus::noDuplicate;

        } else {
            uint64_t h1 = allFiles[p.i].pHash;
            uint64_t h2 = allFiles[p.j].pHash;

            p.pHashDistance = hammingDistance(h1, h2);
            maxDistance = std::max(maxDistance, p.pHashDistance);
            minDistance = std::min(minDistance, p.pHashDistance);

            if (p.pHashDistance > distanceUpperLimit) { // 15   // HASH COMPARE !!!!!!!!!!!!!!!!!!!!  60  15  25  35
                p.orbNeeded = orbStatus::noDuplicate;   // ❌ garantiert kein Duplikat → ORB überspringen
            }
            else if (p.pHashDistance < distanceLowerLimit) {
                p.orbNeeded = orbStatus::Duplicate;     // ❌ garantiert  ein Duplikat → ORB überspringen
            } else
                p.orbNeeded = orbStatus::calculate;     // ✅ Kandidat → ORB prüfen
        }
    }

    qInfo() << "pHash Distanzen: " << minDistance << " ... " << maxDistance;

    // ORB Not Needed herausfiltern (1) ---------------------------------------------------------------------------------------------------------------
    qInfo() << "ORB Not Needed herausfiltern (1) - Start with: " << nbrFiles;
    filesToProcess.clear();
    for (int i = 0; i < pairs.size(); ++i) {
        if (pairs[i].orbNeeded == orbStatus::calculate) {
            filesToProcess.append(i);
        }
    }
    nbrFiles = filesToProcess.size();

    // Canny Differenz berechnen ----------------------------------------------------------------------------------------------------------------------
    qInfo() << "Canny Differenz berechnen - Start with: " << nbrFiles;
    float maxDiff = 0;
    float minDiff = 30000;

    for (auto &i : filesToProcess) {
        // qInfo() << "i:" << i;
        if (i == 6147743)
            int a=1;
        auto &p = pairs[i];
        QVector<float> c1 = allFiles[p.i].canny;
        QVector<float> c2 = allFiles[p.j].canny;

        p.cannyDiff = compareCanny(c1, c2);
        maxDiff = std::max(maxDiff, p.cannyDiff);
        minDiff = std::min(minDiff, p.cannyDiff);

        if (p.cannyDiff > cannyDiffUpperLimit) {
            // if (!p.foundInDB) p.score = 0;
            p.orbNeeded = orbStatus::noDuplicate;   // ❌ garantiert kein Duplikat → ORB überspringen
        }
        else if (p.cannyDiff < cannyDiffLowerLimit) {
            // if (!p.foundInDB || p.score == 0) p.score = 1;
            p.orbNeeded = orbStatus::Duplicate;   // ❌ garantiert  ein Duplikat → ORB überspringen
        } else
            p.orbNeeded = orbStatus::calculate;    // ✅ Kandidat → ORB prüfen
    }

    qInfo() << "Canny Differenzen: " << minDiff << " ... " << maxDiff;

    // ORB Not Needed herausfiltern (2) ---------------------------------------------------------------------------------------------------------------
    qInfo() << "ORB Not Needed herausfiltern (2) - Start with: " << nbrFiles;

    for (int i=filesToProcess.size()-1; i >= 0; --i) {
        if (pairs[filesToProcess[i]].orbNeeded != orbStatus::calculate) {
            filesToProcess.removeAt(i);
        }
    }
    nbrFiles = filesToProcess.size();

    // Nur nicht gefundene Pairs behalten -------------------------------------------------------------------------------------------------------------
    qInfo() << "Nur nicht gefundene Pairs behalten - Start with: " << nbrFiles;
    for (int i=filesToProcess.size()-1; i >= 0; --i) {
        auto &p = pairs[filesToProcess[i]];
        if (p.foundInDB && p.score != 1) {      //  p.score != 1  <= da es noch Relikte mit 'künstlichem' "p.score == 1" geben kann
            filesToProcess.removeAt(i);
        }
    }
    nbrFiles = filesToProcess.size();

    // Zum nächsten Step, wenn es nichts zu tun gibt --------------------------------------------------------------------------------------------------
    if (nbrFiles == 0) {
        scanAndFindDuplicates_step3();
        return;
    }

    // Bucket Map erzeugen ----------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Bucket Map erzeugen: ";
    QHash<uint16_t, QVector<int>> bucketMap;
    for (auto &i : filesToProcess) {
        uint bucket1 = allFiles[pairs[i].i].pHash >> 48;
        uint bucket2 = allFiles[pairs[i].j].pHash >> 48;

        bucketMap[bucket1].push_back(i);
        bucketMap[bucket2].push_back(i);
    }




    // Valid Buckets sammeln --------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Bucket Map Größe: " << bucketMap.size();
    QSet<uint16_t> validBuckets;
    for (auto bucket : bucketMap.keys())
    {   for (int offset = -1; offset <= 1; ++offset)
        {   validBuckets.insert(bucket + offset);
        }
    }

    // Bucket Map erzeugen ----------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Valid Bucket Map Größe: " << validBuckets.size();
    filesToProcess.erase(
        std::remove_if(filesToProcess.begin(), filesToProcess.end(),
                       [&](int i)
                       {
                           uint16_t b1 = allFiles[pairs[i].i].pHash >> 48;
                           uint16_t b2 = allFiles[pairs[i].j].pHash >> 48;

                           return !(validBuckets.contains(b1) || validBuckets.contains(b2));
                       }),
        filesToProcess.end());

    nbrFiles = filesToProcess.size();


    // Scores berechnen -------------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Calculate Scores - Start with: " << nbrFiles;

    const auto &localFiles = allFiles;

    prepareProgress(qtTrId("mainwindow.label.progressTask.calcScores"), nbrFiles, "darkGreen", "#45FF45");   // green
/*
    progressLabel->setText("Calculate Scores: ");
    progressLabel->setStyleSheet("color: darkGreen;");
    progressBar->setRange(0, std::max(qint64(1), nbrFiles));
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: #45FF45; }");   // green
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();
*/

    done = 0;

    current_step      = 2;
    currentchunkStart = 0;
    chunkSize         = idealCount * 20;  // in echt: 1000;

    chunkOfFilesToProcess = filesToProcess.mid(currentchunkStart, chunkSize);

    timer.start(); lastTimeElapsedMs = 0;
    runScanAndFindDuplicates();
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::runScanAndFindDuplicates()
{
    watcher->setFuture(
        QtConcurrent::map(filesToProcess, [this](int idx) {
            CompareInfo &p = pairs[idx];

            if (allFiles[p.i].uuid == allFiles[p.j].uuid) {  // identische uuid => score = 1
                p.score = 1.0;

            } else if (p.calculationDone) {  // calculation could already be done by reading from DB
                return;

            } else {
                bool found = false;
                for (int pd = 0; pd < currentchunkStart + chunkSize; ++pd) {  // search, whether this uuid pair was already calculated with other images indices
                    if (pairs[pd].calculationDone  &&
                        pairs[pd].uuid1 == p.uuid1 &&
                        pairs[pd].uuid2 == p.uuid2   ) {
                        found = true;
                        p.score = pairs[pd].score;
                        break;
                    }
                }
                if (!found) {
                    const QByteArray &d1 = allFiles[p.i].descriptors;
                    const QByteArray &d2 = allFiles[p.j].descriptors;

                    p.score = compareORB(d1, d2);
                    p.calculationDone = true;

                    // ++done;
                }
            }
        })
    );

    qInfo() << "scanAndFindDuplicates_step2 - watcher läuft (" << currentchunkStart / chunkSize << ")";
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::scanAndFindDuplicates_step3()
{
    if (progressTimer) progressTimer->stop();

    // Scores in DB speichern ----------------------------------------------------------------------------------------------------------------------------
    qint64 nbrFiles = filesToProcess.size();
    qInfo() << "Store Scores in DB - Start with: " << nbrFiles;

    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        return;
    }

    prepareProgress(qtTrId("mainwindow.label.progressTask.storeScores"), nbrFiles, "darkcyan", "cyan");
/*
    progressLabel->setText("Store Scores in DB: ");
    progressLabel->setStyleSheet("color: darkcyan;");
    progressBar->setRange(0, std::max(qint64(1), nbrFiles));
    progressBar->setValue(0);
    progressBar->setStyleSheet("QProgressBar::chunk { background-color: cyan; }");
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();
*/

    timer.start(); lastTimeElapsedMs = 0;
    qint64 count = 0;
    for (auto &i : filesToProcess) {
        count++;
        if ((count % 100) == 0) {
            if (stopScan) {
                stopCurrentScan();
                return;
            }

            progressBar->setValue(count);
            estimatedTime(count, nbrFiles);
        }
        writeScoreToDB(db, pairs[i]);
    }
    progressBar->setValue(nbrFiles);
    volumeLabel->setText(QLocale().toString(nbrFiles));
    QCoreApplication::processEvents();

    if (stopScan) {
        stopCurrentScan();
        return;
    }

    CalculateDuplicateList(true);
    stopCurrentScan();
    prepareProgress(qtTrId("mainwindow.label.progressTask.Idle"), -1, "black", "black");
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::CalculateDuplicateList(bool newData) {

    if (!newData) {   // pairs may contain entries which are marked as 'deleted'
        for (int i=pairs.size()-1; i >= 0; --i)
            if (pairs[i].deleted)
                pairs.removeAt(i);  // only at this point the real deletion in pairs can happen, as the UI will be set up completely new
    }

    // Create Duplicate List  ----------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Create Duplicate List - Start with: " << pairs.size();
    allDupl.clear();
    for (auto &f : pairs) {
        if (f.orbNeeded != orbStatus::noDuplicate && (f.score > scoreLimit || f.orbNeeded == orbStatus::Duplicate)) {
            allDupl.emplace_back(f.i, f.j,
                                 (f.orbNeeded == orbStatus::calculate)? f.score :
                                     (f.score == 0 || f.score == 1)? -1.0 : f.score,
                                 f.pHashDistance, f.cannyDiff);
        }
    }

    qint64 nbrDupl = allDupl.size();

    // Fertig, wenn keine Duplikate gefunden ----------------------------------------------------------------------------------------------------------
    if (nbrDupl == 0) {
        if (!m_duplicateView)
            QMessageBox::information(this, qtTrId("mainwindow.output.info"),
                                     qtTrId("mainwindow.output.info.noDupl"));

        else {
            QMessageBox::StandardButton reply =
                QMessageBox::question(this, qtTrId("mainwindow.output.info"),
                                     qtTrId("mainwindow.output.info.noDuplShowOldList"));


            if (reply == QMessageBox::Yes) {
                m_duplicateView->deleteLater();
                m_duplicateView = nullptr;
            }
        }
        return;
    }


    // Create parent image list -----------------------------------------------------------------------------------------------------------------------
    qInfo() << "Create parent image list: " << nbrDupl;
    QVector<qint64>parentUUIDs;
    QVector<QString>parentPaths;
    QVector<int>parentIndex;
    QVector<bool>parentItself;
    for (int i=0; i < nbrDupl; ++i) {
        int img1 = allDupl[i].indexImg1;
        int img2 = allDupl[i].indexImg2;

        qint64 uuid1 = allFiles[img1].uuid;
        qint64 uuid2 = allFiles[img2].uuid;

        int index1 = parentUUIDs.indexOf(uuid1);
        if (index1 == -1) {
            parentUUIDs.append(uuid1);
            parentPaths.append(allFiles[img1].parentPath);
            parentIndex.append(img1);
            parentItself.append(allFiles[img1].parentPath == allFiles[img1].path);
        } else if (allFiles[img1].parentPath == allFiles[img1].path)
            parentIndex[index1] = img1;   // Parent Index überschreiben, weil Parent selbst enthalten ist

        int index2 = parentUUIDs.indexOf(uuid2);
        if (index2 == -1) {
            parentUUIDs.append(uuid2);
            parentPaths.append(allFiles[img2].parentPath);
            parentIndex.append(img2);
            parentItself.append(allFiles[img2].parentPath == allFiles[img2].path);
        } else if (allFiles[img2].parentPath == allFiles[img2].path)
            parentIndex[index2] = img2;   // Parent Index überschreiben, weil Parent selbst enthalten ist
    }

    // Create list of duplicate images ----------------------------------------------------------------------------------------------------------------
    qInfo() << "Create list of duplicate images - Start with: " << nbrDupl;

    if (nbrDupl > maxDuplToShow) {
        QMessageBox::warning(this, qtTrId("mainwindow.output.warning"),
                                   qtTrId("mainwindow.output.warning.tooManyDupl").arg(nbrDupl, maxDuplToShow));

        allDupl.erase(allDupl.begin() + maxDuplToShow, allDupl.end());

        nbrDupl = allDupl.size();
        qInfo() << "Create list of duplicate images - Reduced to: " << nbrDupl;
    }


    QVector<DuplicateImage> duplImages;

    for (int i=0; i < nbrDupl; ++i) {
        int foundImage1 = -1;
        int foundImage2 = -1;

        for (int j=0; j <= duplImages.size() - 1; ++j) {
            if (duplImages[j].indexImg == allDupl[i].indexImg1) {
                foundImage1 = j;
            } else if (duplImages[j].indexImg == allDupl[i].indexImg2) {
                foundImage2 = j;
            }
            if ((foundImage1 >= 0) && (foundImage2 >= 0)) break;
        }

        if (foundImage1 == -1) {
            duplImages.emplace_back( DuplicateImage(allDupl[i].indexImg1,
                                                    allFiles[allDupl[i].indexImg1].path,
                                                    allFiles[allDupl[i].indexImg1].filename,
                                                    allFiles[allDupl[i].indexImg1].referenceImg
                                                   ) );
            foundImage1 = duplImages.size() - 1;

            // check in parent list
            int index1 = parentUUIDs.indexOf(allFiles[allDupl[i].indexImg1].uuid);
            if (allDupl[i].indexImg1 == parentIndex[index1]) {
                duplImages[foundImage1].parentImg = true;
            }
            duplImages[foundImage1].parentIdx = parentIndex[index1];
        }

        if (foundImage2 == -1) {
            duplImages.emplace_back( DuplicateImage(allDupl[i].indexImg2,
                                                    allFiles[allDupl[i].indexImg2].path,
                                                    allFiles[allDupl[i].indexImg2].filename,
                                                    allFiles[allDupl[i].indexImg2].referenceImg
                                                   ) );
            foundImage2 = duplImages.size() - 1;

            // check in parent list
            int index2 = parentUUIDs.indexOf(allFiles[allDupl[i].indexImg2].uuid);
            if (allDupl[i].indexImg2 == parentIndex[index2]) {
                duplImages[foundImage2].parentImg = true;
            }
            duplImages[foundImage2].parentIdx = parentIndex[index2];
        }

        duplImages[foundImage1].duplIndices.emplaceBack(foundImage2);
        duplImages[foundImage1].duplallFilesIndices.emplaceBack(duplImages[foundImage2].indexImg);
        duplImages[foundImage1].duplScores.emplaceBack(allDupl[i].score);
        duplImages[foundImage1].duplHashDistances.emplaceBack(allDupl[i].pHashDistance);
        duplImages[foundImage1].duplCannyDiff.emplaceBack(allDupl[i].cannyDiff);

        duplImages[foundImage2].duplIndices.emplaceBack(foundImage1);
        duplImages[foundImage2].duplallFilesIndices.emplaceBack(duplImages[foundImage1].indexImg);
        duplImages[foundImage2].duplScores.emplaceBack(allDupl[i].score);
        duplImages[foundImage2].duplHashDistances.emplaceBack(allDupl[i].pHashDistance);
        duplImages[foundImage2].duplCannyDiff.emplaceBack(allDupl[i].cannyDiff);

/*
        if (duplImages[foundImage1].filepath == allFiles[allDupl[i].indexImg2].parentPath ||  // ich bin der Parent
            allFiles[foundImage1].parentPath == allFiles[allDupl[i].indexImg2].parentPath   ) {
            duplImages[foundImage2].parentIdx = duplImages[foundImage1].indexImg;
            duplImages[foundImage1].parentImg = true;
        }

        if (duplImages[foundImage2].filepath == allFiles[allDupl[i].indexImg1].parentPath ||  // ich bin der Parent
            allFiles[foundImage2].parentPath == allFiles[allDupl[i].indexImg1].parentPath   ) {
            duplImages[foundImage1].parentIdx = duplImages[foundImage2].indexImg;
            duplImages[foundImage2].parentImg = true;
        }
*/
    }

    // Füge Width und Height hinzu --------------------------------------------------------------------------------------------------------------------
    qInfo() << "Füge Width und Height hinzu - Start with: " << duplImages.size();
    for (auto &f : duplImages) {
        QImageReader reader(f.filepath);
        QSize size = reader.size();

        f.width  = (size.width()  > 0)? size.width()  : -1;
        f.height = (size.height() > 0)? size.height() : -1;
    }

/*
    // References und echte Parents zuerst !!! --------------------------------------------------------------------------------------------------------
    // !!!!!! ECHTE PARENTS MÜSSEN HIER AM ANFANG STEHEN !!!!!!!
    qInfo() << "Sort duplImages list - References und echte Parents am Anfang: " << duplImages.size();
    std::sort(duplImages.begin(), duplImages.end(),
              [](const DuplicateImage &a, const DuplicateImage &b) {

                  if (a.referenceImg != b.referenceImg)
                      return a.referenceImg > b.referenceImg;

                  if (a.parentImg != b.parentImg)
                      return a.parentImg > b.parentImg;

                  if ( (a.width * a.height) != (b.width * b.height) )
                      return(a.width * a.height) > (b.width * b.height);

                  return a.filepath < b.filepath;
              }
              );
*/

    // Überflüssige Referenzen entfernen ----------------------------------------------------------------------------------------------------------------
    qInfo() << "Überflüssige Referenzen entfernen - Start with: " << duplImages.size();
    for (int fIdx = 0; fIdx < duplImages.size(); ++fIdx) {
        auto &f = duplImages[fIdx];

        // First check whether any Reference or Parent
        int  refImg      = f.referenceImg? fIdx : -1;
        int  parentImg   = f.parentImg   ? fIdx : -1;
        int  bigImg      = -1;
        long bigImgSize  = (long)f.height * f.width;
        int  maxListe    = -1;
        int  maxListSize = f.duplIndices.size();
        // !!!!! -> Finde das größte Bild !!! -> Hat Vorrang vor Listengröße !!!!!!

        for (auto &r : f.duplIndices) {
            if (duplImages[r].referenceImg && refImg    == -1) refImg    = r;
            if (duplImages[r].parentImg    && parentImg == -1) parentImg = r;

            if ((duplImages[r].height * duplImages[r].width) > bigImgSize) {
                bigImgSize  = (long)duplImages[r].height * duplImages[r].width;
                bigImg      = r;
            }

            if (duplImages[r].duplIndices.size() > maxListSize) {
                maxListSize = duplImages[r].duplIndices.size();
                maxListe    = r;
            }
        }

        int selectedIndex = (refImg    >= 0)? refImg :
                            (parentImg >= 0)? parentImg :
                            (bigImg    >= 0)? bigImg :
                            (maxListe  >= 0)? maxListe : -1;

        if (selectedIndex != -1) {
            for (auto &r : f.duplIndices) {
                // Delete in the lists of the others
                for (int i=duplImages[r].duplIndices.size() - 1; i >= 0; --i) {
                    if (r != selectedIndex                                        &&
                        (duplImages[r].duplIndices[i] == fIdx                  ||
                        (f.duplIndices.contains(duplImages[r].duplIndices[i]))   )  ) {
                        duplImages[r].duplIndices.removeAt(i);
                        duplImages[r].duplallFilesIndices.removeAt(i);
                        duplImages[r].duplScores.removeAt(i);
                        duplImages[r].duplHashDistances.removeAt(i);
                        duplImages[r].duplCannyDiff.removeAt(i);
                    }
                }
            }

            // Clear my own lists
            if (selectedIndex != fIdx) {
                f.duplIndices.clear();
                f.duplallFilesIndices.clear();
                f.duplScores.clear();
                f.duplHashDistances.clear();
                f.duplCannyDiff.clear();
            }
        }

        // Nun lösche überflüssiges:
        // Als "Primär" gefundene Liste bleibt unverändert
        // der eigene Indix wird aus den Listen der anderen entfernt
        //
        // Wenn refImg    >= 0      -> hat Vorrang 1
        // Wenn parentImg >= 0      -> hat Vorrang 2
        // Danach nach Bild-Größe   -> das Größte hat Vorrang 3
        // Danach nach Listen-Größe -> längste Liste hat Vorrang 4
/*
        for (auto &r : f.duplIndices) {
            bool reduceThisList = true;
            if (refImg >= 0) {
                if (r == refImg)
                    reduceThisList = false;
            } else if (parentImg >= 0) {
                if(r == parentImg)
                    reduceThisList = false;
            } else if (maxListe >= 0 ) {
                if (r == maxListe)
                    reduceThisList = false;
            } else if ((f.width * f.height) >= (duplImages[r].width * duplImages[r].height))
                 reduceThisList = false;

            if (reduceThisList)
//            if (f.referenceImg ||
//                ((f.width * f.height) >= (duplImages[r].width * duplImages[r].height)) ||
//                (duplImages[fIdx].indexImg == duplImages[r].parentIdx)
//               )
            {
                for (int i=duplImages[r].duplIndices.size() - 1; i >= 0; --i) {
                    if (duplImages[r].duplIndices[i] == r ) {   // bisher war es immer fIdx
                        duplImages[r].duplIndices.removeAt(i);
                        duplImages[r].duplallFilesIndices.removeAt(i);
                        duplImages[r].duplScores.removeAt(i);
                        duplImages[r].duplHashDistances.removeAt(i);
                        duplImages[r].duplCannyDiff.removeAt(i);
                    }
                }
            }
        }
*/
    }

    // Sort list, items with most children first, then reference, then parents, then the rest ---------------------------------------------------------
    qInfo() << "Sort duplImages list - Start with: " << duplImages.size();
    std::sort(duplImages.begin(), duplImages.end(),
              [](const DuplicateImage &a, const DuplicateImage &b) {

                bool t1 = (a.duplIndices.size() > b.duplIndices.size());
                if (a.duplIndices.size()  != b.duplIndices.size())
                    return (a.duplIndices.size() > b.duplIndices.size());

                if (a.referenceImg != b.referenceImg)
                    return a.referenceImg > b.referenceImg;

                if (a.parentImg != b.parentImg)
                    return a.parentImg > b.parentImg;

                if ( (a.width * a.height) != (b.width * b.height) )
                    return(a.width * a.height) > (b.width * b.height);

                return a.filepath < b.filepath;
              }
    );


    // Schreibe Ergebnisse in ein csv File ---------------------------------------------------------------------------------------------------------------
    qInfo() << "Schreibe Ergebnisse in ein csv File - Start with: " << duplImages.size();
    writeToCsv(duplImages);

    // Finde Duplicate Groups ----------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Finde Duplicate Groups (imageDupl) - Start with: " << duplImages.size();
    imageDupl.clear();
    skipCount = 0;

    int   maxDistance = 0;
    int   minDistance = 30000;
    float maxDiff     = 0;
    float minDiff     = 30000;

    for (auto &dI : duplImages) {
        // First check whether current image is in a children list
        bool found = false;
        for (auto &d : imageDupl) {
            for (auto &i : d.items) {
                if (dI.filepath == i.info.path) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        // if found, skip it
        if (found) continue;

        if (dI.duplallFilesIndices.size() > 0) {
            // root erzeugen
            ImageInfo info;
            info.path = dI.filepath;
            info.filename = dI.filename;
            info.size = allFiles[dI.indexImg].size;
            info.lastModified = allFiles[dI.indexImg].lastModified;
            info.width = dI.width;
            info.height = dI.height;

            imageDupl.emplace_back( info );

            // children hinzufügen
            for (int c=0; c < dI.duplallFilesIndices.size(); ++c) {
                int cIdx = dI.duplallFilesIndices[c];

                info.path = allFiles[cIdx].path;
                info.filename = allFiles[cIdx].filename;
                info.size = allFiles[cIdx].size;
                info.lastModified = allFiles[cIdx].lastModified;

                int foundIdx = -1;
                for (int dI2=0; dI2 <= duplImages.size() - 1; ++dI2) {
                    if (duplImages[dI2].indexImg == cIdx) {
                        foundIdx = dI2;
                        break;
                    }
                }

                info.width  = (foundIdx >= 0)? duplImages[foundIdx].width : 0;
                info.height = (foundIdx >= 0)? duplImages[foundIdx].height : 0;

                int newIdx = imageDupl.size() - 1;
                Type t = (dI.parentIdx >= 0 && dI.parentIdx != dI.indexImg)? Copy : Duplicate;
                imageDupl[newIdx].items.emplace_back(info, t, dI.duplScores[c], dI.duplHashDistances[c], dI.duplCannyDiff[c]);

                minDistance = std::min(dI.duplHashDistances[c], minDistance);
                maxDistance = std::max(dI.duplHashDistances[c], maxDistance);

                minDiff = std::min(dI.duplCannyDiff[c], minDiff);
                maxDiff = std::max(dI.duplCannyDiff[c], maxDiff);
            }
        }
    }

    qInfo() << "Für alle angezeigten Bilder - minDistance: " << minDistance;
    qInfo() << "Für alle angezeigten Bilder - maxDistance: " << maxDistance;

    // Öffne Fenster zum Anzeigen ------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Öffne Fenster zum Anzeigen";
    if (m_duplicateView)
    {
        m_duplicateView->deleteLater();
        m_duplicateView = nullptr;
    }

    m_duplicateView = new DuplicateViewWidget(nullptr, 0  /* 0 = List of 'Find Duplicates' */, scoreLimit);
    m_duplicateView->setAttribute(Qt::WA_DeleteOnClose);

    auto *view = m_duplicateView;
    connect(m_duplicateView, &QObject::destroyed,
            this, [this, view]() {
        if (m_duplicateView == view)
            m_duplicateView = nullptr;

        scanActive = false;
        stopScan   = true;
        setButtonState(true, true);
    });

    connect(m_duplicateView, &DuplicateViewWidget::scoreHasChanged,
            this, [=]() {
                forcedApply = true;
                QSettings settings("DuplicateFinder", "DuplicateFinder");
                scoreLimit = settings.value("Program/scoreLimit").toInt() / 100.0;
                sliderScoreLimit->setValue((int)(scoreLimit * 100));
                forcedApply = false;

                CalculateDuplicateList(false);  // run on existing data, which might have deleted pairs
            });

    connect(m_duplicateView, &DuplicateViewWidget::refreshList,
            this, [=]() {
                CalculateDuplicateList(false);  // run on existing data, which might have deleted pairs
            });

    connect(m_duplicateView, &DuplicateViewWidget::deleteRequested,
            this, &MainWindow::deleteImage);

    connect(m_duplicateView, &DuplicateViewWidget::makeMasterRequested,
            this, &MainWindow::makeMaster);

    connect(m_duplicateView, &DuplicateViewWidget::noDuplicates,
            this, &MainWindow::noDuplicates);


    connect(this, &MainWindow::deleteResult,
            m_duplicateView, &DuplicateViewWidget::onDeleteResult);

    connect(this, &MainWindow::makeMasterResult,
            m_duplicateView, &DuplicateViewWidget::onMakeMasterResult);

    connect(this, &MainWindow::noDuplicatesResult,
            m_duplicateView, &DuplicateViewWidget::onNoDuplicateResult);

    m_duplicateView->setData(imageDupl);
    m_duplicateView->resize(900, 600);  //  später  Größe aus preferences holen
    m_duplicateView->setAttribute(Qt::WA_DeleteOnClose);
    m_duplicateView->show();
}


// -------------------------------------------------------------------------------------------------------------
void MainWindow::ShowManualDuplicateList(bool newData)
{
    // Finde Duplicate Groups ----------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Manual No Duplicates";
    imageDupl.clear();

    for (auto &p : pairs)
    {
        if (p.manualNoDuplicate)
        {
            ImageInfo ii1;
            ii1.filename     = allFiles[p.i].filename;
            ii1.lastModified = allFiles[p.i].lastModified;
            ii1.path         = allFiles[p.i].path;
            ii1.size         = allFiles[p.i].size;

            imageDupl.emplace_back( ii1 );
            int newIdx = imageDupl.size() - 1;

            ImageInfo ii2;
            ii2.filename     = allFiles[p.j].filename;
            ii2.lastModified = allFiles[p.j].lastModified;
            ii2.path         = allFiles[p.j].path;
            ii2.size         = allFiles[p.j].size;

            imageDupl[newIdx].items.emplace_back(ii2, Duplicate, 0.0, 0, 0, p.manTime);
        }
    }

    if (imageDupl.isEmpty())
    {
        if (m_duplicateView)
        {
            m_duplicateView->deleteLater();
            m_duplicateView = nullptr;
        }

        QMessageBox::information(this, qtTrId("mainwindow.output.info"),
                                       qtTrId("mainwindow.output.info.noManualNoDupl"));

        scanActive = false;
        stopScan   = true;
        setButtonState(true, true);

        return;
    }

    // Abfragen nach anzuzeigenden Listengröße und Sortierung
    if (newData)
    {
        auto *dlg = new noManualParametersDialog(&noManualSort, &noManualSortDir, &noManualListStart, &noManualListEnd, imageDupl.size(), this);
        if (dlg->exec() != QDialog::Accepted)
        {
            scanActive = false;
            stopScan   = true;
            setButtonState(true, true);

            return;
        }
    }

    if (noManualSort == 4)
    {    qInfo() << "Füge Width und Height hinzu - Start with: " << imageDupl.size();
        for (auto &f : imageDupl) {
            QImageReader reader1(f.root.path);
            QSize size = reader1.size();

            f.root.width  = (size.width()  > 0)? size.width()  : -1;
            f.root.height = (size.height() > 0)? size.height() : -1;
        }
    }

    // Sortieren der Liste
    std::sort(imageDupl.begin(), imageDupl.end(),
              [=](const DuplicateCluster &a, const DuplicateCluster &b) {

        if (noManualSort == 0)
        {
            if (a.items[0].manTime > b.items[0].manTime) return (noManualSortDir == 0);
            if (a.items[0].manTime < b.items[0].manTime) return (noManualSortDir == 1);
        }
        if (noManualSort == 2)
        {
            if (a.root.size < b.root.size) return (noManualSortDir == 0);
            if (a.root.size > b.root.size) return (noManualSortDir == 1);
        }
        if (noManualSort == 3)
        {
            if (a.root.lastModified < b.root.lastModified) return (noManualSortDir == 0);
            if (a.root.lastModified > b.root.lastModified) return (noManualSortDir == 1);
        }
        if (noManualSort == 4)
        {
            if ((a.root.width * a.root.height) < (b.root.width * b.root.height)) return (noManualSortDir == 0);
            if ((a.root.width * a.root.height) > (b.root.width * b.root.height)) return (noManualSortDir == 1);
        }
        if (a.root.path.toLower() < b.root.path.toLower()) return (noManualSortDir == 0);
        if (a.root.path.toLower() > b.root.path.toLower()) return (noManualSortDir == 1);
        return (a.root.size < b.root.size)? (noManualSortDir == 0) : (noManualSortDir == 1);
    });

    // Liste kürzen
    imageDupl = imageDupl.mid(noManualListStart, noManualListEnd - noManualListStart + 1);


    if (imageDupl.size() > maxDuplToShow) {
        QMessageBox::warning(this, qtTrId("mainwindow.output.warning"),
                                   qtTrId("mainwindow.output.warning.tooManyManNoDupl").arg(imageDupl.size(), maxDuplToShow));

        imageDupl.erase(imageDupl.begin() + maxDuplToShow, imageDupl.end());

        qInfo() << "Create list of duplicate images - Reduced to: " << imageDupl.size();
    }

    // Füge Width und Height hinzu --------------------------------------------------------------------------------------------------------------------
    qInfo() << "Füge Width und Height hinzu - Start with: " << imageDupl.size();
    for (auto &f : imageDupl)
    {
        QSize size;

        if (noManualSort != 4)
        {
            QImageReader reader1(f.root.path);
            size = reader1.size();

            f.root.width  = (size.width()  > 0)? size.width()  : -1;
            f.root.height = (size.height() > 0)? size.height() : -1;
        }

        QImageReader reader2(f.items[0].info.path);
        size = reader2.size();

        f.items[0].info.width  = (size.width()  > 0)? size.width()  : -1;
        f.items[0].info.height = (size.height() > 0)? size.height() : -1;
    }


    // Fenster anzeigen -------------------------------------------------------------------------------------------------------------------------------
    qInfo() << "Öffne Fenster zum Anzeigen - Manual No Duplicates - " << newData;
    if (m_duplicateView)
    {
        m_duplicateView->deleteLater();
        m_duplicateView = nullptr;
    }

    m_duplicateView = new DuplicateViewWidget(nullptr, 1  /* 0 = List of 'Manual No Duplicates' */, scoreLimit);
    m_duplicateView->setAttribute(Qt::WA_DeleteOnClose);

    auto *view = m_duplicateView;
    connect(m_duplicateView, &QObject::destroyed,
            this, [this, view]() {
                if (m_duplicateView == view)
                    m_duplicateView = nullptr;

                scanActive = false;
                stopScan   = true;
                setButtonState(true, true);
            });

    connect(m_duplicateView, &DuplicateViewWidget::refreshList,
            this, [=]() {
                ShowManualDuplicateList(false);  // run on existing data, which might have deleted pairs
            });

    // connect(m_duplicateView, &DuplicateViewWidget::deleteRequested,
    //          this, &MainWindow::deleteImage);

    connect(m_duplicateView, &DuplicateViewWidget::noDuplicates,
            this, &MainWindow::noDuplicates);


    // connect(this, &MainWindow::deleteResult,
    //         m_duplicateView, &DuplicateViewWidget::onDeleteResult);

    connect(this, &MainWindow::noDuplicatesResult,
            m_duplicateView, &DuplicateViewWidget::onNoDuplicateResult);

    m_duplicateView->setData(imageDupl);
    m_duplicateView->resize(900, 600);  //  später  Größe aus preferences holen
    m_duplicateView->setAttribute(Qt::WA_DeleteOnClose);
    m_duplicateView->show();

    prepareProgress(qtTrId("mainwindow.label.progressTask.Idle"), -1, "black", "black");
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::findFolderInSet(const QVector<FolderNode> &fn, QString f)
{
    for (auto &e : fn)
    {
        if (e.path == f && e.state != Excluded)
            return true;

        if (findFolderInSet(e.children, f))
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::FetchManualDuplicateList()
{
    if (m_currentSetIndex < 0 || m_currentSetIndex >= m_sets.size() || scanActive)
        return;

    // Absichern gegen weitere UI Aktionen ------------------------------------------------------------------------------------------------------------
    scanActive = true;
    stopScan   = false;
    setButtonState(false, true);

    // use 'pairs' to store manual Duplicates pairs
    pairs.clear();

    // Ordnerliste holen ------------------------------------------------------------------------------------------------------------------------------
    const FolderSet& set = m_sets[m_currentSetIndex];

    // Excluded Folders sammeln -----------------------------------------------------------------------------------------------------------------------
    sameFolders.clear();
    collectedExcludedSameFolders(set.roots);

    // Collect all 'manuals' from database
    allManualsInDB.clear();
    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        return;
    }
    readManualEntriesFromDB(db);

    // Use those entries for which the folders of both pictures are in this list and add them to 'pairs'
    allFiles.clear();
    for (auto it = allManualsInDB.cbegin(); it != allManualsInDB.cend(); ++it)
    {
        const PairKey    &key = it.key();
        const ManualInfo &p   = it.value();

        QString folder1 = QFileInfo(p.f1_filepath).absolutePath();
        QString folder2 = QFileInfo(p.f2_filepath).absolutePath();

        QString a1 = p.f1_filepath.left(p.f1_filepath.size() - p.f1_filename.size() - 1);
        QString a2 = p.f2_filepath.left(p.f2_filepath.size() - p.f2_filename.size() - 1);

        bool compareAllowed = true;
        switch (commonSameDir)
        {
        case 0:  // Use individual config per folder
            compareAllowed = !(sameFolders.contains(a1) && sameFolders.contains(a2) && (a1 == a2));
            break;
        case 1:  // Always exclude same folders
            compareAllowed = (a1 != a2);
            break;
        case 2:  // Only allow same folders
            compareAllowed =  (a1 == a2);
            break;
        default:
            break;
        }

        if (compareAllowed && findFolderInSet(set.roots, folder1) && findFolderInSet(set.roots, folder2))
        {
            // Add to allFiles, if not in yet
            bool found1 = false;  int  id1 = 0;
            bool found2 = false;  int  id2 = 0;

            for (int idx = 0; idx < allFiles.size(); ++idx)
            {
                if (allFiles[idx].filename == p.f1_filepath)
                {
                    found1 = true;
                    id1   = idx;
                }
                if (allFiles[idx].filename == p.f2_filepath)
                {
                    found2 = true;
                    id2   = idx;
                }
                if (found1 && found2) continue;
            }
            if (!found1)
            {
                FileEntry fe;
                fe.filename     = p.f1_filename;
                fe.lastModified = p.f1_lastmodified;
                fe.path         = p.f1_filepath;
                fe.size         = p.f1_filesize;
                fe.uuid         = key.u1;
                allFiles.append(fe);
                id1 = allFiles.size() - 1;
            }
            if (!found2)
            {
                FileEntry fe;
                fe.filename     = p.f2_filename;
                fe.lastModified = p.f2_lastmodified;
                fe.path         = p.f2_filepath;
                fe.size         = p.f2_filesize;
                fe.uuid         = key.u2;
                allFiles.append(fe);
                id2 = allFiles.size() - 1;
            }

            // Add to pairs
            CompareInfo ci;
            ci.i       = id1;
            ci.j       = id2;
            ci.manTime = p.manTime;

            ci.uuid1 = key.u1;
            ci.uuid2 = key.u2;
            ci.deleted = false;
            ci.manualNoDuplicate = true;
            pairs.append(ci);

            int b=1;
        }
        int a = 1;
    }

    // Liste anzeigen ---------------------------------------------------------------------------------------------------------------------------------
    ShowManualDuplicateList(true);

    scanActive = false;
    stopScan   = true;
    setButtonState(true, false);

}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::writeOrbAndPHashToDB(QSqlDatabase &db, FileEntry &entry)
{

    // serialisiere keypoints aus der entry Struktur
    QByteArray kpData = entry.keyPoints;

    QDataStream kpStream(&kpData, QIODevice::WriteOnly);

    QString folderPath = QFileInfo(entry.path).absolutePath();

    if (!entry.foundInDB) {
        QSqlQuery q(db);
        q.prepare(R"(
            INSERT INTO Files
            (OrigFilepath, OrigFilename, OrigFilesize, OrigLastModified,
                       Filepath, Filename, FilenameStripped, Filesize, LastModified, FirstKnownValidFolder)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )");

        q.addBindValue(entry.path);
        q.addBindValue(entry.filename);
        q.addBindValue(entry.size);
        q.addBindValue(entry.lastModified);

        q.addBindValue(entry.path);
        q.addBindValue(entry.filename);
        q.addBindValue(entry.filenameStripped);
        q.addBindValue(entry.size);
        q.addBindValue(entry.lastModified);

        q.addBindValue(folderPath);

        q.exec();
        entry.uuid = q.lastInsertId().toLongLong();
    }

    QSqlQuery q2(db);

    QByteArray cannyBlob(
        reinterpret_cast<const char*>(entry.canny.constData()),
        entry.canny.size() * sizeof(float)
        );


    if (!entry.foundInDB) {
        q2.prepare(R"(
            INSERT OR REPLACE INTO Features
            (file_id, KeyPoints, Descriptors, pHash, canny)
            VALUES (?, ?, ?, ?, ?)
        )");

        q2.addBindValue(entry.uuid);
        q2.addBindValue(kpData);
        q2.addBindValue(entry.descriptors);
        q2.addBindValue(static_cast<qlonglong>(entry.pHash));

        q2.addBindValue(cannyBlob);

    } else {
        q2.prepare(R"(
            UPDATE Features
            SET pHash       = ?,
                KeyPoints   = ?,
                Descriptors = ?,
                canny       = ?
            WHERE file_id = ?
        )");

        q2.addBindValue(static_cast<qlonglong>(entry.pHash));
        q2.addBindValue(entry.keyPoints);
        q2.addBindValue(entry.descriptors);
        q2.addBindValue(cannyBlob);
        q2.addBindValue(entry.uuid);
    }

    if (!q2.exec()) {
         qWarning() << "DB update failed:" << q2.lastError();
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::writeScoreToDB(QSqlDatabase &db, CompareInfo &info)
{
    qint64 uuid1 = allFiles[info.i].uuid;
    qint64 uuid2 = allFiles[info.j].uuid;

    if (uuid1 == uuid2) return;
    if (uuid1 >  uuid2) std::swap(uuid1, uuid2);

    QSqlQuery q(db);

    q.prepare(R"(
        INSERT OR REPLACE INTO Scores
        (file1_id, file2_id, Score)
        VALUES (?, ?, ?)
    )");

    q.addBindValue(uuid1);
    q.addBindValue(uuid2);
    q.addBindValue(info.score);

    q.exec();
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::readAllScoresFromDB(QSqlDatabase &db)
{
    QSqlQuery q(db);

    q.prepare(R"(
        SELECT file1_id, file2_id, Score, manualNoDuplicate
        FROM Scores
    )");


    if (q.exec())
        while (q.next())
        {
            qint64 uuid1 = q.value(0).toLongLong();
            qint64 uuid2 = q.value(1).toLongLong();
            ScoreInfo si;
            si.score             = q.value(2).toFloat();
            si.manualNoDuplicate = q.value(3).toBool();

            PairKey key { uuid1, uuid2 };

            allScoresInDB.insert(key, si);
        }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::readManualEntriesFromDB(QSqlDatabase &db)
{
    QSqlQuery q(db);

    q.prepare(R"(
        SELECT file1_id, file2_id, manNoDuplLastModified,
               f1.filepath, f1.filename, f1.filesize, f1.lastmodified,
               f2.filepath, f2.filename, f2.filesize, f2.lastmodified
        FROM Scores s
        JOIN Files f1 on f1.id = s.file1_id
        JOIN Files f2 on f2.id = s.file2_id
        WHERE s.manualNoDuplicate = 1;
    )");


    if (q.exec())
        while (q.next())
        {
            qint64 uuid1   = q.value(0).toLongLong();
            qint64 uuid2   = q.value(1).toLongLong();

            ManualInfo mi;
            mi.manTime         = q.value(2).toLongLong();

            mi.f1_filepath     = q.value(3).toString();
            mi.f1_filename     = q.value(4).toString();
            mi.f1_filesize     = q.value(5).toLongLong();
            mi.f1_lastmodified = q.value(6).toLongLong();

            mi.f2_filepath     = q.value(7).toString();
            mi.f2_filename     = q.value(8).toString();
            mi.f2_filesize     = q.value(9).toLongLong();
            mi.f2_lastmodified = q.value(10).toLongLong();

            PairKey key { uuid1, uuid2 };

            allManualsInDB.insert(key, mi);
        }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::readScoreFromDB
    (QSqlDatabase &db, CompareInfo &info, qsizetype maxRead)
{
    if (!allFiles[info.i].foundInDB || !allFiles[info.j].foundInDB)   // if one of the files were not found in DB, then there cannot be a score
        return;

    qint64 uuid1 = allFiles[info.i].uuid;
    qint64 uuid2 = allFiles[info.j].uuid;

    if (uuid1 == uuid2) return;
    if (uuid1 >  uuid2) std::swap(uuid1, uuid2);

//    QString key = makeKey(uuid1, uuid2);
    PairKey key { uuid1, uuid2 };

    if (pairIndex.contains(key)) {
/*
        const QVector<int>& indices = pairIndex[key];

        for (int idx : indices) {
            if (idx >= maxRead)
                break;  // wichtig: wir gehen von aufsteigenden Indizes aus

            auto& pd = pairs[idx];

            info.score             = pd.score;
            info.manualNoDuplicate = pd.manualNoDuplicate;
            info.calculationDone   = pd.calculationDone;
            info.foundInDB         = pd.foundInDB;

            return;
        }
*/
        int idx  = pairIndex[key];
        auto& pd = pairs[idx];

        info.score             = pd.score;
        info.manualNoDuplicate = pd.manualNoDuplicate;
        info.calculationDone   = pd.calculationDone;
        info.foundInDB         = pd.foundInDB;

        if (info.foundInDB)
            return;
    }


    QSqlQuery q(db);

    q.prepare(R"(
        SELECT Score, manualNoDuplicate
        FROM Scores
        WHERE file1_id = ? AND file2_id = ?
    )");

    q.addBindValue(uuid1);
    q.addBindValue(uuid2);

    if (q.exec() && q.next()) {
        info.score             = q.value(0).toFloat();
        info.manualNoDuplicate = q.value(1).toBool();
        info.foundInDB         = true;
        info.calculationDone   = true;
    }
}

// -------------------------------------------------------------------------------------------------------------
float MainWindow::compareORB(const QByteArray &d1, const QByteArray &d2)
{
    if (d1.isEmpty() || d2.isEmpty())
        return 0;

    cv::Mat des1(d1.size() / 32, 32, CV_8U);
    memcpy(des1.data, d1.constData(), d1.size());

    cv::Mat des2(d2.size() / 32, 32, CV_8U);
    memcpy(des2.data, d2.constData(), d2.size());

    cv::BFMatcher matcher(cv::NORM_HAMMING);

    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher.knnMatch(des1, des2, knnMatches, 2);

    int goodMatches = 0;

    for (const auto &matchPair : knnMatches)
    {
//        int c = matchPair.size();
        if (matchPair.size() < 2)
            continue;

        const cv::DMatch &m1 = matchPair[0];
        const cv::DMatch &m2 = matchPair[1];

        // Lowe Ratio Test
        if (m1.distance < 0.75f * m2.distance)
            goodMatches++;
    }

    return goodMatches/ (float)std::min(des1.rows, des2.rows);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::onUpdateProgress(int progress)
{
    progressCount += progress;
    progressBar->setValue(progressCount);  // Fortschritt anzeigen

    if (progressCount >= progressMax && nextAction != -1) {
        switch (nextAction) {
            case 0: {
                nextAction = -1;
                // xxx tun
                break;
            }

            case 1: {
                nextAction = -1;
                // die Dupl zusammenführen
                break;
            }

            default: break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::onProcessingFinished()
{
    qInfo() << "Verarbeitung abgeschlossen!";
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::onErrorOccurred(const QString& errorMessage)
{
    qDebug() << "Fehler: " << errorMessage;
    scanActive = false;
    setCombo->setEnabled(true);
}

// -------------------------------------------------------------------------------------------------------------
QString MainWindow::stripFilename(const QString &filename)
{
    // a file name prefix like "[...]"  is removed here:
    QRegularExpression re(R"(^\[[^\]]*\]\s*(.+)$)");
    QRegularExpressionMatch match = re.match(filename);

    if (match.hasMatch()) {
        return match.captured(1);
    }
    return filename;
}

// -------------------------------------------------------------------------------------------------------------
int  MainWindow::countAllPictures()
{
    const FolderSet& set = m_sets[m_currentSetIndex];
    int nbrImages = 0;
    for (const FolderNode& root : set.roots) {
        countAllPictures(root, nbrImages);
    }
    return nbrImages;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::countAllPictures(const FolderNode &node, int &nbrPictures)
{
    if (node.state != Excluded)
        nbrPictures += node.nbrPictures;

    for (const FolderNode &child : node.children) {
        countAllPictures(child, nbrPictures);
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::collectFromFolderNode(const FolderNode &node, QList<FileEntry> &out)
{
    // Dateien im aktuellen Ordner
    QDir dir(node.path);
    if (node.state != Excluded && dir.exists()) {

        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.tiff" << "*.tif" << "*.webp" << "*.avif";   // "*.gif" <<
        const QList<QByteArray> supported = QImageReader::supportedImageFormats();
        filters.removeIf([&supported](const QString &filter) {
            const QString extension = filter.mid(2);   // "*.xxx" -> "xxx"
            return !supported.contains(extension.toUtf8());
        });

        QFileInfoList files = dir.entryInfoList(
            filters,
            QDir::Files | QDir::NoDotAndDotDot
            );

        for (const QFileInfo &info : std::as_const(files))
        {
            FileEntry entry;
            entry.path             = info.absoluteFilePath();
            entry.filename         = info.fileName();
            entry.filenameStripped = stripFilename(entry.filename);
            entry.size             = info.size();
            entry.lastModified     = info.lastModified().toSecsSinceEpoch();
            entry.referenceImg     = (node.state == Reference);
            entry.imgCopy          = false;
            entry.parentPath       = "";

            out.append(entry);
        }
    }

    // 👉 Jetzt zu den Unterordnern gehen
    for (const FolderNode &child : node.children) {
        collectFromFolderNode(child, out);
    }
}

// -------------------------------------------------------------------------------------------------------------
bool MainWindow::tryMatchInDB(QSqlDatabase &db, FileEntry &entry)
{
    QSqlQuery query(db);

    auto updateFolderIfNeeded = [&](const qint64 &uniqueID, const QString &currentFolder)
    {
        QString dbFolder = query.value(7).toString();   // FirstKnownValidFolder

        if (dbFolder.isEmpty() || !QFile::exists(dbFolder + "/" + entry.filename)) {
            QSqlQuery q2(db);

            q2.prepare(R"(
                UPDATE Files
                SET FirstKnownValidFolder = ?, Filepath = ?
                WHERE id = ?
            )");

            q2.addBindValue(currentFolder);
            q2.addBindValue(currentFolder + "/" + entry.filename);
            q2.addBindValue(uniqueID);
            q2.exec();

        } else if (dbFolder != currentFolder) {
            entry.imgCopy = true;
            entry.parentPath = dbFolder + "/" + entry.filename;
        }
    };

    auto checkResult = [&](QSqlQuery &q) -> int {
        if (!q.next())
            return -2;   // no entry found

        qint64 dbSize = q.value(5).toLongLong();
        qint64 dbTime = q.value(6).toLongLong();

        if (dbSize == entry.size && dbTime == entry.lastModified) {
            entry.keyPoints   = q.value(1).toByteArray();
            entry.descriptors = q.value(2).toByteArray();
            entry.pHash       = static_cast<uint64_t>(q.value(3).toLongLong());

            QByteArray cannyBlob = q.value(4).toByteArray();

            if (cannyBlob.size() % sizeof(float) != 0) {
                qDebug() << "tryMatchInDB - checkResult: Ungültige canny BLOB-Größe:" << cannyBlob.size();
                return 2;  // Simulating -> no entry found
            }

            const int count = cannyBlob.size() / sizeof(float);
            entry.canny.resize(count);
            if (count > 0) {
                memcpy(entry.canny.data(),
                       cannyBlob.constData(),
                       cannyBlob.size());
            }

            entry.foundInDB   = true;
            return 1;  // found
        }
        return -1;   // entry found but no match with size and lastModified
    };


    // Eintrag suchen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    query.prepare(R"(
        SELECT fi.id,
           f.KeyPoints, f.Descriptors, f.pHash, f.canny, fi.Filesize, fi.LastModified, fi.FirstKnownValidFolder,
           fi.Filename, fi.OrigFilename, fi.FilenameStripped
        FROM Features f
        JOIN Files fi ON fi.id = f.file_id
        WHERE (fi.Filepath = ?)
    )");
    query.addBindValue(entry.path);

    if (query.exec()) {
        int result = checkResult(query);
        while (result != -2){  // noch zeilen vorhanden
            if (result == 1) {
                QString currentFolder = QFileInfo(entry.path).absolutePath();
                qint64  uniqueID = query.value(0).toLongLong();

                entry.uuid       = uniqueID;
                entry.foundLevel = 1;

                updateFolderIfNeeded(uniqueID, currentFolder);

                return true;
            }
            result = checkResult(query);
        }
    } else
        return false;  // DB error -> better to stop here

    // 2. Filename - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    query.prepare(R"(
        SELECT fi.id,
           KeyPoints, Descriptors, f.pHash, f.canny, Filesize, LastModified, fi.FirstKnownValidFolder
        FROM Features f
        JOIN Files fi ON fi.id = f.file_id
        WHERE fi.Filename = ?
    )");
    query.addBindValue(entry.filename);

    if (query.exec()) {
        int result = checkResult(query);
        while (result != -2){  // noch zeilen vorhanden
            if (result == 1) {
                QString currentFolder = QFileInfo(entry.path).absolutePath();
                qint64 uniqueID = query.value(0).toLongLong();

                entry.uuid       = uniqueID;
                entry.foundLevel = 2;

                updateFolderIfNeeded(uniqueID, currentFolder);

                return true;
            }
            result = checkResult(query);
        }
    } else
        return false;  // DB error -> better to stop here

    // 3. stripped filename - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    query.prepare(R"(
        SELECT fi.id,
           KeyPoints, Descriptors, f.pHash, f.canny, Filesize, LastModified, fi.FirstKnownValidFolder
        FROM Features f
        JOIN Files fi ON fi.id = f.file_id
        WHERE fi.FilenameStripped = ?
    )");
    query.addBindValue(entry.filenameStripped);

    if (query.exec()) {
        int result = checkResult(query);
        while (result != -2){  // noch zeilen vorhanden
            if (result == 1) {
                QString currentFolder = QFileInfo(entry.path).absolutePath();
                qint64 uniqueID = query.value(0).toLongLong();

                entry.uuid       = uniqueID;
                entry.foundLevel = 3;

                updateFolderIfNeeded(uniqueID, currentFolder);

                return true;
            }
            result = checkResult(query);
        }
        return false;
    }
    return false;
}

// -------------------------------------------------------------------------------------------------------------
cv::Mat MainWindow::descriptorsFromByteArray(const QByteArray &data)
{
    if (data.isEmpty())
        return cv::Mat();

    cv::Mat mat(1, data.size(), CV_8U);
    memcpy(mat.data, data.constData(), data.size());
    return mat;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::onProgressUpdated(int progress)
{
    qInfo() << "onProgressUpdated getriggert";
    progressBar->setValue(progress);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::deleteImage(int clusterIndex, int itemIndex, int imageBoxType)
{
    // ---------------------------------------------------- //
    // NUR ZUM TESTEN:                                      //
    // emit deleteResult(clusterIndex, itemIndex, true );   //
    // return;                                              //
    // ---------------------------------------------------- //

    QString name;
    QString path;

    switch (imageBoxType)
    {case 0:
    {

        if (clusterIndex < 0 || clusterIndex >= imageDupl.size() || m_duplicateView == nullptr)
            return;

        DuplicateCluster root = imageDupl[clusterIndex];

        if (itemIndex < 0 || itemIndex >= root.items.size())
            return;

        DuplicateItem item = root.items[itemIndex];
                name       = item.info.filename;
                path       = item.info.path;

        break;
    }
    case 1:
    {
        return;   //  <- TEMPORÄR NICHTS TUN
        break;
    }
    default:
        return;
    }

    if (!QFileInfo::exists(path))
    {
        QMessageBox::warning(m_duplicateView, qtTrId("mainwindow.output.error"),
                                              qtTrId("mainwindow.output.error.fileNotExists").arg(path));
        return;
    }

    QMessageBox::StandardButton reply =
        QMessageBox::question(m_duplicateView,   // old: this
                              qtTrId("mainwindow.output.deleteConfirm"),
                              qtTrId("mainwindow.output.deleteConfirm.delFile").arg(name),
                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    if (!QFile::moveToTrash(path))
    {
        QMessageBox::warning(m_duplicateView,
                             qtTrId("mainwindow.output.error"),
                             qtTrId("mainwindow.output.error.fileNotDeleted").arg(name));
        return;
    }


    //  Bild in allFiles finden:
    int removedIndex = -1;
    for (int k = 0; k < allFiles.size(); ++k)
    {   if (allFiles[k].path == path)
        {   removedIndex = k;
            break;
        }
    }
    if (removedIndex == -1)
        return;

/*  DOCH NICHT in pairs löschen: Wenn in pairs gelöscht, verschiebt sich pairs gegenüber UI
    // Diesen Index in den pairs suchen und entsprechendes "pair" köschen
    pairs.erase(std::remove_if(pairs.begin(), pairs.end(),
                               [removedIndex](const CompareInfo &p)
                               {
                                   return (p.i == removedIndex || p.j == removedIndex);
                               }),
                pairs.end());
*/
    // Only mark pairs entries as 'deleted', if the image is used in this pair
    for (auto &p : pairs) {
        if (p.i == removedIndex || p.j == removedIndex) {
            p.deleted = true;
        }
    }

    // Image Datei in Database löschen
    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        return;
    }

    if (!db.transaction()) {
        qWarning() << "DB transaction failed to start";
        return;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM Files WHERE id = ?");
    query.addBindValue(allFiles[removedIndex].uuid);

    if (!query.exec())
    {
        qWarning() << "DB delete failed:" << query.lastError();
        db.rollback();
        return;
    }

    if (!db.commit())
    {
        qWarning() << "DB commit failed:" << db.lastError();
        db.rollback();
    }

    // Gelöscht-Meldung zurückgeben
    emit deleteResult(clusterIndex, itemIndex, true );
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::makeMaster(int clusterIndex, int itemIndex)
{
    QVector<int>   itemIndices;
    QVector<float> newScores;
    QVector<int>   newDistances;
    QVector<float> newDiff;

    if (clusterIndex < 0 || clusterIndex >= imageDupl.size())
        return;

    DuplicateCluster root = imageDupl[clusterIndex];

    if (itemIndex < 0 || itemIndex >= root.items.size())
        return;

    DuplicateItem item = root.items[itemIndex];


    //  ehem. Root-Bild in allFiles finden:
    QString oldRootPath = root.root.path;
    int rootIdx = -1;
    for (int k = 0; k < allFiles.size(); ++k)
    {   if (allFiles[k].path == oldRootPath)
        {   rootIdx = k;
            break;
        }
    }
    if (rootIdx == -1)
        return;

    //  "Items"-Bilder in allFiles finden:
    for (auto &o : root.items) {
        QString oldItemPath = o.info.path;
        int oldItemIdx = -1;
        for (int k = 0; k < allFiles.size(); ++k)
        {   if (allFiles[k].path == oldItemPath)
            {   oldItemIdx = k;
                break;
            }
        }
        if (oldItemIdx == -1)
            return;

        itemIndices.append(oldItemIdx);
    }

    std::swap(imageDupl[clusterIndex].root, imageDupl[clusterIndex].items[itemIndex].info);
    std::swap(rootIdx, itemIndices[itemIndex]);

    // Via pairs den neuen Score ermitteln
    for (int o=0; o < imageDupl[clusterIndex].items.size(); ++o) {
        float score;
        for (auto &p : pairs) {
            if ( (p.i == rootIdx && p.j == itemIndices[o]) ||
                 (p.j == rootIdx && p.i == itemIndices[o])   ) {
                score = p.score;
                imageDupl[clusterIndex].items[o].score = score;
                newScores.append(score);
                newDistances.append(p.pHashDistance);
                newDiff.append(p.cannyDiff);
                break;
            }
        }
    }

    emit makeMasterResult(clusterIndex, itemIndex, newScores, newDistances, newDiff);
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::noDuplicates(int clusterIndex, int itemIndex, int imageBoxType)
{
    int rootIdx  = -1;
    int itemIdx  = -1;
    int rootUUID = -1;
    int itemUUID = -1;
    float currentScore = -2.0;

    switch (imageBoxType)
    {case 0:
    {
        if (clusterIndex < 0 || clusterIndex >= imageDupl.size())
            return;

        DuplicateCluster root = imageDupl[clusterIndex];

        if (itemIndex < 0 || itemIndex >= root.items.size())
            return;

        DuplicateItem item = root.items[itemIndex];


        //  Root-Bild in allFiles finden:
        QString rootPath = root.root.path;
        for (int k = 0; k < allFiles.size(); ++k)
        {   if (allFiles[k].path == rootPath)
            {   rootIdx = k;
                break;
            }
        }
        if (rootIdx == -1)
            return;

        //  "Item"-Bild in allFiles finden:
        QString itemPath = root.items[itemIndex].info.path;
        for (int k = 0; k < allFiles.size(); ++k)
        {   if (allFiles[k].path == itemPath)
            {   itemIdx = k;
                break;
            }
        }
        if (itemIdx == -1)
            return;

        QMessageBox::StandardButton reply =
            QMessageBox::question(m_duplicateView,
                                  qtTrId("mainwindow.output.noDuplConfirm"),
                                  qtTrId("mainwindow.output.noDuplConfirm.duplicates")
                                      .arg(allFiles[rootIdx].filename,
                                           allFiles[itemIdx].filename),
                                  QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes)
            return;

        // Das zugehörige pair finden
        for (auto &p : pairs) {
            if ( (p.i == rootIdx && p.j == itemIdx) ||
                (p.j == rootIdx && p.i == itemIdx)   ) {
                currentScore        = p.score;
                p.manualNoDuplicate = true;
                p.orbNeeded         = orbStatus::noDuplicate;
                break;
            }
        }

        if (currentScore == -2.0) {
            qWarning() << "noDuplicates: Pair not found:";
            return;
        }
    break;
    }
    case 1:
    {
        if (clusterIndex < 0 || clusterIndex >= imageDupl.size())
            return;

        DuplicateCluster root = imageDupl[clusterIndex];

        if (itemIndex != 0 || itemIndex >= root.items.size())
            return;

        DuplicateItem item = root.items[itemIndex];

        //  Root-Bild in allFiles finden:
        QString rootPath = root.root.path;
        for (int k = 0; k < allFiles.size(); ++k)
        {   if (allFiles[k].path == rootPath)
            {   rootIdx  = k;
                rootUUID = allFiles[k].uuid;
                break;
            }
        }
        if (rootIdx == -1)
            return;

        QString itemPath = root.items[itemIndex].info.path;
        for (int k = 0; k < allFiles.size(); ++k)
        {   if (allFiles[k].path == itemPath)
            {   itemIdx  = k;
                itemUUID = allFiles[k].uuid;
                break;
            }
        }
        if (itemIdx == -1)
            return;

        QMessageBox::StandardButton reply =
            QMessageBox::question(m_duplicateView,
                                  qtTrId("mainwindow.output.noManNoDuplConfirm"),
                                  qtTrId("mainwindow.output.noManNoDuplConfirm.duplicates")
                                      .arg(allFiles[rootIdx].filename,
                                           allFiles[itemIdx].filename),
                                  QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes)
            return;

        // Das zugehörige pair finden
        for (auto &p : pairs) {
            if ( (p.uuid1 == rootUUID && p.uuid2 == itemUUID) ||
                 (p.uuid2 == rootUUID && p.uuid1 == itemUUID)   ) {
                currentScore        = p.score;
                p.manualNoDuplicate = false;
                // p.orbNeeded         = orbStatus::Duplicate;
                break;
            }
        }

        currentScore = -1.0;  // <- this means, remove row in table if score in DB is 0.0
        break;
    }
    default:
        return;
    }

    // Update DB (Scores)
    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "noDuplicates: DB is not open!";
        return;
    }

    qint64 uuid1 = allFiles[rootIdx].uuid;
    qint64 uuid2 = allFiles[itemIdx].uuid;
    if (uuid1 == uuid2) return;
    if (uuid1 >  uuid2) std::swap(uuid1, uuid2);

    QSqlQuery query(db);
    switch (imageBoxType)
    {
    case 0:
    {
        query.prepare(R"(
            INSERT OR REPLACE INTO Scores
            (file1_id, file2_id, Score, manualNoDuplicate, manNoDuplLastModified)
            VALUES (?, ?, ?, ?, ?)
        )");

        qint64 timestamp = QDateTime::currentSecsSinceEpoch();

        query.addBindValue(uuid1);
        query.addBindValue(uuid2);
        query.addBindValue(currentScore);
        query.addBindValue(1);
        query.addBindValue(timestamp);

        if (!query.exec())
        {
            qWarning() << "DB update of manualNoDuplicate failed: (set manualNoDuplicate)" << query.lastError();
            return;
        }

        emit noDuplicatesResult(clusterIndex, itemIndex);
        break;
    }

    case 1:
    {
        query.prepare(R"(
            DELETE FROM Scores
            WHERE file1_id = ? AND file2_id = ? AND Score = ? AND  manualNoDuplicate = ?
        )");

        query.addBindValue(uuid1);
        query.addBindValue(uuid2);
        query.addBindValue(0.0);
        query.addBindValue(1);

        if (!query.exec())
        {
            qWarning() << "DB delete of manualNoDuplicate row failed:" << query.lastError();
            return;
        }

        if (query.numRowsAffected() == 0)
        {   // DELETE war zwar erfolgreich, aber keine passende Zeile gefunden
            QSqlQuery resetQuery(db);
            resetQuery.prepare(R"(
                UPDATE Scores
                SET manualNoDuplicate = 0
                WHERE file1_id = ? AND file2_id = ?
            )");

            resetQuery.addBindValue(uuid1);
            resetQuery.addBindValue(uuid2);

            if (!resetQuery.exec())
            {
                qWarning() << "DB update of manualNoDuplicate failed (reset manualNoDuplicate):" << resetQuery.lastError();
                return;
            }
        }

        emit noDuplicatesResult(clusterIndex, itemIndex);
        break;
    }
    default:
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setButtonState(bool newState, bool all)
{
    if (all)
    {
        setCombo->setEnabled(newState);
        btnNewSet->setEnabled(newState);
        btnCloneSet->setEnabled(newState);
        btnDeleteSet->setEnabled(newState);

        btnAddFolder->setEnabled(newState);
        btnRemoveFolder->setEnabled(newState);

        btnFindDuplicates->setEnabled(newState);
        btnCleanDB->setEnabled(newState);
        btnManual->setEnabled(newState);
        btnLanguage->setEnabled(newState);

        cbHandleSameDir->setEnabled(newState);

        sliderScoreLimit->setEnabled(newState);
        sliderDistLower->setEnabled(newState);
        sliderDistUpper->setEnabled(newState);
        sliderCannyLower->setEnabled(newState);
        sliderCannyUpper->setEnabled(newState);
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::cleanDB()
{
    setButtonState(false, true);

    long pageSize  = 0;
    long pageCount = 0;
    long freeCount = 0;
    long usedSpace;
    long freeSpace;
    bool vacuum = false;

    setButtonState(false, true);

    auto reply = QMessageBox::question(this, qtTrId("mainwindow.output.confirm"),
                                             qtTrId("mainwindow.output.confirm.cleanDB"));
    if (reply != QMessageBox::Yes) {
        setButtonState(true, true);
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("PictureDBConnection");
    if (!db.isOpen()) {
        qDebug() << "DB is not open!";
        setButtonState(true, true);
        return;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    {   QSqlQuery q(db);
        QVector<qint64> uuidToDelete1;

        // UUIDs der Files sammeln, die nicht mehr existieren  - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if (!q.exec(R"(
                SELECT fi.id, fi.Filename, fi.Filepath, fi.Filesize, fi.LastModified
                FROM Files fi
            )"))
        {
            QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                       qtTrId("mainwindow.output.error.readTableFiles").arg(q.lastError().text()));
            setButtonState(true, true);
            return;
        }

        while (q.next()){
            qint64  uuid     = q.value("id").toLongLong();
            QString filename = q.value("Filename").toString();
            QString filepath = q.value("Filepath").toString();
            qint64  filesize = q.value("Filesize").toLongLong();
            qint64  filedate = q.value("LastModified").toLongLong();

            QFileInfo filedata = QFileInfo(filepath);;
            if (!filedata.exists() || filedata.size() != filesize || filedata.lastModified().toSecsSinceEpoch() != filedate) {
                uuidToDelete1.append(uuid);
            }
        }

        if (uuidToDelete1.size() > 0)
        {
            int count = 0;
            int step  = std::max(1, (int)uuidToDelete1.size() / 50);
            prepareProgress(qtTrId("mainwindow.label.progressTask.removeDBentries"), uuidToDelete1.size(), "green", "lightgreen");

            QMessageBox::information(this, qtTrId("mainwindow.output.info"),
                                           qtTrId("mainwindow.output.info.DBentriesFiles").arg(uuidToDelete1.size()));

            // Diese gefundenen UUIDs aus der DB löschen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            if (!db.transaction()) {
                qDebug() << "Could not start transaction";
                QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                           qtTrId("mainwindow.output.error.DBTransaction"));
                setButtonState(true, true);
                return;
            }
            q.prepare("DELETE FROM Files WHERE id = :id");

            for (const qint64 &id : uuidToDelete1) {
                count++;
                if ((count % step) == 0) {
                    progressBar->setValue(count);
                    QCoreApplication::processEvents();
                }

                q.bindValue(":id", id);

                if (!q.exec()) {
                    qDebug() << "Delete failed for" << id << ":" << q.lastError().text();
                    QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                               qtTrId("mainwindow.output.error.deleteID")
                                                     .arg(id)
                                                     .arg(q.lastError().text()));
                    db.rollback();
                    setButtonState(true, true);
                    return;
                }
            }
            db.commit();
        }
    }


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    {   QSqlQuery q(db);
        QVector<qint64> uuidToDelete1;

        // Ids sammeln von Eintrags-Duplikate (identischer filepath) - - - - - - - - - - - - - - - - - - - - - -
        if (!q.exec(R"(
            select id from files f1 where exists (
                select 1  from files f2 where f1.filepath = f2.filepath and f1.id > f2.id );
        )")) {
            QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                       qtTrId("mainwindow.output.error.readTableFiles").arg(q.lastError().text()));
            setButtonState(true, true);
            return;
        }

        while (q.next()){
            qint64  uuid = q.value("id").toLongLong();
            uuidToDelete1.append(uuid);
        }

        if (uuidToDelete1.size() > 0) {
            int count = 0;
            int step  = std::max(1, (int)uuidToDelete1.size() / 50);
            prepareProgress(qtTrId("mainwindow.label.progressTask.removeDBduplicates"), uuidToDelete1.size(), "green", "lightgreen");

            QMessageBox::information(this, qtTrId("mainwindow.output.info"),
                                           qtTrId("mainwindow.output.info.DBentryDuplFiles").arg(uuidToDelete1.size()));

            // Diese gefundenen UUIDs aus der DB löschen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            if (!db.transaction()) {
                qDebug() << "Could not start transaction";
                QMessageBox::warning(this, qtTrId("mainwindow.output.error"), qtTrId("mainwindow.output.error.DBTransaction"));
                setButtonState(true, true);
                return;
            }
            q.prepare("DELETE FROM Files WHERE id = :id");

            for (const qint64 &id : uuidToDelete1) {
                count++;
                if ((count % step) == 0) {
                    progressBar->setValue(count);
                    QCoreApplication::processEvents();
                }

                q.bindValue(":id", id);

                if (!q.exec()) {
                    qDebug() << "Delete failed for" << id << ":" << q.lastError().text();
                    QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                               qtTrId("mainwindow.output.error.deleteID")
                                                    .arg(id)
                                                    .arg(q.lastError().text()));
                    db.rollback();
                    setButtonState(true, true);
                    return;
                }
            }
            db.commit();
        }
    }

    // UUIDs Paare in Features finden, wo die UUID nicht mehr in Files existiert - - - - - - - - - - - - - - - - -
    {   QSqlQuery q(db);
        QVector<qint64> uuidToDelete1;

        if (!q.exec(R"(
            SELECT f.file_id
            FROM Features f
            LEFT JOIN Files fi ON fi.id = f.file_id
            WHERE fi.id IS NULL;
        )")) {
            QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                       qtTrId("mainwindow.output.error.readTableFeatures").arg(q.lastError().text()));
            setButtonState(true, true);
            return;
        }

        while (q.next())
        {
            qint64 uuid = q.value("file_id").toLongLong();
            uuidToDelete1.append(uuid);
        }

        if (uuidToDelete1.size() > 0) {
            QMessageBox::information(this, "Info",
                                     QString("%1 'Features' Einträge, bei denen die UUIDs nicht in 'Files' enthalten sind.\nDiese werden jetzt gelöscht").arg(uuidToDelete1.size()));
        }
    }

    // UUIDs Paare in Scores finden, wo mindestens eine UUID nicht mehr in Files existiert - - - - - - - - - - -
    {   QSqlQuery q(db);
        QVector<qint64> uuidToDelete1;
        QVector<qint64> uuidToDelete2;

        if (!q.exec(R"(
            SELECT s.*
            FROM Scores s
            LEFT JOIN Files f1 ON f1.id = s.file1_id
            LEFT JOIN Files f2 ON f2.id = s.file2_id
            WHERE f1.id IS NULL
               OR f2.id IS NULL;
        )")) {
            QMessageBox::warning(this, qtTrId("mainwindow.output.error"),
                                       qtTrId("mainwindow.output.error.readTableScores").arg(q.lastError().text()));
            setButtonState(true, true);
            return;
        }

        while (q.next()){
            qint64 uuid1 = q.value("file1_id").toLongLong();
            qint64 uuid2 = q.value("file2_id").toLongLong();

            uuidToDelete1.append(uuid1);
            uuidToDelete2.append(uuid2);
        }

        if (uuidToDelete1.size() > 0) {
            QMessageBox::information(this, "Info",
                                     QString("%1 'Scores' Einträge, bei denen die IDs nicht in 'Files' enthalten sind.\nDiese werden jetzt gelöscht").arg(uuidToDelete1.size()));

        }
    }

    // Nun den freien Platz hearusfinden und nach 'Vacuum' nachfragen - - - - - - - - - - - - - - - - - - - - -
    {   QSqlQuery check(db);

        // Get page_size
        check.exec("PRAGMA page_size;");
        if (check.next()) {
            pageSize = check.value(0).toLongLong();
        } else {
            setButtonState(true, true);
            return;
        }

        // Get page_count
        check.exec("PRAGMA page_count;");
        if (check.next()) {
            pageCount = check.value(0).toLongLong();
        } else {
            setButtonState(true, true);
            return;
        }

        // Get freelist_count
        check.exec("PRAGMA freelist_count;");
        if (check.next()) {
            freeCount = check.value(0).toLongLong();
        } else {
            setButtonState(true, true);
            return;
        }

        usedSpace = pageSize * pageCount;
        freeSpace = pageSize * freeCount;

        QString usedSpaceOutput = QLocale().formattedDataSize(usedSpace);
        QString freeSpaceOutput = QLocale().formattedDataSize(freeSpace);

        QMessageBox msgBox;
        msgBox.setWindowTitle(qtTrId("mainwindow.output.question"));
        msgBox.setText(qtTrId("mainwindow.output.question.VacuumUsedSpace")
                           .arg(usedSpaceOutput, freeSpaceOutput)
                       );

        QPushButton *btnYes    = msgBox.addButton(qtTrId("mainwindow.output.question.optionShrinkYes"), QMessageBox::YesRole);
        QPushButton *btnNo     = msgBox.addButton(qtTrId("mainwindow.output.question.optionShrinkNo") , QMessageBox::NoRole);
        // QPushButton *btnCancel = msgBox.addButton("Abbrechen"    , QMessageBox::RejectRole);

        msgBox.exec();

        // if (msgBox.clickedButton() == btnCancel)
        //    return;

        if (msgBox.clickedButton() == btnYes)
            vacuum = true;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if (vacuum)
    {
        prepareProgress(qtTrId("mainwindow.label.progressTask.shrinkDBfile"), 1, "darkviolet", "violet");

        QSqlQuery q(db);

        if (!q.exec("VACUUM")) {
            QMessageBox::information(this, qtTrId("mainwindow.output.error"),
                                           qtTrId("mainwindow.output.error.vacuum").arg(q.lastError().text()));

            setButtonState(true, true);
            return;
        }
    }

    prepareProgress(qtTrId("mainwindow.label.progressTask.Idle"), -1, "black", "black");
    setButtonState(true, true);
}

// -------------------------------------------------------------------------------------------------------------
uint64_t MainWindow::computePHash(const cv::Mat& image)
{
    if (image.empty())
        return 0;

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(32, 32));
    resized.convertTo(resized, CV_32F);

    // DCT (Discrete Cosine Transform)
    cv::Mat dct;
    cv::dct(resized, dct);

    // 8x8 Top-Left (low frequencies)
    cv::Mat dctLow = dct(cv::Rect(0, 0, 8, 8));

    // Mittelwert (ohne DC-Komponente optional)
    // double mean = cv::mean(dctLow)[0];

    double sum = 0.0;
    for (int i = 0; i < 64; ++i)
    {
        int row = i / 8;
        int col = i % 8;

        if (row == 0 && col == 0) continue; // DC überspringen

        sum += dctLow.at<float>(row, col);
    }
    double mean = sum / 63.0;


    // Hash generieren
    uint64_t hash = 0;
    for (int i = 0; i < 64; ++i)
    {
        int row = i / 8;
        int col = i % 8;

        if (row == 0 && col == 0) continue; // DC überspringen

        if (dctLow.at<float>(row, col) > mean)
            hash |= (1ULL << i);
    }

    return hash;
}

// -------------------------------------------------------------------------------------------------------------
int MainWindow::hammingDistance(uint64_t a, uint64_t b)
{
    return __builtin_popcountll(a ^ b); // GCC/Clang
}

// -------------------------------------------------------------------------------------------------------------
QVector<float> MainWindow::computeCanny(const cv::Mat& image)
{
    QVector<float> features;

    if (image.empty())
        return features;

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(128, 128));
    // resized.convertTo(resized, CV_32F);

    cv::Mat edges;
    cv::Canny(resized, edges, 50, 150);


    // Grid definieren
    const int GRID = 8;  // Immer eine Zahl, durch die die Größe oben geteilt werden kann

    int cellW = edges.cols / GRID;
    int cellH = edges.rows / GRID;

    // Feature Vektor bauen
    features.reserve(GRID * GRID);

    // pro Zelle Kanten zählen
    for (int y = 0; y < GRID; y++)
    {
        for (int x = 0; x < GRID; x++)
        {
            cv::Rect cell(x * cellW, y * cellH, cellW, cellH);
            cv::Mat roi = edges(cell);

            int count = cv::countNonZero(roi);
            float ratio = (float)count / (cellW * cellH);

            features.push_back(ratio);
        }
    }

    return features;
}

// -------------------------------------------------------------------------------------------------------------
float MainWindow::compareCanny(const QVector<float> &pic1, const QVector<float> &pic2)
{
    float dist = 0;
    int minSize = std::min(pic1.size(), pic2.size());

    if (minSize > 0)
    {
        for (int i = 0; i < minSize; i++)
            dist += fabs(pic1[i] - pic2[i]);

        return dist / minSize;
    }
    else
        return 1.0;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::writeToCsv(QVector<DuplicateImage> &duplImages)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(path);
    dir.cdUp();
    QString configRoot = dir.path() + "/DuplicateFinder";

    QFile file(configRoot + "/daten.csv");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        out.setLocale(QLocale::C);

        // Kopfzeile
        out << "Bild1;Bild2;pHash;canny;ORB\n";

        for (auto &d : duplImages) {
            for (int c=0; c < d.duplIndices.size(); ++c) {
                out << QString("\"%1\";\"%2\";%3;%4;%5\n")
                           .arg(d.filename,
                                allFiles[d.duplallFilesIndices[c]].filename,
                                QString::number(d.duplHashDistances[c]),
                                QString::number(d.duplCannyDiff[c], 'f', 4),
                                QString::number(d.duplScores[c]   , 'f', 4));
            }
        }

        file.close();
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::setLanguage(const QString &locale)
{
    qApp->removeTranslator(&m_translator);
    qApp->removeTranslator(&m_qtTranslator);

    const QString baseName = "Picture_Duplicates_" + locale;
    //    const QString path = qApp->applicationDirPath() + "/translations";
    const QString path = TRANSLATION_DIR;  // <<<==== Defined in CMakeLists.txt  <<<====


    const QStringList candidates = {
        locale,
        QLocale(locale).name(),   // Fallback to System-Language
        "en_US"                   // Fallback to en_US - always part of this programm
    };

    qInfo() << "Open Language File: " << path << baseName;

    for (const QString &lang : candidates)
    {
        const QString baseName = "Picture_Duplicates_" + lang;

        if (m_translator.load(baseName, path)) {
            bool loaded = qApp->installTranslator(&m_translator);

            qInfo() << "installTranslator : " << loaded;
            qInfo() << "Translation loaded: " << baseName;

            qInfo() << "qrTrId   :" << qtTrId("mainwindow.table.colFolder");
            qInfo() << "tr       :" << tr("mainwindow.table.colFolder");
            qInfo() << "translate:" << qApp->translate("", "mainwindow.table.colFolder");
            qInfo() << "normal   :" << QCoreApplication::translate("", "mainwindow.table.colFolder");

            break;
        }
        else
            qWarning() << "Translation NOT loaded";
    }

    // Qt Language Update
    for (const QString &lang : candidates)
    {
        const QString baseName = "qtbase_" + lang.split('_').first();

        if (m_qtTranslator.load(baseName, QLibraryInfo::path( QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&m_qtTranslator);

            qInfo() << "Translation loaded: " << baseName;
            return;
        }
    }

    qDebug() << "No translation loaded, even fallback failed.";
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        // ui->retranslateUi(this);
        retranslateUi();
        rebuildTree();
    }
    QMainWindow::changeEvent(event);
}

// -------------------------------------------------------------------------------------------------------------
QStringList MainWindow::availableLanguages() const
{
    QStringList result;

    // QDir dir(qApp->applicationDirPath());
    QDir dir(TRANSLATION_DIR);  // <<<==== Defined in CMakeLists.txt  <<<====

    qInfo() << "availableLanguages: dir.path(): " << dir.path();

    const QFileInfoList files = dir.entryInfoList(
        QStringList() << "Picture_Duplicates_*.qm",
        QDir::Files
        );

    qInfo() << "availableLanguages: files.size(): " << files.size();

    for (const QFileInfo &file : files) {
        QString name = file.baseName();
        name.remove("Picture_Duplicates_");
        result << name;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::showLanguageDialog()
{
    QSettings settings("DuplicateFinder", "DuplicateFinder");

    QString currentLanguage = settings.value("General/Language", QLocale::system().name()).toString();

    struct LangItem {
        QString code;
        QString name;
    };


    // List of all available languages
    QVector<LangItem> items;
    for (const QString &lang : availableLanguages()) {
        QLocale locale(lang);

        items.push_back({
            lang,
            locale.nativeLanguageName()
        });
    }

    // Sort language list:
    std::sort(items.begin(), items.end(),
              [](const LangItem &a, const LangItem &b) {
                  return QString::localeAwareCompare(a.name, b.name) < 0;
              });


    // Create dialog:
    QDialog dialog(this);
    dialog.setWindowTitle(qtTrId("mainwindow.language.title"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *list   = new QListWidget(&dialog);


    for (const LangItem &item : items) {
        auto *listItem = new QListWidgetItem(item.name, list);
        listItem->setData(Qt::UserRole, item.code);
    }

    // Select current entry:
    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *it = list->item(i);

        if (it->data(Qt::UserRole).toString() == currentLanguage) {
            list->setCurrentItem(it);
            break;
        }
    }

    layout->addWidget(list);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        &dialog);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && list->currentItem()) {
        const QString newLanguage = list->currentItem()->data(Qt::UserRole).toString();
        settings.setValue("General/Language", newLanguage);
        setLanguage(newLanguage);
    }
}

// -------------------------------------------------------------------------------------------------------------
void MainWindow::retranslateUi()
{

    QString lang = m_translator.language();
    QString countryCode = lang.section('_', 1, 1).toLower();
    QString flag;
    for (qsizetype i = 0; i < countryCode.size(); ++i)
    {
        const QChar c = countryCode.at(i);
        const char32_t codePoint = 0x1F1E6 + (c.toUpper().unicode() - 'A');
        flag += QString::fromUcs4(&codePoint, 1);
    }

    this->setWindowTitle(qtTrId("mainwindow.title"));

    if (btnNewSet != nullptr)
    {
        btnNewSet->setText(qtTrId("mainwindow.button.newSet"));
        btnCloneSet->setText(qtTrId("mainwindow.button.duplSet"));
        btnDeleteSet->setText(qtTrId("mainwindow.button.delSet"));
        btnAddFolder->setText(qtTrId("mainwindow.button.addFolder"));
        btnRemoveFolder->setText(qtTrId("mainwindow.button.remFolder"));
        btnFindDuplicates->setText(qtTrId("mainwindow.button.findDupl"));
        SliderLbl1->setText(qtTrId("mainwindow.slider.ORB.Title"));
        SliderLbl2->setText(qtTrId("mainwindow.slider.pHashLower.Title"));
        SliderLbl3->setText(qtTrId("mainwindow.slider.pHashHigher.Title"));
        SliderLbl4->setText(qtTrId("mainwindow.slider.cannyLower.Title"));
        SliderLbl5->setText(qtTrId("mainwindow.slider.cannyHigher.Title"));
        lblSameDir->setText(qtTrId("mainwindow.combobox.lblMngSameDir"));
        cbHandleSameDir->setItemText(0, qtTrId("mainwindow.combobox.valOff"));
        cbHandleSameDir->setItemText(1, qtTrId("mainwindow.combobox.valExcl"));
        cbHandleSameDir->setItemText(2, qtTrId("mainwindow.combobox.valOnly"));
        btnApply->setText(qtTrId("mainwindow.button.apply"));
        btnReset->setText(qtTrId("mainwindow.button.reset"));
        btnManual->setText(qtTrId("mainwindow.button.showManual"));
        btnCleanDB->setText(qtTrId("mainwindow.button.cleanDB"));
        btnLanguage->setText(qtTrId("mainwindow.button.language") + "      " + flag);
        progressLabel->setText(qtTrId("mainwindow.label.progressTask.Idle"));

        folderModel->setHeaderData(0, Qt::Horizontal, qtTrId("mainwindow.table.colFolder"));
        QString tmp = qtTrId("mainwindow.table.colImages.withNumber").arg(QLocale().toString(countAllPictures()));
        folderModel->setHeaderData(1, Qt::Horizontal, tmp);
        folderModel->setHeaderData(2, Qt::Horizontal, qtTrId("mainwindow.table.colStatus"));
        folderModel->setHeaderData(3, Qt::Horizontal, qtTrId("mainwindow.table.colExclDir"));

    }
}

// -------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------
