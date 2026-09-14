#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "folderset.h"

#include <QMainWindow>
class QStandardItem;
class QStandardItemModel;
class QCloseEvent;
// #include <QMutex>

#include <QSettings>
#include <QVector>

#include <QFutureWatcher>
#include <QElapsedTimer>

#include <QSqlDatabase>

#include <QTranslator>

/*
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeView>
*/
class QComboBox;
class QHBoxLayout;
class QLabel;
class QProgressBar;
class QPushButton;
class QCheckBox;
class QTreeView;


#include <opencv2/opencv.hpp>

#include "duplicatemodel.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


// ---------------------------------------------------------------------------------
struct PairKey
{
    qint64 u1;
    qint64 u2;

    bool operator==(const PairKey &other) const
    {
        return u1 == other.u1 && u2 == other.u2;
    }
};

inline size_t qHash(const PairKey &k, size_t seed = 0)
{
    return qHashMulti(seed, k.u1, k.u2);
}

// ---------------------------------------------------------------------------------
struct ScoreInfo
{
    float score;
    bool  manualNoDuplicate;
};

// ---------------------------------------------------------------------------------
struct ManualInfo
{
    qint64  manTime;
    QString f1_filename;
    QString f1_filepath;
    qint64  f1_filesize;
    qint64  f1_lastmodified;
    QString f2_filename;
    QString f2_filepath;
    qint64  f2_filesize;
    qint64  f2_lastmodified;
};


// ---------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString v, QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;

signals:
    void updateProgress(qint64 progress);
    void updateProgressForThreads(qint64 progress);
    void progressUpdate(int value, int total);
    void deleteResult(int clust, int item, bool success);
    void makeMasterResult(int clust, int item, QVector<float> newScores, QVector<int> newDistances, QVector<float> newDiff);
    void noDuplicatesResult(int clust, int item);

public slots:
    // void startProcessing();
    void onProgressUpdated(int progress);
    void onUpdateProgress(int progress);
    void onProcessingFinished();
    void onErrorOccurred(const QString& errorMessage);

    void deleteImage(int clusterIndex, int itemIndex, int imageBoxType);
    void makeMaster(int clusterIndex, int itemIndex);
    void noDuplicates(int clusterIndex, int itemIndex, int imageBoxType);

private slots:
    void addFolder();
    bool folderExistsInModel(const QString &path, QStandardItem *parent);
    bool folderExistsInSet(const QString &path, QVector<FolderNode> *parent, FolderState &existingState, int &setNbr);
    // QModelIndex toRootIndex(const QModelIndex &index);
    // FolderNode* findRootForIndex(const QModelIndex &index);
    FolderNode* getNodeFromIndex(const QModelIndex &index);
    void removeFolder();
    void handleItemClicked(const QModelIndex &index);
    void createNewSet();
    void cloneSet();
    void deleteSet();
    void onSetChanged(int index);
    void onApply();
    void onReset();
    void cleanDB();

    void startStopScanAndFind();
    void scanAndFindDuplicates();
    void scanAndFindDuplicates_stepX();
    void scanAndFindDuplicates_step2();
    void scanAndFindDuplicates_step3();


private:
    QString version;

    QStandardItemModel *folderModel;
    QTreeView *folderTreeView;
    QComboBox *setCombo;
    QPushButton *btnApply;
    QPushButton *btnReset;

    QHBoxLayout *topLayout;
    QPushButton *btnNewSet;
    QPushButton *btnCloneSet;
    QPushButton *btnDeleteSet;

    QPushButton *btnFindDuplicates;
    QPushButton *btnCleanDB;
    QPushButton *btnManual;
    QPushButton *btnLanguage;

    // QCheckBox   *cbExcludeSameDir;
    QComboBox   *cbHandleSameDir;

    QSlider     *sliderScoreLimit = nullptr;
    QSlider     *sliderDistLower  = nullptr;
    QSlider     *sliderDistUpper  = nullptr;
    QSlider     *sliderCannyLower = nullptr;
    QSlider     *sliderCannyUpper = nullptr;
    QWidget     *sliderContainer1 = nullptr;
    QWidget     *sliderContainer2 = nullptr;
    QWidget     *sliderContainer3 = nullptr;
    QWidget     *sliderContainer4 = nullptr;
    QWidget     *sliderContainer5 = nullptr;
    QLabel      *value1Label      = nullptr;
    QLabel      *value2Label      = nullptr;
    QLabel      *value3Label      = nullptr;
    QLabel      *value4Label      = nullptr;
    QLabel      *value5Label      = nullptr;
    QLabel      *SliderLbl1;
    QLabel      *SliderLbl2;
    QLabel      *SliderLbl3;
    QLabel      *SliderLbl4;
    QLabel      *SliderLbl5;
    QLabel      *lblSameDir;


    QPushButton *btnAddFolder;
    QPushButton *btnRemoveFolder;

    QLabel       *progressLabel;
    QProgressBar *progressBar;
    QLabel       *volumeLabel;


    QElapsedTimer timer;

    QString lastDir = "";
    QString currentSetting = "Default";
    float scoreLimit          = 0.5;
    int   distanceLowerLimit  = 14;
    int   distanceUpperLimit  = 20;
    float cannyDiffLowerLimit = 0.03;  // Darunter immer ein Duplikat
    float cannyDiffUpperLimit = 0.1;   // Darüber  nie   ein Duplikat
    int   maxDuplToShow       = 400;

    int noManualSort      = 0;  // 0 -> last added entries first; 1 = Alphabetically
                                //                                2 = FileSize
                                //                                3 = File LastModified
                                //                                4 = File Image Size
    int noManualSortDir   = 0;  // 0 -> Ascending;   1 = Descending
    int noManualListStart = 0;
    int noManualListEnd   = maxDuplToShow - noManualListStart - 1;

    QVector<int> rootsToBeRemoved;

    bool m_updating = false; // wichtig gegen Rekursion
    bool scanActive = false;
    bool stopScan   = false;

    qint64 progressCount = 0;
    qint64 progressMax = 0;
    int nextAction = -1;
    int skipCount = 0;

    QVector<FolderSet> m_sets;
    int m_currentSetIndex = -1;
    QVector<FileEntry> allFiles;
    QVector<int> filesToProcess;
    QVector<int> chunkOfFilesToProcess;
    QVector<ImageDuplPair> allDupl;
    QVector<CompareInfo> pairs;
