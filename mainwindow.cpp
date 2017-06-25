#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "log.h"
#include <QFileDialog>
#include <iostream>
#include <fstream>
#include "taskmanager.h"
#include "task.h"
#include "structs.h"
#include "settings.h"
#include "utils.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastPlotted(UNSPECIFIED),
    selModel_(0, 1, this),
	graphSelected_(-1),
	changingComboText(false)
{
    ui->setupUi(this);

    colour_[0] = QColor(255, 0, 0);
    colour_[1] = QColor(0, 255, 0);
    colour_[2] = QColor(0, 0, 255);
    colour_[3] = QColor(255, 255, 0);
    colour_[4] = QColor(0, 255, 255);
    colour_[5] = QColor(255, 0, 255);
    colour_[6] = QColor(255, 100, 100);
    colour_[7] = QColor(100, 255, 100);
    colour_[8] = QColor(100, 255, 255);
    colour_[9] = QColor(0, 0, 0);

    loadSettings();
	rebuildProfilesCombo();

    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    connect(ui->plot, SIGNAL(plottableClick(QCPAbstractPlottable*,
            QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));
}

void MainWindow::loadSettings() {
    Settings& settings = Settings::get();

	ui->coresEdit->setText(intToQString(settings.cores));
	ui->xMinEdit->setText(floatToQString(settings.xMinCustom));
	ui->xMaxEdit->setText(floatToQString(settings.xMaxCustom));
	ui->yMinEdit->setText(floatToQString(settings.yMinCustom));
	ui->yMaxEdit->setText(floatToQString(settings.yMaxCustom));
	ui->customZoom->setChecked(settings.customZoom);
	ui->showDescendingCheckBox->setChecked(settings.showDescending);
	ui->showAscendingCheckBox->setChecked(settings.showAscending);
	ui->filterXWidthEdit->setText(intToQString(settings.filterXWidth));
	ui->filterYWidthEdit->setText(intToQString(settings.filterYWidth));
	ui->filterXMaxEdit->setText(floatToQString(settings.filterXMaximum));
	ui->filterYMaxEdit->setText(floatToQString(settings.filterYMaximum));
	ui->filterXSlopeEdit->setText(intToQString(settings.filterXSlope));
	ui->filterYSlopeEdit->setText(intToQString(settings.filterYSlope));
	ui->changeSamplingEdit->setText(intToQString(settings.changeSampling));
	ui->increaseThresholdEdit->setText(floatToQString(settings.increaseThreshold));
	ui->decreaseThresholdEdit->setText(floatToQString(settings.decreaseThreshold));
	ui->pointsEdit->setText(floatToQString(settings.points));
	ui->filterEnabledCheck->setChecked(settings.filterEnabled);
    on_filterEnabledCheck_toggled(settings.filterEnabled);
	ui->loadNextCheckBox->setChecked(settings.proceedFromLoadToDerivatives);
	ui->startFromEdit->setText(floatToQString(settings.startDerivativeFrom));
	ui->endBeforeEdit->setText(floatToQString(settings.endDerivativeBefore));
	ui->derivativeSmoothingSlopeEdit->setText(
				floatToQString(settings.derivativeSmoothingSlope));
	ui->derivativeSmoothingWidthEdit->setText(
				floatToQString(settings.derivativeSmoothingWidth));
	ui->intoleranceThresholdEdit->setText(
				floatToQString(settings.intoleranceThreshold));
	ui->popinSkipEdit->setText(floatToQString(settings.popinSkip));
    ui->loadNextAgainCheckBox->setChecked(settings.proceedFromDerivativesToIntegrals);
	ui->descendingFormulaEdit->setText(QString(
				settings.downFormula.print(settings.downVariables).c_str()));
	ui->ascendingFormulaEdit->setText(QString(
				settings.upFormula.print(settings.upVariables).c_str()));
	ui->areaFormulaEdit->setText(QString(
				settings.areaFormula.print(settings.areaVariables).c_str()));
    const std::vector<std::string>& vars = settings.generateVarNames(settings);
    std::vector<std::pair<std::string, float (*)(float)>> unaryFuncs;
    unaryFuncs.push_back(std::make_pair("A", Settings::A));
    ui->func0Edit->setText(settings.userFormula[0].print(vars, unaryFuncs).c_str());
    ui->func1Edit->setText(settings.userFormula[1].print(vars, unaryFuncs).c_str());
    ui->func2Edit->setText(settings.userFormula[2].print(vars, unaryFuncs).c_str());
    ui->func3Edit->setText(settings.userFormula[3].print(vars, unaryFuncs).c_str());
    ui->func4Edit->setText(settings.userFormula[4].print(vars, unaryFuncs).c_str());
    ui->func5Edit->setText(settings.userFormula[5].print(vars, unaryFuncs).c_str());
	ui->fittingGuessesEdit->setText(intToQString(settings.fitGuessSteps));
	ui->guessPointsEdit->setText(intToQString(settings.fitGuessPoints));
	ui->fitPointsEdit->setText(intToQString(settings.fitPoints));
	ui->integralSmoothedCheckBox->setChecked(settings.integrateSmoothed);

	// Proceed to next steps is missing... is there a reason for that?
}

void MainWindow::rebuildProfilesCombo() {
	changingComboText = true;
	ui->profilesBox->clear();
	const Settings& settings = Settings::get();
	ui->profilesBox->addItem(QString(settings.currentProfile.c_str()));
	for (const std::pair<const std::string,
			std::map<std::string, std::string>>& it : settings.profiles) {
		if (it.first != settings.currentProfile)
			ui->profilesBox->addItem(QString(it.first.c_str()));
	}
	changingComboText = false;
}

MainWindow::~MainWindow()
{
    for (unsigned int i = 0; i < data_.size(); i++)
        delete data_[i];
    delete ui;
}

bool MainWindow::saveSettings() {
    Settings& settings = Settings::get();
    try {
        auto readInt = [&] (QLineEdit* input, int min,
                int max, QString error) -> int
        {
            int got = qStringToInt(input->text());
			//std::cerr << "Min " << min << " got " << got << " max " << max << std::endl;
            if (got < min || got > max) {
                throw error;
            }
            return got;
        };
        auto readFloat = [&] (QLineEdit* input, float min,
                float max, QString error) -> float
        {
            float got = qStringToFloat(input->text());
			if (got < min || got > max) {
                throw error;
            }
            return got;
        };

        settings.cores = readInt(ui->coresEdit, 1,
                                 999999, "Bad number of cores to use.");
		settings.xMinCustom = readFloat(ui->xMinEdit, -99999,
								 99999, "Bad custom x range.");
		settings.xMaxCustom = readFloat(ui->xMaxEdit, settings.xMinCustom,
								 99999, "Bad custom x range.");
		settings.yMinCustom = readFloat(ui->yMinEdit, -99999,
								 99999, "Bad custom y range.");
		settings.yMaxCustom = readFloat(ui->yMaxEdit, settings.yMinCustom,
								 99999, "Bad custom y range.");
		settings.customZoom = (ui->customZoom->checkState() == Qt::Checked);
		settings.showAscending = (ui->showAscendingCheckBox->checkState() == Qt::Checked);
		settings.showDescending = (ui->showDescendingCheckBox->checkState() == Qt::Checked);
        settings.filterXWidth = readInt(ui->filterXWidthEdit, 1,
                                        999, "Bad filter width in X.");
        settings.filterYWidth = readInt(ui->filterYWidthEdit, 1,
                                        999, "Bad filter width in Y.");
        settings.filterXMaximum = readFloat(ui->filterXMaxEdit, 0,
                                            99,"Bad filter maximum in X.");
        settings.filterYMaximum = readFloat(ui->filterYMaxEdit, 0,
                                            9999, "Bad filter maximum in Y.");
        settings.filterXSlope = readFloat(ui->filterYSlopeEdit, 1,
                                        10, "Bad filter slope in X.");
        settings.filterYSlope = readFloat(ui->filterYSlopeEdit, 1,
                                        10, "Bad filter slope in Y.");
        settings.changeSampling = readInt(ui->changeSamplingEdit, 10,
                                            9999, "Bad change sampling.");
        settings.increaseThreshold = readFloat(ui->increaseThresholdEdit, 0.001,
                                5, "Bad increase threshold.");
        settings.decreaseThreshold = readFloat(ui->decreaseThresholdEdit, 0.001,
                                5, "Bad decrease threshold.");
        settings.points = readFloat(ui->pointsEdit, 10,
                                10000, "Bad number of points.");
        settings.proceedFromLoadToDerivatives = ui->loadNextCheckBox->isChecked();
        settings.startDerivativeFrom = readFloat(ui->startFromEdit, 0,
                                100, "Bad value to start derivative at.");
        settings.endDerivativeBefore = readFloat(ui->endBeforeEdit, 0,
                                100, "Bad value to end derivative before.");
        settings.derivativeSmoothingWidth = readFloat(ui->derivativeSmoothingWidthEdit, 1,
                                50, "Bad value to derivative smoothing width.");
        settings.derivativeSmoothingSlope = readFloat(ui->derivativeSmoothingSlopeEdit, 1,
                                5, "Bad value to derivative smoothing slope.");
        settings.intoleranceThreshold = readFloat(ui->intoleranceThresholdEdit, 1,
                                1000, "Bad value of intolerance threshold.");
		settings.popinSkip = readFloat(ui->popinSkipEdit, 0,
								20, "Bad value of popin skip.");
		settings.proceedFromDerivativesToIntegrals = ui->loadNextAgainCheckBox->isChecked();
		settings.integrateSmoothed = ui->integralSmoothedCheckBox->isChecked();
		std::string descendingFormula = ui->descendingFormulaEdit->text().toUtf8().constData();
		std::string ascendingFormula = ui->ascendingFormulaEdit->text().toUtf8().constData();
		std::string areaFormula = ui->areaFormulaEdit->text().toUtf8().constData();
		Settings::readFormulaVal(settings.downFormula, settings.downVariables,
								descendingFormula, "$var1*$x^2");
		Settings::readFormulaVal(settings.upFormula, settings.upVariables,
								 ascendingFormula, "$var1*($x-$var2)^2");
		Settings::readFormulaVal(settings.areaFormula, settings.areaVariables,
								 areaFormula, "24.5*$h^2");
        Settings::readStatFormula(settings, settings.userFormula[0],
                ui->func0Edit->text().toUtf8().constData(), "$Lmax / (9.81 * A($hmax))");
        Settings::readStatFormula(settings, settings.userFormula[1],
                ui->func1Edit->text().toUtf8().constData(), "$Lmax / A($hmax)");
        Settings::readStatFormula(settings, settings.userFormula[2],
                ui->func2Edit->text().toUtf8().constData(), "$Lmax / A($hmax - 0.73*($hmax - $hr))");
        Settings::readStatFormula(settings, settings.userFormula[3],
                ui->func3Edit->text().toUtf8().constData(), "$hr");
        Settings::readStatFormula(settings, settings.userFormula[4],
                ui->func4Edit->text().toUtf8().constData(), "$Wirr");
        Settings::readStatFormula(settings, settings.userFormula[5],
                ui->func5Edit->text().toUtf8().constData(), "$We");
		if (settings.areaVariables.size() > 1) throw(QString("Area function depends only on one "
				"variable, penetration depth (h), you used ") + QString::number(settings.areaVariables.size()));
		settings.fitGuessSteps = readInt(ui->fittingGuessesEdit, 10000,
										 2000000000, "Bad number of fitting guess steps.");
		settings.fitGuessPoints = readInt(ui->guessPointsEdit, 5,
										 settings.points, "Bad number of fitting guess points.");
		settings.fitPoints = readInt(ui->fitPointsEdit, 10,
										 settings.points, "Bad number of fitting points.");

        return true;
    } catch (QString error) {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", error);
        return false;
    }
	return true;
}

void MainWindow::on_addFilesButton_clicked()
{
    QFileDialog dialog(this);
	Settings& settings = Settings::get();
	dialog.setDirectory((settings.folder.empty()) ? QDir::homePath()
												  : QString(settings.folder.c_str()));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("*.txt"));
    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();
    for (QStringList::iterator it = fileNames.begin();
         it != fileNames.end(); ++it) {
        filesToLoad.push_back(it->toUtf8().constData());
    }
	if (!filesToLoad.empty())
		settings.folder = filesToLoad.front().substr(0,
										filesToLoad.front().find_last_of("/\\"));
}

