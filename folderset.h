#ifndef FOLDERSET_H
#define FOLDERSET_H

#include <QString>
#include <QList>

enum FolderState {
    Excluded = 0,
    Included = 1,
    Reference = 2
};

enum Type {
    Duplicate,  // echtes Duplikat
    Copy        // gleiche Datei, anderer Pfad (DB Match)
};


struct FolderNode {
    QString path;
    int     nbrPictures;
    FolderState state = Included;
    QVector<FolderNode> children;
    bool    excludeSameFolder = false;
};

struct FolderSet {
    QString name;
    QVector<FolderNode> roots;
};

struct FileEntry {
    qint64   uuid;
    QString path;
    QString filename;
    QString filenameStripped;
    qint64  size;
    qint64  lastModified;
    bool    referenceImg;
    bool    imgCopy;
    QString parentPath;
    int     foundLevel;  // 0 = full path;  1 = filename;  2 = stripped filename

    // aus DB (optional)

    QByteArray      keyPoints;
    QByteArray      descriptors;
    uint64_t        pHash;
    QVector<float>  canny;
    bool            foundInDB    = false;
    bool            entryUpdated = false;
    // Operator für den Vergleich von FileEntry-Objekten
    bool operator==(const FileEntry &other) const {
        return filename == other.filename && path == other.path;
    }
};

struct ImageInfo
{
    QString path;
    QString filename;
    qint64  size;
    qint64  lastModified;
    int width  = 0;
    int height = 0;
};

enum struct orbStatus { noDuplicate, Duplicate, calculate };

struct CompareInfo {
    int    i;
    int    j;
    qint64 uuid1;
    qint64 uuid2;
    float  score;
    int    pHashDistance;
    float  cannyDiff;
    bool   foundInDB;
    orbStatus  orbNeeded;
    bool   deleted;
    bool   manualNoDuplicate;
    bool   calculationDone;
    qint64 manTime;
};


struct ImageDuplPair
{
    int   indexImg1;
    int   indexImg2;
    float score;
    int   pHashDistance;
    float cannyDiff;

    ImageDuplPair(int i1, int i2, float s, int d, float c)
        : indexImg1(i1), indexImg2(i2), score(s), pHashDistance(d), cannyDiff(c) {}
};



struct DuplicateImage {
    int     indexImg;
    QString filepath;
    QString filename;
    int     width;
    int     height;
    bool    referenceImg;
    bool    parentImg;
    int     parentIdx;
    QVector<int>   duplIndices;
    QVector<int>   duplallFilesIndices;
    QVector<float> duplScores;
    QVector<int>   duplHashDistances;
    QVector<float> duplCannyDiff;

    DuplicateImage(int i, const QString& fp, const QString& fn, bool rI) :
        indexImg(i), filepath(fp), filename(fn), referenceImg(rI), parentImg(false), parentIdx(-1), width(-1), height(-1) {}
};

struct DuplicateItem
{
    ImageInfo info;
    Type      type;
    float     score         = 0.0f;
    int       pHashDistance = 0;
    float     cannyDiff     = 0;
    qint64    manTime       = 0;

    DuplicateItem(ImageInfo &i, Type t, float s, int d, float c, qint64 mt = 0) :
        info(std::move(i)), type(t), score(s), pHashDistance(d), cannyDiff(c), manTime(mt) {}
};

struct DuplicateCluster
{
    ImageInfo root;
    QVector<DuplicateItem> items;

    DuplicateCluster(ImageInfo &i) : root(std::move(i)) {}
};




#endif // FOLDERSET_H