//    QHash<QString, QVector<int>> pairIndex;
    QHash<PairKey, int> pairIndex;
    QHash<PairKey, ScoreInfo> allScoresInDB;
    QHash<PairKey, ManualInfo> allManualsInDB;
    QVector<DuplicateCluster> imageDupl;
    QVector<QString> sameFolders;

    DuplicateViewWidget *m_duplicateView = nullptr;

    QTimer *progressTimer;
    int idealCount = 8;
    std::atomic<int> done;
    int current_step = 1;
    int chunkSize    = 1000;
    int currentchunkStart = 0;
    QFutureWatcher<void> *watcher = nullptr;

    QString hexApplied;
    QString hexChanged;

    bool forcedApply  = false;
    int commonSameDir = 0;

    qint64 lastTimeElapsedMs = 0;
    qint64 lastTimeDone      = 0;


    void stopCurrentScan();

    // Settings
    void loadSettingsPart1();
    void loadSettingsPart2();
    void saveSettings();
    FolderNode loadNode(QSettings &settings, const QString &groupName);
    void saveNode(QSettings &settings, const FolderNode &node, const QString &groupName);

    //void addDirectoryRecursive(QStandardItem *parentNameItem, const QString &path);
    void buildDirectoryTree(FolderNode &parent, const QString &path);
    QList<QStandardItem*> createRow(const QString &path);

    void syncNode(FolderNode &node);
    void syncWithFilesystem(QVector<FolderSet> &sets);

    bool setNameExists(const QString &name);
    void reloadSetNames();
    // void saveCurrentSet();
    void renameCurrentSet(const QString &newName);

    void rebuildTree(bool expand = true);                        // Daten → UI
    FolderNode buildNode(QStandardItem *nameItem, QStandardItem *stateItem); // UI → Daten


    void setState(QStandardItem *stateItem, FolderState state);
    void updateAppearance(QStandardItem *stateItem);
    void applyState(QStandardItem *item, FolderState state, bool nameItem);

    void setChildrenState(QStandardItem *parentNameItem, FolderState state);
    QList<QStandardItem*> createRowFromNode(const FolderNode &node, int flatIndex);

    FolderNode* findNodeByIndex(const QModelIndex &index);
    void setNodeStateRecursive(FolderNode &node, FolderState state);
    void setNodeExclDirRecursive(FolderNode &node, bool exclDir);

    QStringList getExpandedPaths();
    void restoreExpandedPaths(const QStringList &expanded);

    // Database
    bool initializePictureDB();

    bool tryMatchInDB(QSqlDatabase &db, FileEntry &entry);
    void writeOrbAndPHashToDB(QSqlDatabase &db, FileEntry &entry);

    void readScoreFromDB(QSqlDatabase &db, CompareInfo &info, qsizetype maxRead);
    void readAllScoresFromDB(QSqlDatabase &db);
    void readManualEntriesFromDB(QSqlDatabase &db);
    void writeScoreToDB(QSqlDatabase &db, CompareInfo &info);

    bool findFolderInSet(const QVector<FolderNode> &fn, QString f);
    QString stripFilename(const QString &filename);
    void collectFromFolderNode(const FolderNode &node, QList<FileEntry> &out);
    int countAllPictures();
    void countAllPictures(const FolderNode &node, int &nbrPictures);
//    bool calculateORB(QSqlDatabase &db, FileEntry &entry);
    bool calculateORBandPHash(FileEntry &entry);

    float compareORB(const QByteArray &d1, const QByteArray &d2);
    void CalculateDuplicateList(bool newData);
    void FetchManualDuplicateList();
    void ShowManualDuplicateList(bool newData);

    cv::Mat descriptorsFromByteArray(const QByteArray &data);
    QByteArray descriptorsToByteArray(const cv::Mat &mat);

    void runcalculateORBandPHash();
    void runScanAndFindDuplicates();
    void startProcessing_calculateORB(QList<FileEntry> filesToProces);

    uint64_t computePHash(const cv::Mat& image);
    int hammingDistance(uint64_t a, uint64_t b);

    QVector<float> computeCanny(const cv::Mat& image);
    float compareCanny(const QVector<float> &pic1, const QVector<float> &pic2);

    void updateLabel1Pos();
    void updateLabel2Pos();
    void updateLabel3Pos();
    void updateLabel4Pos();
    void updateLabel5Pos();

    void writeToCsv(QVector<DuplicateImage> &duplImages);

    void prepareProgress(QString text, qint64 nbr, QString colorText, QString colorBar);

    void estimatedTime(qint64 done, qint64 volume);
    void setButtonState(bool newState, bool all);

    void collectedExcludedSameFolders(const QVector<FolderNode> &fn);

    // For UI language:
    QTranslator m_translator;
    QTranslator m_qtTranslator;
    void setLanguage(const QString &locale);
    QStringList availableLanguages() const;
    void retranslateUi();
    void showLanguageDialog();



};

#endif // MAINWINDOW_H