void MainWindow::on_replaceFilesButton_clicked()
{
    filesToLoad.clear();
    on_addFilesButton_clicked();
}

void MainWindow::on_computeLoadButton_clicked()
{
	if (!saveSettings()) return;
    for (unsigned int i = 0; i < data_.size(); i++)
        delete data_[i];
    data_.clear();
    for (unsigned int i = 0; i < filesToLoad.size(); i++) {
        graph* created = new graph;
        created->visible = true;
        created->name = filesToLoad[i];
        data_.push_back(created);
    }
    setupTasks(READ);
    listGraphs();
    plot();
}

void MainWindow::setupTasks(taskType first) {
	TaskManager& taskManager = TaskManager::getInstance();
	for (unsigned int i = 0; i < data_.size(); i++) {
		graph* data = data_[i];
		switch (first) {
			case UNSPECIFIED:
			case READ:
				taskManager.addTask(new TaskLoad(data));
				lastPlotted = READ;
				if (!ui->loadNextCheckBox->isChecked()) break;
			case DERIVATIVES:
				taskManager.addTask(new TaskDerivative(data));
				lastPlotted = DERIVATIVES;
				if (!ui->loadNextAgainCheckBox->isChecked()) break;
			case INTEGRALS:
			case INTEGRAL_SUMMARY:
				taskManager.addTask(new TaskIntegral(data));
				if (first == INTEGRALS) lastPlotted = INTEGRALS;
				if (!ui->andLoadNextAgainCheckBox->isChecked()) break;
				// Integrals will not be plotted by default
			case FITTING:
				taskManager.addTask(new TaskFit(data));
				if (first == FITTING) lastPlotted = FITTING;
			default:
			{}
		}
	}
	logStream stream;
	stream << "Working" << std::endl;
    try {
        taskManager.work();
    } catch(std::exception& error) {
        QMessageBox::critical(nullptr, "Error", error.what());
    }

	stream << "Done working" << std::endl;
	ui->reportEdit->document()->setPlainText(QString(Log::read().c_str()));
	QScrollBar* sb = ui->reportEdit->verticalScrollBar();
	sb->setValue(sb->maximum());
}

