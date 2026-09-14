#pragma once

#include "folderset.h"
#include "imagebox.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>



class DuplicateViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DuplicateViewWidget(QWidget *parent = nullptr, int ibt = 0, const float &sL = 0.5);

    void setData(const QVector<DuplicateCluster> &data);

signals:
    void scoreHasChanged();
    void deleteRequested(int clust, int item, int imageBoxType);
    void makeMasterRequested(int clust, int item);
    void noDuplicates(int clust, int item, int imageBoxType);
    void refreshList();

public slots:
    void openImageExtern(const QString &path);
    void onDeleteResult(int clusterIndex, int itemIndex, bool ok);
    void onMakeMasterResult(int clusterIndex, int itemIndex, QVector<float> newScores, QVector<int> newDistances, QVector<float> newDiff);
    void onNoDuplicateResult(int clusterIndex, int itemIndex);

private slots:
    void onApply();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // QTreeWidget *tree;
    QGridLayout *grid;
    QPushButton *btnOK;
    QPushButton *btnRefresh;
    QPushButton *btnApply;
    QSlider     *sliderScoreLimit = nullptr;
    QWidget     *sliderContainer  = nullptr;
    QLabel      *valueLabel       = nullptr;

    QVector<DuplicateCluster> m_pendingClusters;
    QVector<ImageBox*> m_rootBoxes;
    QMap<QPair<int,int>, ImageBox*> m_boxes;

    int clusterIdx = -1;
    int row       = 0;
    int totalPic  = 0;

    int datasize  = 0;

    int imageBoxType;

    int  maxPictures      = 500;
    bool picturesOverflow = false;

    QString hexApplied;
    QString hexChanged;

    float scoreLimit = 0.5;

    QWidget* createImageBox(const ImageInfo &origImg, const ImageInfo &img, const bool firstImg, int groupNbr, const Type t, const float score, const int distance, float diff, int clust, int item);
    void updateLabelPos();

    void processNextCluster();
    void addCluster(const DuplicateCluster &cluster);

};
