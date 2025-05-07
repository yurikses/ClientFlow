#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QCategoryAxis>
#include <QComboBox>
#include <QDate>
#include "database.h"

class Statistic : public QWidget {
    Q_OBJECT

public:
    explicit Statistic(QWidget *parent = nullptr, Database *db = nullptr);

private slots:
    void updateChart();

private:
    enum PeriodeMode {
        ONE_MONTH,
        TWO_MONTH,
        THREE_MONTH,
        SIX_MONTH,
        ONE_YEAR,
        TWO_YEAR,
        FIVE_YEAR
    };

    enum ChartType {
        BarChart,
        LineChart
    };

    QComboBox *periodSelector;
    QComboBox *chartTypeSelector; // <-- Новый контроллер
    QChartView *chartView;

    QChart *chart;
    QCategoryAxis *axisX;
    QValueAxis *axisY;

    Database *database;

    void createUI();
    void loadChartData(PeriodeMode mode, ChartType type); // <-- Изменили сигнатуру
    QDate getStartDateFromMode(PeriodeMode mode);
    bool isWeeklyInterval(PeriodeMode mode);
};

#endif // STATISTIC_H
