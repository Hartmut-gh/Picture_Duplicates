#pragma once

#include "folderset.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>


struct BoxData {
    QImage   image;
    QString  type;
    QString  score;
    QString  filename;
    QString  folder;
    QString  fileDate;
    QString  fileSize;
    QString  imageSize;
    QString  imgPath;
};


// ---------------------------------------------------------------------------------------------------------------
class ImageBox : public QWidget
{
    Q_OBJECT

public:
    ImageBox(const ImageInfo &origImg,
             const ImageInfo &img,
             bool firstImg,
             int groupNbr,
             Type t,
             float score,
             int distance,
             float diff,
             int clust,
             int item,
             int showType,
             QWidget *parent = nullptr);

    void setDeleted(bool deleted);
    void setNoDuplicate();
    BoxData getBoxData();
    void setBoxData(BoxData &newData, BoxData &newOrig, bool rootEntry, bool setType, bool setScore);
    void updateLabelColors(ImageBox *rootBox);
    void setNewMasterImg(QImage &newMaster);
    void setScore(float newScore, int newDistance, float newDiff);

signals:
    void openRequested(const QString &path);
    void deleteRequested(int clust, int item, int imageBoxType);
    void makeMasterRequested(int clust, int item);
    void compareRequested(int clust, int item);
    void noDuplicates(int clust, int item, int imageBoxType);

private:
    QLabel *m_image;
    int m_clust;
    int m_item;

    int imageHeightDisplay = 300;

    QImage m_originalB;
    QImage m_originalA;
//    QImage m_diffImage;

    int imageBoxType;

    QLabel *row2Label;

    QLabel *row1Data;
    QLabel *row2Data;
    QLabel *row3Data;
    QLabel *row4Data;
    QLabel *row5Data;
    QLabel *row6Data;
    QLabel *row7Data;
    QHBoxLayout *btns;
    QPushButton *btnNoDupl;
    QWidget *btnContainer;
    QString imgPath;

    QPalette defaultPalette  ;
    QPalette highlightPalette;
    QPalette highlightPaletteGreen;
    QPalette highlightPaletteRed;


    int    showDiff = 0;
    Type   imgType = Duplicate;

    void toggleCompare();
    void setImage(const QImage &img);
    QImage calculateDiffImg(const QImage &pic1, const QImage &pic2);
    QImage calculateExclImg(const QImage &pic1, const QImage &pic2);
};