void MainWindow::listGraphs() {
    for (unsigned int i = 0; i < selModelItems_.size(); i++)
        delete selModelItems_[i];
    selModelItems_.clear();
    selModel_.clear();
    graphSelected_ = -1;
    for (unsigned int i = 0; i < data_.size(); i++) {
        // Remove the path from the file name
        std::string text;
        size_t lastOccurrence = data_[i]->name.find_last_of("/\\");
        if (lastOccurrence != std::string::npos)
            text = data_[i]->name.substr(lastOccurrence + 1);
        else text = data_[i]->name;
        selModelItems_.push_back(new QStandardItem(QString(text.c_str())));
        selModelItems_.back()->setForeground(QBrush(colour_[i % 10]));
        selModel_.appendRow(selModelItems_.back());
    }
    ui->lineList->setModel(&(selModel_));
    ui->showAllButton->setEnabled(true);
    ui->showOnlyButton->setEnabled(true);
    ui->hideButton->setEnabled(false);
    ui->showButton->setEnabled(false);
}

void MainWindow::plot(taskType what) {
	saveSettings();
    if (what == UNSPECIFIED) what = lastPlotted;
	if (what == UNSPECIFIED) {
        std::cerr << "Trying to plot an unknown type of graph." << std::endl;
		Log::write("Trying to plot an unknown type of graph.\n");
		return;
	}
    ui->plot->clearGraphs();
	float minX = 9000000000;
	float maxX = -900000000;
	float minY = 9000000000;
	float maxY = -9000000000;
	Settings& settings = Settings::get();
	if (what == INTEGRAL_SUMMARY || what >= FUNC0) {
		QCPGraph* line = ui->plot->addGraph();
		line->setPen(QPen(colour_[0]));
		specialPlotted_.clear();
		if (what == INTEGRAL_SUMMARY) {
			line->setName("Integral");
			for (unsigned int i = 0; i < data_.size(); i++) {
				float x = data_[i]->base[DOWN].end;
				float y = data_[i]->integral[DOWN].points[data_[i]->integral[0].length - 1];
				line->addData(x, y);
				specialPlotted_.push_back(std::make_pair(x, y));
				if (!settings.customZoom) {
					if (x > maxX) maxX = x;
					if (x < minX) minX = x;
					if (y < minY) minY = y;
					if (y > maxY) maxY = y;
				}
			}
		}
		if (what >= FUNC0) {
			line->setName("User function");
			for (unsigned int i = 0; i < data_.size(); i++) {
				float x = data_[i]->Lmax;
				const std::vector<float>& vars = data_[i]->getVariables();
				float y = Settings::get().userFormula[what - FUNC0](&(vars[0]));
				line->addData(x, y);
				specialPlotted_.push_back(std::make_pair(x, y));
				if (!settings.customZoom) {
					if (x > maxX) maxX = x;
					if (x < minX) minX = x;
					if (y < minY) minY = y;
					if (y > maxY) maxY = y;
				}
			}
		}
		if (data_.size() > 0) {
			if (what == INTEGRAL_SUMMARY) {
				ui->plot->xAxis->setLabel(QString::fromUtf8(
									std::string("max " + data_[0]->axisX).c_str()));
				ui->plot->yAxis->setLabel(QString::fromUtf8(
									std::string("max ∫(" + data_[0]->axisY + ")dh").c_str()));
			}
			if (what >= FUNC0) {
				ui->plot->xAxis->setLabel(QString::fromUtf8(
									std::string("max " + data_[0]->axisX).c_str()));
				ui->plot->yAxis->setLabel(QString::fromUtf8(
							std::string("Function " + to_string(what - FUNC0 + 1)).c_str()));
			}
		}
	} else {
		for (unsigned int i = 0; i < data_.size(); i++) {
			graph* data = data_[i];
			if (!data->visible) continue;
			auto drawLine = [&] (curve& current) -> void {
				if (current.length == 0 || current.points == nullptr) {
					std::cerr << "Plotting an empty line" << std::endl;
					Log::write("Plotting an empty line.\n");
					return;
				}
				QCPGraph* line = ui->plot->addGraph();
				line->setPen(QPen(colour_[i % 10]));
				line->setName(intToQString(i));
				if (!settings.customZoom) {
					if (current.start < minX)
						minX = current.start;
					if (current.start > maxX)
						maxX = current.start;
					if (current.end > maxX)
						maxX = current.end;
					if (current.end < minX)
						minX = current.end;
				}
				float step = (current.end - current.start)
						/ current.length;
				float pos = current.start;
				for (unsigned int i = 0; i < current.length; i++) {
					if (current.points[i] == current.points[i]) // Not NaN
						line->addData(pos, current.points[i]);
					if (!settings.customZoom) {
						if (current.points[i] < minY)
							minY = current.points[i];
						if (current.points[i] > maxY)
							maxY = current.points[i];
					}
					pos += step;
				}
			};
			if (what == READ) {
				if (settings.showDescending) drawLine(data->base[DOWN]);
				if (settings.showAscending) drawLine(data->base[UP]);
			} else if (what == DERIVATIVES) {
				if (settings.showDescending) drawLine(data->derivative[DOWN]);
				if (settings.showAscending) drawLine(data->derivative[UP]);
			} else if (what == INTEGRALS) {
				if (settings.showDescending) drawLine(data->integral[DOWN]);
				if (settings.showAscending) drawLine(data->integral[UP]);
			} else if (what == FITTING) {
				if (settings.showDescending) drawLine(data->fitted[DOWN]);
				if (settings.showAscending) drawLine(data->fitted[UP]);
			}
		}

		if (data_.size() > 0) {
			ui->plot->xAxis->setLabel(QString::fromUtf8(data_[0]->axisX.c_str()));
			if (what == DERIVATIVES) {
				ui->plot->yAxis->setLabel(QString::fromUtf8(std::string("d/dh2("
												+ data_[0]->axisY + ")").c_str()));
			} else if (what == INTEGRALS) {
				ui->plot->yAxis->setLabel(QString::fromUtf8(std::string("∫("
												+ data_[0]->axisY + ")dh").c_str()));
			} else {
				ui->plot->yAxis->setLabel(QString::fromUtf8(data_[0]->axisY.c_str()));
			}
		}
	}

	if (settings.customZoom) {
		ui->plot->xAxis->setRange(settings.xMinCustom, settings.xMaxCustom);
		ui->plot->yAxis->setRange(settings.yMinCustom, settings.yMaxCustom);
	} else {
		ui->plot->xAxis->setRange(minX, maxX);
		ui->plot->yAxis->setRange(minY, maxY);
	}
	lastPlotted = what;

    ui->plot->replot();
}

