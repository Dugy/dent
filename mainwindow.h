#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

class QStandardItem;

struct graph;
class QCPAbstractPlottable;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_addFilesButton_clicked();

    void on_lineList_clicked(const QModelIndex &index);

    void on_hideButton_clicked();

    void on_showButton_clicked();

    void on_showAllButton_clicked();

    void on_showOnlyButton_clicked();

    void on_filterEnabledCheck_toggled(bool checked);

    void on_replaceFilesButton_clicked();

    void graphClicked(QCPAbstractPlottable* plottable);

    void on_plotLoadedButton_clicked();

    void on_computeDerivativeButton_clicked();

    void on_plotDerivativeButton_clicked();

    void on_computeLoadButton_clicked();

    void on_exitButton_clicked();

    void on_saveButton_clicked();

	void on_customZoom_stateChanged(int arg1);

	void on_xMinEdit_editingFinished();

	void on_xMaxEdit_editingFinished();

	void on_yMinEdit_editingFinished();

	void on_yMaxEdit_editingFinished();

	void on_renameProfileButton_clicked();

	void on_newProfileButton_clicked();

	void on_profilesBox_currentTextChanged(const QString &arg1);

	void on_showAscendingCheckBox_stateChanged(int arg1);

	void on_showDescendingCheckBox_stateChanged(int arg1);

	void on_computeIntegralButton_clicked();

	void on_plotIntegralButton_clicked();

	void on_plotFittingButton_clicked();

	void on_plotIntegralButton_2_clicked();

private:
    Ui::MainWindow *ui;

    std::vector<graph*> data_;
    std::vector<std::string> filesToLoad;
    void listGraphs();
    QColor colour_[10];

    enum taskType {
        UNSPECIFIED,
        READ,
		DERIVATIVES,
		INTEGRALS,
		INTEGRAL_SUMMARY,
		FITTING,
		STATISTICS
    };
    taskType lastPlotted;
    void plot(taskType what = UNSPECIFIED);
    void setupTasks(taskType first);

    QStandardItemModel selModel_;
    std::vector<QStandardItem*> selModelItems_;
    int graphSelected_;
    void graphSelected();
	bool saveSettings();
	void loadSettings();
	void rebuildProfilesCombo();
	bool changingComboText;
	std::vector<std::pair<float, float>> specialPlotted_;

	void customZoomEditDone();
};

#endif // MAINWINDOW_H
