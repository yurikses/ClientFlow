#ifndef STATISTIC_H
#define STATISTIC_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QCategoryAxis>


class Statistic : public QWidget
{
    Q_OBJECT

private:
    enum PeriodeMode{
        ONE_MONTH,
        TWO_MONTH,
        THREE_MONTH,
        SIX_MONTH,
        ONE_YEAR,
        TWO_YEAR,
        FIVE_YEAR,
    };

    PeriodeMode currentChartMode;

public:
    explicit Statistic(QWidget *parent = nullptr);
    void createTestChart();
    void createClientsChart();
};

#endif // STATISTIC_H