void MainWindow::on_lineList_clicked(const QModelIndex &index)
{
    if (!data_.empty()) {
        graphSelected_ = selModel_.itemFromIndex(index)->row();
        graphSelected();
    }
}

void MainWindow::graphSelected() {
    ui->hideButton->setEnabled(data_[graphSelected_]->visible);
    ui->showButton->setEnabled(!data_[graphSelected_]->visible);
}

void MainWindow::on_hideButton_clicked()
{
    if (graphSelected_ >= 0) {
        data_[graphSelected_]->visible = false;
    }
    graphSelected();
    plot();
}

void MainWindow::on_showButton_clicked()
{
    if (graphSelected_ >= 0) {
        data_[graphSelected_]->visible = true;
    }
    graphSelected();
    plot();
}

void MainWindow::on_showAllButton_clicked()
{
    for (unsigned int i = 0; i < data_.size(); i++) {
        data_[i]->visible = true;
    }
    graphSelected();
    plot();
}

void MainWindow::on_showOnlyButton_clicked()
{
    if (graphSelected_ >= 0) {
        for (unsigned int i = 0; i < data_.size(); i++) {
            data_[i]->visible =
                    (i == (unsigned int)graphSelected_);
        }
    }
    graphSelected();
    plot();
}

void MainWindow::on_filterEnabledCheck_toggled(bool checked)
{
    Settings::get().filterEnabled = checked;
    ui->filterXLabel->setEnabled(checked);
    ui->filterYLabel->setEnabled(checked);
    ui->filterXWidthEdit->setEnabled(checked);
    ui->filterYWidthEdit->setEnabled(checked);
    ui->filterXMaxEdit->setEnabled(checked);
    ui->filterYMaxEdit->setEnabled(checked);
    ui->filterSlopeLabel->setEnabled(checked);
    ui->filterXSlopeEdit->setEnabled(checked);
    ui->filterYSlopeEdit->setEnabled(checked);
    ui->filterWidthLabel->setEnabled(checked);
    ui->filterMaximumLabel->setEnabled(checked);
}

