#include "imagebox.h"

#include <QDateTime>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QImageReader>
#include <QLocale>
#include <QSettings>
#include <QTimer>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QClipboard>

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QDesktopServices>

// -----------------------------------------------------------------------------------------------------------------------
namespace
{
class FilenameLabel : public QLabel
{
public:
    explicit FilenameLabel(const QString& text, QWidget* parent = nullptr)
        : QLabel(text, parent)
    {
        setTextInteractionFlags(Qt::TextSelectableByMouse |
                                Qt::TextSelectableByKeyboard);
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        QLabel::mouseDoubleClickEvent(event);

        if (event->button() == Qt::LeftButton)
        {
            QString baseName = QFileInfo(text()).completeBaseName();
            QGuiApplication::clipboard()->setText(baseName);
        }
    }
};
} // namespace

// -----------------------------------------------------------------------------------------------------------------------
ImageBox::ImageBox(const ImageInfo &origImg, const ImageInfo &img, bool firstImg, int groupNbr, Type t, float score, int distance, float diff, int clust, int item,
                   int showType,QWidget *parent) : QWidget(parent),
    m_clust(clust),
    m_item(item)
{
    QLocale locale;
    QFont boldFont;
    boldFont.setBold(true);

    imgPath = img.path;
    imgType = t;

    imageBoxType = showType;

    QString dateString = QDateTime::fromSecsSinceEpoch(img.lastModified).toString(qtTrId("imagebox.formatting.date"));

    // QWidget *w = new QWidget;
    auto *vertical = new QVBoxLayout(this);

    m_image = new QLabel;
//    m_image->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_image->setFixedHeight(imageHeightDisplay);
    m_image->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_image->setStyleSheet("background: transparent;"); // Platzhalter

    // Bild laden:
    QImageReader::setAllocationLimit(512);  // Ersetzt 128MB-Limit durch 512MB-Limit

    QImageReader reader(img.path);
    reader.setAutoTransform(true);
    QSize originalSize = reader.size();   // nur Header-Info
    if (originalSize.isValid())
    {
        int newWidth = originalSize.width() * imageHeightDisplay / originalSize.height();
        reader.setScaledSize(QSize(newWidth, imageHeightDisplay));
    } else
        qDebug() << "originalSize is not valid: " << img.path;  // tttttttttt

    QImage imgScaled = reader.read();
    if (!imgScaled.isNull())
    {
        m_image->setPixmap(QPixmap::fromImage(imgScaled));
        m_image->setAlignment(Qt::AlignLeft);
    }

/*
    QPixmap pix = QPixmap(img.path);

    if (!pix.isNull()) {
        pix = pix.scaledToHeight( imageHeightDisplay,
                                 // Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        m_image->setPixmap(pix);
        m_image->setAlignment(Qt::AlignLeft);
    }
*/
    QLabel *row1Label = new QLabel(qtTrId("imagebox.label.type"));
    row1Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    switch (imageBoxType)
    {case 0:
        row2Label = new QLabel((firstImg)? qtTrId("imagebox.label.number") : qtTrId("imagebox.label.score")); break;
    case 1:
        row2Label = new QLabel("");                             break;
    default: break;
    }
                                                                        row2Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel *row3Label = new QLabel(qtTrId("imagebox.label.filename"));  row3Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel *row4Label = new QLabel(qtTrId("imagebox.label.folder"));    row4Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel *row5Label = new QLabel(qtTrId("imagebox.label.filedate"));  row5Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel *row6Label = new QLabel(qtTrId("imagebox.label.filesize"));  row6Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QLabel *row7Label = new QLabel(qtTrId("imagebox.label.imageSize")); row7Label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto makeLabel = [&](const QString& text) {
        QLabel* label = new QLabel(text);
        label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        return label;
    };

    auto makeLabelFilename = [&](const QString& filename)
    {
        auto* label = new FilenameLabel(filename);
        label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return label;
    };

    switch (imageBoxType)
    {case 0:
        row1Data = makeLabel((firstImg)? qtTrId("imagebox.data.root") : ((t == Copy)? qtTrId("imagebox.data.copy") : qtTrId("imagebox.data.duplicate")));
        row2Data = makeLabel((firstImg)? QString("%1").arg(groupNbr) : QString("pHash: %2, canny: %3, orb: %1").arg(QString::number(score, 'f', 2)).arg(distance).arg(QString::number(diff, 'f', 2)));
        break;
    case 1:
        row1Data = makeLabel((firstImg)? qtTrId("imagebox.data.first") : qtTrId("imagebox.data.second"));
        // row2Data = makeLabel((firstImg)? QString("%1").arg(groupNbr) : QString("%1 (pHash: %2, canny: %3)").arg(QString::number(score, 'f', 2)).arg(distance).arg(QString::number(diff, 'f', 2)));
        break;
    default: break;
    }
    row3Data = makeLabelFilename(img.filename);
    row4Data = makeLabel(QFileInfo(img.path).absolutePath());
    row5Data = makeLabel(QString("%1").arg(dateString));
    row6Data = makeLabel(locale.toString(img.size));
    row7Data = makeLabel(QString("%1 x %2 (%3)").arg(img.width).arg(img.height).arg((img.height > 0)? QString::number(img.width * 1.0f / img.height, 'f', 2) : "..."));

    if ( firstImg && imageBoxType == 0) row1Data->setFont(boldFont);
    if (!firstImg && imageBoxType == 0) {
        QFont boldFont;
        boldFont.setBold(true);

        QPalette greenPalette = row3Data->palette();
        greenPalette.setColor(QPalette::WindowText, Qt::darkGreen);

        if (score <= 0 && distance == 0 && diff == 0) {
            row2Data->setPalette(greenPalette);
            row2Data->setFont(boldFont);
        }
    }

    defaultPalette   = row5Data->palette();
    highlightPalette = defaultPalette;
    highlightPaletteGreen = defaultPalette;
    highlightPaletteRed = defaultPalette;
    highlightPalette.setColor(QPalette::WindowText, Qt::blue);
    highlightPaletteGreen.setColor(QPalette::WindowText, Qt::darkGreen);
    highlightPaletteRed.setColor(QPalette::WindowText, Qt::darkRed);

    if (origImg.lastModified != img.lastModified                         ) row5Data->setPalette(highlightPalette);
    if (origImg.size         != img.size                                 ) row6Data->setPalette(highlightPalette);
    if (origImg.width        != img.width || origImg.height != img.height) row7Data->setPalette(highlightPalette);

    if (origImg.size         < img.size                                 ) row6Data->setPalette(highlightPaletteGreen);
    if (origImg.size         > img.size                                 ) row6Data->setPalette(highlightPaletteRed);

         if ((origImg.width * origImg.height)  < (img.width * img.height)) row7Data->setPalette(highlightPaletteGreen);
    else if ((origImg.width * origImg.height)  > (img.width * img.height)) row7Data->setPalette(highlightPaletteRed);
    else if (origImg.width != img.width  ||  origImg.height != img.height) row7Data->setPalette(highlightPalette);


    QGridLayout *g = new QGridLayout();
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(2);
    g->setContentsMargins(0,0,0,0);
    g->setColumnStretch(0, 0); // Labels
    g->setColumnStretch(1, 1); // Daten

    g->addWidget(row1Label, 0, 0); g->addWidget(row1Data,  0, 1);
    if (imageBoxType == 0)
    {
        g->addWidget(row2Label, 1, 0); g->addWidget(row2Data,  1, 1);
    }
    g->addWidget(row3Label, 2, 0); g->addWidget(row3Data,  2, 1);
    g->addWidget(row4Label, 3, 0); g->addWidget(row4Data,  3, 1);
    g->addWidget(row5Label, 4, 0); g->addWidget(row5Data,  4, 1);
    g->addWidget(row6Label, 5, 0); g->addWidget(row6Data,  5, 1);
    g->addWidget(row7Label, 6, 0); g->addWidget(row7Data,  6, 1);


    QPushButton *btnOpen;
    QPushButton *btnDelete;
    QPushButton *btnMakeRoot;
    QPushButton *btnCompare;

    btnOpen     = new QPushButton(qtTrId("imagebox.button.open"));
    if (!firstImg && imageBoxType == 0) btnDelete   = new QPushButton(qtTrId("imagebox.button.delete"));
    if (!firstImg && imageBoxType == 0) btnMakeRoot = new QPushButton(qtTrId("imagebox.button.master"));
    // if (             imageBoxType == 1) btnMakeRoot = new QPushButton("-> Tauschen");
    if (!firstImg                     ) btnCompare  = new QPushButton(qtTrId("imagebox.button.compare"));
    if (!firstImg && imageBoxType == 0) btnNoDupl   = new QPushButton(qtTrId("imagebox.button.noDupl"));
    if (!firstImg && imageBoxType == 1) btnNoDupl   = new QPushButton(qtTrId("imagebox.button.manNoDuplDel"));

    btnContainer = new QWidget;
    btns = new QHBoxLayout(btnContainer);
    btns->setAlignment(Qt::AlignLeft);
    btns->setSpacing(20);
    btnContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // 'Open' Button
    btns->addWidget(btnOpen); btnOpen->setMinimumWidth(100);
    connect(btnOpen, &QPushButton::clicked,
            this, [=]() {
                emit openRequested(imgPath);
            });

    // 'Delete' Button
    if (!firstImg && imageBoxType == 0)
    {
        btns->addWidget(btnDelete); btnDelete->setMinimumWidth(100);
        connect(btnDelete, &QPushButton::clicked,
                this, [=]() {
                    emit deleteRequested(clust, item, imageBoxType);
                });
    }

    // 'Master' Button
    if (!firstImg && imageBoxType == 0)
    {
        btns->addWidget(btnMakeRoot); btnMakeRoot->setMinimumWidth(100);
        connect(btnMakeRoot, &QPushButton::clicked,
                this, [=]() {
                    emit makeMasterRequested(clust, item);
                });
    }

    // 'Compare' Button
    if (!firstImg)
    {
        btns->addWidget(btnCompare); btnCompare->setMinimumWidth(100);
        connect(btnCompare, &QPushButton::clicked,
                this, &ImageBox::toggleCompare);
    }

    // 'No Duplicate' Button
    if (!firstImg)
    {
        btns->addWidget(btnNoDupl); btnNoDupl->setMinimumWidth(100);
        connect(btnNoDupl, &QPushButton::clicked,
                this, [=]() {
                    emit noDuplicates(clust, item, imageBoxType);
                });
    }

    vertical->setContentsMargins(0,0,0,0);
    vertical->addWidget(m_image);
    vertical->addLayout(g);
    vertical->addWidget(btnContainer);

/*
    m_originalA = QImage(origImg.path);
    m_originalB = QImage(img.path);
*/
    QImageReader readerA(origImg.path);
    readerA.setAutoTransform(true);
    QSize originalSizeA = readerA.size();
    if (originalSizeA.isValid())
    {
        int newWidthA = originalSizeA.width() * imageHeightDisplay / originalSizeA.height();
        readerA.setScaledSize(QSize(newWidthA, imageHeightDisplay));
    } else
        qDebug() << "originalSize is not valid: " << origImg.path;
    m_originalA = readerA.read();

    QImageReader readerB(img.path);
    readerB.setAutoTransform(true);
    QSize originalSizeB = readerB.size();
    if (originalSizeB.isValid())
    {
        int newWidthB = originalSizeB.width() * imageHeightDisplay / originalSizeB.height();
        readerB.setScaledSize(QSize(newWidthB, imageHeightDisplay));
    } else
        qDebug() << "originalSize is not valid: " << img.path;
    m_originalB = readerB.read();


/*
    QSize size = m_originalA.size().boundedTo(m_originalB.size());

    QImage a = m_originalA.scaled(size);
    QImage b = m_originalB.scaled(size);

    m_diffImage = QImage(size, QImage::Format_RGB32);

    for (int y = 0; y < size.height(); ++y)
    {
        const QRgb *pa = (const QRgb*)a.scanLine(y);
        const QRgb *pb = (const QRgb*)b.scanLine(y);
              QRgb *pr = (QRgb*)m_diffImage.scanLine(y);

        for (int x = 0; x < size.width(); ++x)
        {
            pr[x] = qRgb(
                qAbs(qRed(pa[x])   - qRed(pb[x])),
                qAbs(qGreen(pa[x]) - qGreen(pb[x])),
                qAbs(qBlue(pa[x])  - qBlue(pb[x]))
                );
        }
    }
*/

}

