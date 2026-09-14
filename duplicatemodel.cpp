#include "duplicatemodel.h"

#include <QDateTime>
#include <QEvent>
#include <QFileInfo>
#include <QLocale>
#include <QSettings>
#include <QTimer>

#include <QLabel>
#include <QScrollArea>
#include <QStyleFactory>
#include <QVBoxLayout>

#include <QDesktopServices>


DuplicateViewWidget::DuplicateViewWidget(QWidget *parent, int ibt, const float &sL)
    : QWidget(parent)
{
    scoreLimit   = sL;
    imageBoxType = ibt;

    btnOK            = new QPushButton(qtTrId("duplicatemodel.button.OK"));
    btnRefresh       = new QPushButton(qtTrId("duplicatemodel.button.Refresh"));

    QColor c = palette().color(QPalette::Highlight);
    hexApplied = c.name();
    QColor hsv  = c.toHsv();
    hsv.setHsv(hsv.hue(), std::max(0, hsv.saturation() - 120), std::min(255, hsv.value() + 0));
    hexChanged = hsv.name();

    btnApply         = new QPushButton(qtTrId("duplicatemodel.button.Apply"));
    sliderScoreLimit = new QSlider(Qt::Horizontal);
    btnApply->setEnabled(false);
    sliderScoreLimit->setRange(5, 100);
    sliderScoreLimit->setValue((int)(scoreLimit * 100));
    sliderScoreLimit->setStyle(QStyleFactory::create("fusion"));

    // Scrollable grid erzeugen  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    QWidget *container = new QWidget;
    grid = new QGridLayout(container);

    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(30);

    scroll->setWidget(container);


    // Score Limit Slider - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    QLabel *minValue = new QLabel("0.05");
    QLabel *maxValue = new QLabel("1.0");

    sliderContainer = new QWidget;
    sliderContainer->setFixedHeight(60);
    sliderContainer->setMinimumWidth(200);
    sliderContainer->installEventFilter(this);

    auto *containerLayout = new QVBoxLayout(sliderContainer);
    containerLayout->setContentsMargins(0, 20, 0, 0);
    containerLayout->setSpacing(0);

    // Slider normal ins Layout
    containerLayout->addWidget(sliderScoreLimit);

    // Label als Overlay (frei beweglich)
    valueLabel = new QLabel("0.5", sliderContainer);
    valueLabel->setFixedWidth(50);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->raise();

    connect(sliderScoreLimit, &QSlider::valueChanged,
            this, [=](int value) {

            btnApply->setEnabled(true);

            updateLabelPos();

            QPalette p = sliderScoreLimit->palette();
            p.setColor(QPalette::Highlight, QColor(hexChanged));
            sliderScoreLimit->setPalette(p);
    });

    QTimer::singleShot(0, this, [=]() {
        sliderScoreLimit->setValue(sliderScoreLimit->value());  // triggert reposition
    });


    // Footer - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto footer = new QHBoxLayout();
    footer->addStretch(20);
    footer->addWidget(btnRefresh);

    footer->addStretch(5);

    footer->addWidget(btnOK);
    footer->addStretch(20);

    if (imageBoxType == 0)
    {
        footer->addWidget(minValue);
        footer->addWidget(sliderContainer);   // alt: Layout
        footer->addWidget(maxValue);
        footer->addWidget(btnApply);
    }


    // Layout erzeugen - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    auto layout = new QVBoxLayout(this);
    layout->addWidget(scroll);
    // layout->addWidget(btnOK);
    layout->addLayout(footer);

    connect(btnOK, &QPushButton::clicked,
            this, &DuplicateViewWidget::close);

    connect(btnRefresh, &QPushButton::clicked,
            this, [this]() { emit refreshList(); }); // lambda to emit to MainWindow eeeeeeeeeeeeeeee

    switch (imageBoxType)
    {case 0:
        setWindowTitle(qtTrId("duplicatemodel.title.Duplicates"));
        break;
     case 1:
        setWindowTitle(qtTrId("duplicatemodel.title.manNoDupl"));
         break;
    }


    // Geometrie wiederherstellen  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    QTimer::singleShot(0, this, [this]() {
        QSettings settings("DuplicateFinder", "DuplicateFinder");

        settings.beginGroup("DuplicateView");

        if (settings.contains("geometry"))
            restoreGeometry(settings.value("geometry").toByteArray());

        settings.endGroup();

        scoreLimit = settings.value("Program/scoreLimit").toInt() / 100.0;
    });

    sliderScoreLimit->setValue(scoreLimit * 100);


    connect(btnApply, &QPushButton::clicked,
            this, &DuplicateViewWidget::onApply);

}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::closeEvent(QCloseEvent *event)
{
    QSettings settings("DuplicateFinder", "DuplicateFinder");

    settings.beginGroup("DuplicateView");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    settings.setValue("Program/scoreLimit",  (int)(scoreLimit * 100) );

    QWidget::closeEvent(event);
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::updateLabelPos()
{
    int value = sliderScoreLimit->value();

    valueLabel->setText(QString::number(value / 100.0, 'f', 2));

    int sliderWidth = sliderScoreLimit->width();
    int min = sliderScoreLimit->minimum();
    int max = sliderScoreLimit->maximum();

    double ratio = (value - min) / double(max - min);

    int handleWidth = 26;
    int margin = handleWidth / 2;
    int usableWidth = sliderWidth - handleWidth;

    int x = margin + ratio * usableWidth;

    valueLabel->move(x - valueLabel->width()/2, 0);
};

// --------------------------------------------------------------------------------------------------------------
bool DuplicateViewWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sliderContainer && event->type() == QEvent::Resize)
        updateLabelPos();
    return false;

    return QWidget::eventFilter(obj, event);
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::onApply()
{
    btnApply->setEnabled(false);

    QPalette p = sliderScoreLimit->palette();
    p.setColor(QPalette::Highlight, QColor(hexApplied));
    sliderScoreLimit->setPalette(p);

    scoreLimit = sliderScoreLimit->value() / 100.0f;

    QSettings settings("DuplicateFinder", "DuplicateFinder");
    settings.setValue("Program/scoreLimit",  (int)(scoreLimit * 100) );

    emit scoreHasChanged();
}