void MainWindow::graphClicked(QCPAbstractPlottable *plottable)
{
	if (lastPlotted == INTEGRAL_SUMMARY) return;
    graphSelected_ = qStringToInt(plottable->name());
    QModelIndex index = selModel_.index(graphSelected_, 0);
    ui->lineList->setCurrentIndex(index);
    graphSelected();
}

void MainWindow::on_plotLoadedButton_clicked()
{
    plot(READ);
}

void MainWindow::on_computeDerivativeButton_clicked()
{
	if (!saveSettings()) return;
    setupTasks(DERIVATIVES);
    plot();
}

void MainWindow::on_plotDerivativeButton_clicked()
{
    plot(DERIVATIVES);
}

void MainWindow::on_saveButton_clicked()
{
	if (!saveSettings()) return;
	if (data_.empty()) return;
	if (lastPlotted == INTEGRAL_SUMMARY || lastPlotted >= FUNC0) {
		QString fileName = QFileDialog::getSaveFileName(this,
							tr("Choose file name and location"), "");
		std::ofstream stream(fileName.toUtf8().constData(),
							 std::ofstream::out | std::ofstream::trunc);
		for (unsigned int i = 0; i < specialPlotted_.size(); i++) {
			stream << specialPlotted_[i].first << "\t" <<
					  specialPlotted_[i].second << std::endl;
		}
		stream.close();
	} else {
		QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
					"", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		for (unsigned int i = 0; i < data_.size(); i++) {
			graph* current = data_[i];
			if (!current->visible) continue;
			std::string fileName = dirName.toUtf8().constData()
					+ current->name.substr(current->name.find_last_of("/\\")) + ".csv";
			std::ofstream stream(fileName, std::ofstream::out | std::ofstream::trunc);
			auto saveCurve = [&] (curve& saved) {
				float pos = saved.start;
				float step = (saved.end - saved.start) / saved.length;
				for (unsigned int j = 0; j < saved.length; j++) {
					if (saved.points[i] == saved.points[i]) // skip NaN values of unusable parts
						stream << pos << "\t" << saved.points[j] << std::endl;
					pos += step;
				}
			};
			if (Settings::get().showDescending) {
				if (lastPlotted == READ) {
					saveCurve(current->base[DOWN]);
				} else if (lastPlotted == DERIVATIVES) {
					saveCurve(current->derivative[DOWN]);
				} else if (lastPlotted == INTEGRALS) {
					saveCurve(current->integral[DOWN]);
				} else if (lastPlotted == FITTING) {
					saveCurve(current->fitted[DOWN]);
				}
			}
			if (Settings::get().showAscending) {
				if (lastPlotted == READ) {
					saveCurve(current->base[UP]);
				} else if (lastPlotted == DERIVATIVES) {
					saveCurve(current->derivative[UP]);
				} else if (lastPlotted == INTEGRALS) {
					saveCurve(current->integral[UP]);
				} else if (lastPlotted == FITTING) {
					saveCurve(current->fitted[UP]);
				}
			}
			stream.close();
			Log::write("Saved to " + fileName + "\n");
		}
	}
}