// -----------------------------------------------------------------------------------------------------------------------
QString showDiffText(int showDiff)
{
    switch (showDiff) {
    case 1 : return qtTrId("imagebox.text.compareDiff");
    case 2 : return qtTrId("imagebox.text.compareXOR");
    default: return "";
    }
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::toggleCompare()
{
    switch (showDiff) {

    case 0:
        // setImage(m_diffImage);
        setImage(calculateDiffImg(m_originalA, m_originalB));
        showDiff = 1;
        break;

    case 1:
        setImage(calculateExclImg(m_originalA, m_originalB));
        showDiff = 2;
        break;

    case 2:
        setImage(m_originalB);
        showDiff = 0;
    }

    if (imgType == Copy)
        row1Data->setText(qtTrId("imagebox.data.copy.extra").arg(showDiffText(showDiff)));
    else
        row1Data->setText(qtTrId("imagebox.data.duplicate.extra").arg(showDiffText(showDiff)));
    //row1Data->setText(QString("%1%2").arg((imgType == Copy)? "Kopie" : "DUPLIKAT").arg(showDiffText(showDiff)));
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setImage(const QImage &img)
{
    QPixmap pix = QPixmap::fromImage(img);

    int w = m_image->width()  > 0 ? m_image->width()  : imageHeightDisplay;
    int h = m_image->height() > 0 ? m_image->height() : imageHeightDisplay;

    pix = pix.scaled(
        w,
        h,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    m_image->setPixmap(pix);
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setDeleted(bool deleted)
{
    QFont font;
    font.setBold(true);

    QPalette redPalette = row1Data->palette();
    redPalette.setColor(QPalette::WindowText, Qt::red);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.25);  // 0.0 = unsichtbar, 1.0 = voll sichtbar


    if (deleted)
    {
        row1Data->setText(qtTrId("imagebox.data.deleted"));
        row1Data->setPalette(redPalette);
        row1Data->setFont(font);
        btnContainer->setVisible(false);
        setImage(m_originalB);
        m_image->setGraphicsEffect(effect);
    }
}


// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setNoDuplicate()
{
    QFont font;
    font.setBold(true);

    QPalette bluePalette = row1Data->palette();
    bluePalette.setColor(QPalette::WindowText, Qt::darkCyan);

    switch (imageBoxType)
    {case 0:
        row1Data->setText(qtTrId("imagebox.data.manNuDupl").arg(showDiffText(showDiff)));
        break;
    case 1:
        row1Data->setText(qtTrId("imagebox.data.delManNuDupl"));
        break;
    default: break;
    }
    row1Data->setPalette(bluePalette);
    // row1Data->setFont(font);
    btnNoDupl->setEnabled(false);
}

// -----------------------------------------------------------------------------------------------------------------------
BoxData ImageBox::getBoxData()
{   BoxData result;

    result.image     = m_originalB;
    result.type      = row1Data->text();
    if (imageBoxType == 0)
        result.score     = row2Data->text();
    result.filename  = row3Data->text();
    result.folder    = row4Data->text();
    result.fileDate  = row5Data->text();
    result.fileSize  = row6Data->text();
    result.imageSize = row7Data->text();
    result.imgPath   = imgPath;

    return result;
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setBoxData(BoxData &newData, BoxData &newOrig, bool rootEntry, bool setType, bool setScore)
{
    m_originalB = newData.image;

    // Calculate new "Diff" Image
    if (!rootEntry) {
        m_originalA = newOrig.image;
        // m_diffImage = calculateDiffImg(newOrig.image, m_originalB);

    }

    switch (showDiff) {
        case 0: setImage(m_originalB); break;
//      case 1: setImage(m_diffImage); break;
        case 1: setImage(calculateDiffImg(m_originalA, m_originalB)); break;
        case 2: setImage(calculateExclImg(m_originalA, m_originalB)); break;
    }


    if (setType  && !rootEntry) row1Data->setText(QString("%1%2").arg(newData.type, showDiffText(showDiff)));
//    else if (!setType && !rootEntry) row1Data->setText(QString("%1%2").arg(newOrig.type).arg(showDiffText(showDiff)));
    if (setScore && !rootEntry) row2Data->setText(newData.score);

    row3Data->setText(newData.filename);
    row4Data->setText(newData.folder);
    row5Data->setText(newData.fileDate);
    row6Data->setText(newData.fileSize);
    row7Data->setText(newData.imageSize);

    imgPath = newData.imgPath;

}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::updateLabelColors(ImageBox *rootBox)
{
    QLocale locale;
    bool ok1 = false;
    bool ok2 = false;

    if (rootBox->row5Data->text() != row5Data->text())
        row5Data->setPalette(highlightPalette);
    else
        row5Data->setPalette(defaultPalette);

    int valueRoot = locale.toDouble(rootBox->row6Data->text(), &ok1);
    int value2    = locale.toDouble(row6Data->text()         , &ok2);

    if (ok1 && ok2)
    {
             if (valueRoot < value2) row6Data->setPalette(highlightPaletteGreen);
        else if (valueRoot > value2) row6Data->setPalette(highlightPaletteRed);
        else row6Data->setPalette(defaultPalette);
    }
/*
    if (rootBox->row6Data->text() != row6Data->text())
        row6Data->setPalette(highlightPalette);
    else
        row6Data->setPalette(defaultPalette);
*/

    QString textRoot = rootBox->row7Data->text();
    QString text2    = row7Data->text();

    QRegularExpression re(R"(^\s*(\d+)\s*x\s*(\d+))");

    QRegularExpressionMatch matchRoot = re.match(textRoot);  // Checking whether image size info has the format  'w x h ...'
    QRegularExpressionMatch match2    = re.match(text2);

    if (matchRoot.hasMatch() && match2.hasMatch())
    {
        int widthRoot  = matchRoot.captured(1).toInt();
        int heightRoot = matchRoot.captured(2).toInt();
        int width2     = match2.captured(1).toInt();
        int height2    = match2.captured(2).toInt();

             if ((widthRoot * heightRoot)  < (width2 * height2)) row7Data->setPalette(highlightPaletteGreen);
        else if ((widthRoot * heightRoot)  > (width2 * height2)) row7Data->setPalette(highlightPaletteRed);
        else if (widthRoot != width2  ||  heightRoot != height2) row7Data->setPalette(highlightPalette);
        else row7Data->setPalette(defaultPalette);
    }

/*
    if (rootBox->row7Data->text() != row7Data->text())
        row7Data->setPalette(highlightPalette);
    else
        row7Data->setPalette(defaultPalette);
*/
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setNewMasterImg(QImage &newMaster)
{
    m_originalA = newMaster;

    if (showDiff == 1)
        setImage(calculateDiffImg(m_originalA, m_originalB));
//        setImage(m_diffImage);
    else if (showDiff == 2)
        setImage(calculateExclImg(m_originalA, m_originalB));
}

// -----------------------------------------------------------------------------------------------------------------------
void ImageBox::setScore(float newScore, int newDistance, float newDiff)
{
    QFont boldFont;
    boldFont.setBold(true);

    QFont normalFont;
    normalFont.setBold(false);

    QPalette stdPalette   = row3Data->palette();
    QPalette greenPalette = row3Data->palette();
    greenPalette.setColor(QPalette::WindowText, Qt::darkGreen);

    if (imageBoxType == 0)
    {
        row2Data->setText(qtTrId("imagebox.data.scoreData").arg(QString::number(newScore, 'f', 2)).arg(newDistance).arg(QString::number(newDiff, 'f', 2)));
        row2Data->setText(qtTrId("imagebox.data.scoreData").arg(QString::number(newScore, 'f', 2),
                                                                QString::number(newDistance),
                                                                QString::number(newDiff, 'f', 2)));
        if (newScore <= 0 && newDistance == 0 && newDiff == 0) {
            row2Data->setPalette(greenPalette);
            row2Data->setFont(boldFont);
        } else {
            row2Data->setPalette(stdPalette);
            row2Data->setFont(normalFont);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------
QImage ImageBox::calculateDiffImg(const QImage &pic1, const QImage &pic2)
{
//    QSize size = pic1.size().boundedTo(pic2.size());

    const int h  = pic1.height();
    const int w1 = pic1.width();
    const int w2 = pic2.width();
    const int W  = std::max(w1, w2);

//    QImage a = pic1.scaled(size);
//    QImage b = pic2.scaled(size);

//    QImage diffImage = QImage(size, QImage::Format_RGB32);
    QImage diffImage(W, h, QImage::Format_RGB32);
    diffImage.fill(Qt::black);

    const int off1 = (W - w1) / 2;
    const int off2 = (W - w2) / 2;

//    for (int y = 0; y < size.height(); ++y)
    for (int y = 0; y < h; ++y)
    {
        const QRgb *pa = (const QRgb*)pic1.scanLine(y);
        const QRgb *pb = (const QRgb*)pic2.scanLine(y);
        QRgb *pr = (QRgb*)diffImage.scanLine(y);

//        for (int x = 0; x < size.width(); ++x)
        for (int x = 0; x < W; ++x)
        {
            // A pixel holen oder "missing"
            bool aValid = (x >= off1 && x < off1 + w1);
            bool bValid = (x >= off2 && x < off2 + w2);

            QRgb ca = aValid ? pa[x - off1] : qRgb(0,0,0);
            QRgb cb = bValid ? pb[x - off2] : qRgb(0,0,0);

            pr[x] = qRgb(
//                qAbs(qRed(pa[x])   - qRed(pb[x])),
//                qAbs(qGreen(pa[x]) - qGreen(pb[x])),
//                qAbs(qBlue(pa[x])  - qBlue(pb[x]))
                qAbs(qRed(ca)   - qRed(cb)),
                qAbs(qGreen(ca) - qGreen(cb)),
                qAbs(qBlue(ca)  - qBlue(cb))
                );
        }
    }

    return diffImage;
}

// -----------------------------------------------------------------------------------------------------------------------
QImage ImageBox::calculateExclImg(const QImage &pic1, const QImage &pic2)
{
    const int h  = pic1.height();
    const int w1 = pic1.width();
    const int w2 = pic2.width();
    const int W  = std::max(w1, w2);

    QImage diffImage(W, h, QImage::Format_RGB32);
    diffImage.fill(Qt::black);

    const int off1 = (W - w1) / 2;
    const int off2 = (W - w2) / 2;

    for (int y = 0; y < h; ++y)
    {
        const QRgb *pa = (const QRgb*)pic1.scanLine(y);
        const QRgb *pb = (const QRgb*)pic2.scanLine(y);
        QRgb *pr = (QRgb*)diffImage.scanLine(y);

        for (int x = 0; x < W; ++x)
        {
            // A pixel holen oder "missing"
            bool aValid = (x >= off1 && x < off1 + w1);
            bool bValid = (x >= off2 && x < off2 + w2);

            QRgb ca = aValid ? pa[x - off1] : qRgb(0,0,0);
            QRgb cb = bValid ? pb[x - off2] : qRgb(0,0,0);

            pr[x] = qRgb(
                qAbs(qRed(ca)   ^ qRed(cb)),
                qAbs(qGreen(ca) ^ qGreen(cb)),
                qAbs(qBlue(ca)  ^ qBlue(cb))
                );
        }
    }

    return diffImage;

/*
    QSize size = pic1.size().boundedTo(pic2.size());

    QImage a = pic1.scaled(size);
    QImage b = pic2.scaled(size);

    QImage diffImage = QImage(size, QImage::Format_RGB32);

    for (int y = 0; y < size.height(); ++y)
    {
        const QRgb *pa = (const QRgb*)a.scanLine(y);
        const QRgb *pb = (const QRgb*)b.scanLine(y);
        QRgb *pr = (QRgb*)diffImage.scanLine(y);

        for (int x = 0; x < size.width(); ++x)
        {
            pr[x] = qRgb(
                qRed(pa[x])   ^ qRed(pb[x]),
                qGreen(pa[x]) ^ qGreen(pb[x]),
                qBlue(pa[x])  ^ qBlue(pb[x])
                );
        }
    }

    return diffImage;
*/
}

// -----------------------------------------------------------------------------------------------------------------------



// -----------------------------------------------------------------------------------------------------------------------



// -----------------------------------------------------------------------------------------------------------------------



// -----------------------------------------------------------------------------------------------------------------------