// --------------------------------------------------------------------------------------------------------------
QWidget* DuplicateViewWidget::createImageBox(const ImageInfo &origImg, const ImageInfo &img, const bool firstImg, int groupNbr,const Type t, const float score, const int distance, const float diff, int clust, int item)
{

    ImageBox *box = new ImageBox(origImg, img, firstImg, groupNbr, t, score, distance, diff, clust, item, imageBoxType);

    if (firstImg && (item < 0))
        m_rootBoxes.insert(clust, box);
    else if (item >= 0)
        m_boxes.insert({clust, item}, box);

    connect(box, &ImageBox::openRequested,
            this, &DuplicateViewWidget::openImageExtern);

    connect(box, &ImageBox::deleteRequested,
            this, &DuplicateViewWidget::deleteRequested);

    if (imageBoxType == 0)
        connect(box, &ImageBox::makeMasterRequested,
                this, &DuplicateViewWidget::makeMasterRequested);

    connect(box, &ImageBox::noDuplicates,
            this, &DuplicateViewWidget::noDuplicates);

    grid->addWidget(box);

    return box;
}


// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::processNextCluster()
{
    if (clusterIdx >= m_pendingClusters.size())
    {
        if (picturesOverflow)
            switch (imageBoxType)
            {case 0:
                setWindowTitle(qtTrId("duplicatemodel.title.Duplicates.overflow").arg(datasize));
                break;
            case 1:
                setWindowTitle(qtTrId("duplicatemodel.title.manNoDupl.overflow").arg(datasize));
                break;
            }

        grid->setEnabled(true);
        return;
    }

    addCluster(m_pendingClusters[clusterIdx]);
    clusterIdx++;

    // UI kurz atmen lassen
    QTimer::singleShot(0, this, &DuplicateViewWidget::processNextCluster);
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::addCluster(const DuplicateCluster &cluster)
{
    int col = 0;
//    ++groupNbr;

    QVector<ImageInfo> allInfo;
    QVector<Type>      allTypes;
    QVector<float>     allScores;
    QVector<int>       allDistances;
    QVector<float>     allDiff;
    allInfo.push_back(cluster.root);
    allTypes.push_back(Copy);
    allScores.push_back(0.0);
    allDistances.push_back(0);
    allDiff.push_back(0);

    for (const auto &it : cluster.items) {
        allInfo.push_back(it.info);
        allTypes.push_back(it.type);
        allScores.push_back(it.score);
        allDistances.push_back(it.pHashDistance);
        allDiff.push_back(it.cannyDiff);
    }

    // jetzt paarweise darstellen
    for (int i = 0; i < allInfo.size(); i += 2)
    {   int col = 0;
        if (++totalPic <= maxPictures) {
            grid->addWidget(createImageBox(allInfo[0], allInfo[i], (i == 0), clusterIdx+1, allTypes[i], allScores[i], allDistances[i], allDiff[i], clusterIdx, i-1), row, col++);

            if (++totalPic <= maxPictures) {
                if (i + 1 < allInfo.size())
                    grid->addWidget(createImageBox(allInfo[0], allInfo[i + 1], false, clusterIdx+1, allTypes[i+1], allScores[i+1], allDistances[i+1], allDiff[i+1], clusterIdx, i), row, col++);
            }
        }
        row++;
        if (++totalPic > maxPictures) {
            picturesOverflow = true;
            break;
        }
    }
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::setData(const QVector<DuplicateCluster> &data)
{
    if (data.isEmpty())
        return;

    // clear old UI
    while (auto item = grid->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    datasize = data.size();

    switch (imageBoxType)
    {case 0:
        setWindowTitle(qtTrId("duplicatemodel.title.Duplicates.count").arg(datasize));
        break;
    case 1:
        setWindowTitle(qtTrId("duplicatemodel.title.manNoDupl.count").arg(datasize));
        break;
    }

    row              = 0;
    clusterIdx       = 0;
    totalPic         = 0;
    picturesOverflow = false;

    addCluster(data.first());

    // Rest in Queue
    m_pendingClusters = data;

    // Hintergrund starten
    clusterIdx++;
    QTimer::singleShot(0, this, &DuplicateViewWidget::processNextCluster);
}


// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::openImageExtern(const QString &path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::onDeleteResult(int clusterIndex, int itemIndex, bool ok)
{
    if (!ok)
        return;

    ImageBox *box = m_boxes.value({clusterIndex, itemIndex}, nullptr);

    if (box)
         box->setDeleted(true);
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::onNoDuplicateResult(int clusterIndex, int itemIndex)
{
    ImageBox *box = m_boxes.value({clusterIndex, itemIndex}, nullptr);
    if (box)
        box->setNoDuplicate();
}

// --------------------------------------------------------------------------------------------------------------
void DuplicateViewWidget::onMakeMasterResult(int clusterIndex, int itemIndex, QVector<float> newScores, QVector<int> newDistances, QVector<float> newDiff)
{
    ImageBox *rootBox = m_rootBoxes[clusterIndex];
    BoxData dummy{};

    QVector<ImageBox*> itemBoxes;
    for (int i = 0; i < newScores.size(); ++i) {
        itemBoxes.append(m_boxes.value({clusterIndex, i}, nullptr));
    }

    BoxData oldRoot;
    BoxData oldItem;
    if (rootBox && itemIndex < itemBoxes.size() && itemBoxes[itemIndex]) {
        oldRoot = rootBox->getBoxData();
        oldItem = itemBoxes[itemIndex]->getBoxData();

        rootBox->setBoxData(oldItem, dummy, true, false, false);
        itemBoxes[itemIndex]->setBoxData(oldRoot, oldItem, false, false, false);
    }

    for (int c = 0; c < itemBoxes.size(); ++c) {
        itemBoxes[c]->setScore(newScores[c], newDistances[c], newDiff[c]);
        itemBoxes[c]->updateLabelColors(rootBox);
        if (c != itemIndex)   // itemIndex update already done above
            itemBoxes[c]->setNewMasterImg(oldItem.image);
    }

    int a=1;
}


// --------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------