void MainWindow::on_exitButton_clicked()
{
    close();
}

void MainWindow::on_customZoom_stateChanged(int arg1)
{
	(void)arg1;
	Settings::get().customZoom = (ui->customZoom->checkState() == Qt::Checked);
	plot();
}

void MainWindow::customZoomEditDone()
{
	if (Settings::get().customZoom) {
		if (!saveSettings()) return;
		plot();
	}
}

void MainWindow::on_xMinEdit_editingFinished()
{
	customZoomEditDone();
}

void MainWindow::on_xMaxEdit_editingFinished()
{
	customZoomEditDone();
}

void MainWindow::on_yMinEdit_editingFinished()
{
	customZoomEditDone();
}

void MainWindow::on_yMaxEdit_editingFinished()
{
	customZoomEditDone();
}

void MainWindow::on_renameProfileButton_clicked()
{
	Settings& settings = Settings::get();
	std::string name = ui->profileNameEdit->text().toUtf8().constData();
	if (settings.profiles.find(name) != settings.profiles.end()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Such profile already exists");
		return;
	}
	settings.profiles[name] = settings.profiles[settings.currentProfile];
	settings.profiles.erase(settings.currentProfile);
	settings.currentProfile = name;
	rebuildProfilesCombo();
}

void MainWindow::on_newProfileButton_clicked()
{
	Settings& settings = Settings::get();
	std::string name = ui->profileNameEdit->text().toUtf8().constData();
	if (settings.profiles.find(name) != settings.profiles.end()) {
		QMessageBox messageBox;
		messageBox.critical(0, "Error", "Such profile already exists");
		return;
	}
	//std::map<std::string, std::string>& curr = settings.profiles[name];
	settings.currentProfile = name;
	if (saveSettings()) {
		settings.synchChanges();
	}
	rebuildProfilesCombo();
}

