#ifndef NOMANUALPARAMETERSDIALOG_H
#define NOMANUALPARAMETERSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSlider>

namespace Ui {
class noManualParametersDialog;
}

class noManualParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit noManualParametersDialog(int *nMSort, int *nMSortDir, int *nMListStart, int *nMListEnd, int nMListSize, QWidget *parent = nullptr);
    ~noManualParametersDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void accept() override;

private slots:
    void on_slFrom_valueChanged(int value);
    void on_slTo_valueChanged(int value);
    void on_rbAdded_clicked();
    void on_rbAlpha_clicked();
    void on_rbSize_clicked();
    void on_rbLastMod_clicked();
    void on_rbImageSize_clicked();
    void on_rbAsc_clicked();
    void on_rbDesc_clicked();

private:
    Ui::noManualParametersDialog *ui;

    QLabel  *lblMinCurrent;
    QLabel  *lblMaxCurrent;
    QSlider *slFrom;
    QSlider *slTo;

    int maxRange = 400;

    bool initPhase = true;

    int *noManualSort;
    int *noManualSortDir;
    int *noManualListStart;
    int *noManualListEnd;
    int noManualListSize;

    int m_start;
    int m_end;
    int m_noManualSort;
    int m_noManualSortDir;


    void updateLabelMin();
    void updateLabelMax();
};

#endif // NOMANUALPARAMETERSDIALOG_H