void MainWindow::on_profilesBox_currentTextChanged(const QString &arg1)
{
	if (!changingComboText) {
		changingComboText = true;
		Settings& settings = Settings::get();
		saveSettings();
		settings.synchChanges();
		settings.currentProfile = arg1.toUtf8().constData();
		settings.setup();
		loadSettings();
		rebuildProfilesCombo();
		changingComboText = false;
	}
}

void MainWindow::on_showAscendingCheckBox_stateChanged(int arg1)
{
	if (ui->showDescendingCheckBox->checkState() == Qt::CheckState::Unchecked &&
			arg1 == Qt::CheckState::Unchecked) {
		ui->showDescendingCheckBox->setChecked(true);
	}
}

void MainWindow::on_showDescendingCheckBox_stateChanged(int arg1)
{
	if (ui->showAscendingCheckBox->checkState() == Qt::CheckState::Unchecked &&
			arg1 == Qt::CheckState::Unchecked) {
		ui->showAscendingCheckBox->setChecked(true);
	}
}

void MainWindow::on_computeIntegralButton_clicked()
{
	if (!saveSettings()) return;
	setupTasks(INTEGRALS);
	plot();

}

void MainWindow::on_plotIntegralButton_clicked()
{
	plot(INTEGRALS);
}

void MainWindow::on_plotFittingButton_clicked()
{
	plot(FITTING);
}

void MainWindow::on_plotIntegralButton_2_clicked()
{
	plot(INTEGRAL_SUMMARY);
}

void MainWindow::on_showFunc0Button_clicked()
{
	plot(FUNC0);
}

void MainWindow::on_showFunc1Button_clicked()
{
	plot(FUNC1);
}

void MainWindow::on_showFunc2Button_clicked()
{
	plot(FUNC2);
}

void MainWindow::on_showFunc3Button_clicked()
{
	plot(FUNC3);
}

void MainWindow::on_showFunc4Button_clicked()
{
	plot(FUNC4);
}

void MainWindow::on_showFunc5Button_clicked()
{
	plot(FUNC5);
}

void MainWindow::on_insertAreaValuesButton_clicked()
{
	QDialog dialog(this);
	QFormLayout form(&dialog);

	form.addRow(new QLabel("Set the standard values of the polynomial"));

	QList<QLineEdit*> coefficients;
	for (int i = 0; i < 6; i++) {
		QLineEdit* lineEdit = new QLineEdit(&dialog);
		QString label = QString(i < 2 ? (i == 0 ? "Power 2" : "Power 1") : std::string("Power 1/"
										+ to_string((int)(1 << (i - 1)))).c_str());
		form.addRow(label, lineEdit);
		coefficients << lineEdit;
	}

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
							   Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	if (dialog.exec() == QDialog::Accepted) {
		std::string result = "0";
		auto addToResult = [&] (const std::string& added) {
			if (result == "0") result = added;
			else result += " + " + added;
		};
		const char* read = coefficients[0]->text().toUtf8().constBegin();
		if (read[0] != 0 && (read[0] != '0' || read[1] != 0)) {
			addToResult(read + std::string(" * $h ^ 2"));
		}
		read = coefficients[1]->text().toUtf8().constBegin();
		if (read[0] != 0 && (read[0] != '0' || read[1] != 0)) {
			addToResult(read + std::string(" * $h"));
		}
		for (int i = 2; i < 6; i++) {
			read = coefficients[i]->text().toUtf8().constBegin();
			if (read[0] != 0 && (read[0] != '0' || read[1] != 0)) {
				addToResult(read + std::string(" * $h ^ ") + to_string( 1.0 / (1 << (i - 1))));
			}
		}
		ui->areaFormulaEdit->setText(result.c_str());
	}
}
